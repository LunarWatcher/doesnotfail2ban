# File parser

The file parser is the simplest parser, and as the name indicates, it reads files.

## Options

* **multiprocess** [bool]: Default false, indicates whether or not the file is used by multiple processes. While not applicable due to the dedicated journald parser, `/var/log/auth.log` contains multiple processes, and signals which provided the logs in each entry. Setting multiprocess to true allows the watchers to filter only to the process they're looking for, which saves computational time when reading lots of entries.
* **pattern** [object]:
    * **full** [regex]: regex pattern describing how to read a line of the file
    * **time** [[time format string](https://en.cppreference.com/w/cpp/io/manip/get_time)]: time format describing how to parse the time
    * **groups** [object]: Describes how the capturing groups in the regex should be read. Each key in this object is a single number corresponding to the number of the capturing group of the regex in `full`
        * **ip** [int, optional]: Refers to a capturing group for an IP, if any. **NOTE:** The IP must not be in the message for this capturing group to be used. If the IP is in the message, this should be handled by the filter implementations instead, and this group should be left undefined.
        * **host** [int, optional]: Capturing group for the hostname
        * **message** [int, mandatory]: Capturing group for the message. As a reminder, the message is the part forwarded to the filters
        * **time** [int, mandatory]: Capturing group for the time. This capturing group MUST be parsable by the `pattern.time` key.

### Example
```
{
    "type": "file",
    "multiprocess": false,
    "pattern": {
        "full": "([^ ]+) - [^ ]+ \\[([^\\]]+) +\\d+\\] (.*)",
        "time": "%d/%b/%Y:%T",
        "groups": {
            "ip": 0,
            "time": 1,
            "message": 2
        }
    }
}
```
This example is based on (an early version of) the matcher for nginx' `/var/log/nginx/access.log`, with messages in this format:
```
192.168.0.179 - - [10/Dec/2023:01:10:01 +0100] "GET /api/push/nevrg0nnag1vey0uup?status=up&msg=OK&ping= HTTP/2.0" 200 11 "-" "curl/7.74.0"
192.168.0.170 - - [10/Dec/2023:01:11:09 +0100] "GET / HTTP/1.1" 200 11193 "-" "Uptime-Kuma/1.23.8"
```

