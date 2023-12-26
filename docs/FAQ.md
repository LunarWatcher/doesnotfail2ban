# FAQ

## What are the different components and files?

There are three supported components: parsers, filters, and watchers. Communicators will be added in the future, but are not yet implemented at this time. 

Parsers are in charge of taking an input resource, and telling dnf2b what the log messages are saying. The two primary parsers are the journald parser and the file parser.

Watchers are a glue component; they receive data from the parsers and forward them to enabled filters. Multiple watchers can subscribe to a single parser under certain circumstances (particularly, this applies to logfiles that multiple processes write to).

Filters, as the name indicates, describes the rules used to match bad log entries. Filters are usually unique per service, as different services naturally have different log messages and ways to describe errors.

## Are there sample configurations for watchers of different services?

Yes. See `docs/Service-config-examples.md`.

## What do the parser files detail?

The parser files describe different usage scenarios for the code-implemented parsers (that's the journald and file parsers at the time of writing). 

### Do I need to use a custom parser file?

Not necessarily. If you're trying to monitor a systemd service, using the plain journald parser (`parsers/journald.json`) is fine. However, as demonstrated by `parsers/sshd.json`, there are cases where a separate parser file is necessary. For sshd, it's used to enable a secondary IP parser and messaage association system. This isn't necessary for all services, and for some, it isn't going to be possible.

For file parsers, it's more likely to be necessary. File parser implementations (such as `parsers/nginx-access.json`) describe how to read a file format. The file to read is defined in the watcher and not in the parser for reusability purposes (multiple files could be using the same format). If the file format you're looking to parse is already defined, you don't need a custom parser file.

## Why is use of the file parser discouraged?

This is outlined in more detail in `docs/parsers/FileParser.md`, but the simplified version is that the time parsing is problematic. Certain formats do not guarantee a standardised time format across different machines. To keep this from being a blocking bug, if the time fails to parse, the file parser falls back to the current time for all messages, regardless of when they were written. This completely prevents any time-based reread mechanics from working.

Even if the given format happens to match the format expected by the file parser, for implementation reasons, timezones are not parsed. The file parser is therefore subject to daylight savings bugs. This means the file parser cannot offer as much reliability against re-reads as the journald parser.

Additionally, any systemd service that logs to stdout (for example using `print()` in whatever language), possibly in addition to a custom file, is automatically logged by journald. This means there's zero overhead to using journald over a file parser for most systemd services. Only a few outliers that don't log to stdout (such as nginx) actually need to use the file parser, which is why it's discouraged for general use.

Journald offers a stable and machine-readable time format, which eliminates all time parsing problems. If you can use a parser with support for machine-readable dates, that parser should **always** be preferred, and the file parser should be a last resort.

## Is there a way to add multiple filters for the same service?

Dnf2b's filter philosophy is to not abstract the rule loading process. One of fail2ban's biggest problems (in my opinion) is that the filter loading process isn't explicit about what filters actually get loaded. With outdated information about f2b lurking around the internet, this makes it unnecessarily difficult to validate if the expected rules were, in fact, loaded.

Due to this, there is not a way to bulk-add filters, and the names have to be explicitly listed. This also makes filter load failures more transparent, as the filter names have a direct corresponding match a file. It does mean typing out more filter names, but it also means you know:

1. Precisely what filters you've loaded,
2. That the filters you've loaded all loaded successfully. Failure to load a filter throws an error, and loaded filters are additionally auditable at runtime, and;
3. That there's no ambiguity introduced by syntactical sugar. Ambiguity means weaker validation, meaning weaker to non-existent guarantees that you get the level of filtering you want.


