# File parser

The file parser is the simplest parser, and as the name indicates, it reads files.

## A warning about the file parser

**TL;DR:** Use the journald parser or another machine readable parser instead, if you can. The file parser is unreliable because humans can't agree on time formats, and the time formats are dependent on the system. If you're considering a file parser, make 500% sure you actually should be.

---

[Time parsing sucks](https://www.youtube.com/watch?v=-5wpm-gesOY), and unfortunately, no good parsing implementations exist in C++[^1][^2][^3]. There are also some significant complications with certain files, particularly anything related to systemd. To spare you the full rant, here are the problems:

1. In certain cases, time format standards aren't guaranteed. `/var/log/auth.log`, for example, depends on a time format as configured in systemd. This means different machines have different formats, and the parsers aren't portable anymore
2. Certain programs use locale-dependent timestamps rather than an ISO standard. This has a few more fun problems:
    1. The US deciding standards can go fuck themselves, [and inventing bad formats for stuff](https://en.wikipedia.org/wiki/Date_and_time_notation_in_the_United_States)[^4]. 
    2. 24 hour time vs. 12 hour time
    3. Months being denoted in the local language rather than in English or, even better, numeric form
3. Certain formats don't include all the time fields, which means parsing has to infer missing fields. This is currently handled
4. Timezones, which the current date parser doesn't handle (and switching to an implementation that does means point 3 would no longer be handled)
5. DST; see the Tom Scott video
6. Any form of custom time parser aimed to solve the domain-specific problems to dnf2b is going to evolve into a full-time job, and even then, it would **still** not be reliable.

There are only two ways to fully avoid this:

1. Get everyone to ditch DST, and have every single program output the full date and time, with the timezone offset, as specified in [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601), or some other standard that becomes cannon. (good luck with that; get world peace while you're at it)
2. Skipping time parsing altogether by using alternate parsing methods with support for machine-readable output

And the good news is that a solution using strategy #2 is already here. Instead of using a file parser, please consider using the journald parser instead. If the thing you're running is a systemd service, and it outputs relevant logs directly to stdout, the journald parser can be used. Journald has a stable API, and outputs a UNIX timestmap (in microseconds) that doesn't need parsing, and isn't subject to nearly as much time-related stupidity.

The file parser should ONLY be used as a last resort. The variance in date formatting means that the parser doesn't support storing the read state of the logfile, and will be rescanned in its entirety when dnf2b is retarted. This can and will result in previously caught offenses being caught more than once, and also results in other parts of logic working less optimally. The files are also subject to variations between distros or even distro versions, and many other things that make them much more difficult to define in a portable way. The journald parser is likely going to be the most commonly used parser, but _any_ parser able to use a system prioritising machine reading is going to be better than this parser.

## Options

* **multiprocess** [bool]: Default false, indicates whether or not the file is used by multiple processes. While not applicable due to the dedicated journald parser, `/var/log/auth.log` contains multiple processes, and signals which provided the logs in each entry. Setting multiprocess to true allows the watchers to filter only to the process they're looking for, which saves computational time when reading lots of entries.
* **pattern** [object]:
    * **full** [regex]: regex pattern describing how to read a line of the file. To keep track of important variables, PCRE's named capture groups are used (syntax: `(?<Group name>regex1234)`). The four built-in groups are (**NOTE**: case-sensitive):
        * **IP** [int, optional]: Refers to a capturing group for an IP, if any. **NOTE:** The IP must not be in the message for this capturing group to be used. If the IP is in the message, this should be handled by the filter implementations instead, and this group should be left undefined.
        * **Host** [int, optional]: Capturing group for the hostname
        * **Msg** [int, mandatory]: Capturing group for the message. As a reminder, the message is the part forwarded to the filters
        * **Time** [int, mandatory]: Capturing group for the time. This capturing group MUST be parsable by the `pattern.time` key.
    * **time** [[time format string](https://en.cppreference.com/w/cpp/io/manip/get_time)]: time format describing how to parse the time. Note that this is both required, and has to correctly match the date. Failing to match the date will guess the read time as the timestamp where it's required, and will result in a complete file reread

### Example
```
{
    "type": "file",
    "multiprocess": false,
    "pattern": {
        "full": "(?<IP>\\S+) - [^ ]+ \\[(?<Time>[^\\]]+) +\\d+\\] (?<Msg>.*)",
        "time": "%d/%b/%Y:%T"
    }
}
```
This example is based on (an early version of) the matcher for nginx' `/var/log/nginx/access.log`, with messages in this format:
```
192.168.0.179 - - [10/Dec/2023:01:10:01 +0100] "GET /api/push/nevrg0nnag1vey0uup?status=up&msg=OK&ping= HTTP/2.0" 200 11 "-" "curl/7.74.0"
192.168.0.170 - - [10/Dec/2023:01:11:09 +0100] "GET / HTTP/1.1" 200 11193 "-" "Uptime-Kuma/1.23.8"
```

[^1]: This is an entire rant onto itself, but here's the simplified version: `std::get_time` is the C++11 method of doing time parsing. C++17 introduced `std::chrono`, which is better in some ways for keeping track of different values of time, but it does not do parsing until C++20. That function has not yet been implemented on my machine, and the implementation isn't all that good anyway. There are some libraries that ship with their own time parsing libraries, including Poco, Boost, and a third one I've seen that I forget the name of, but these are bundles and come with so much overhead. Asio is already sourced for `asio::ip`, but that's a non-boost variant. 
[^2]: As an aside, due to using `std::get_time`, no timezone parsing is done. This means there are DST bugs in countries that still use this crime against humanity, even after an initial vote to get rid of it ([looking at you, EU](https://www.thelocal.com/20221027/whatever-happened-to-the-eu-plan-to-ditch-the-changing-of-the-clocks)), and these are going to be extremely annoying to find and fix.
[^3]: Even with a good implementation, there are challenges. Logfiles don't always contain exact dates. Some just contain a timestamp, which means that in code, the year, month, and day may have to be inferred from context. This is a massive challenge, as many parsing libraries do not make this easy to do. This is particularly true of chrono, which requires a bunch of different subclasses to extract anything other than unix time, and this requires rounding, subtraction, and flooring to do, which is so unnecessary to do manually. I don't understand why there isn't easier access for extracting different formats, but that's what you get when someone who doesn't know how time is used in practice is put in charge of writing an stdlib implementation of a time parser.
[^4]: I personally prefer dd-mm-yyyy (with any separators), but yyyy-mm-dd is equally fine, and if I had to implement a time parser that guessed component meanings, both of these would be trivial. Find the year, determine if there's numbers before or after. If after, the format is yyyy-mm-dd. Otherwise, it's dd-mm-yyyy. Both of these have order, with either the biggest or smallest going first, and each step going in the same direction, rather than the chaotic mess that is the US date format, that also cannot be inferred in the same way. 
