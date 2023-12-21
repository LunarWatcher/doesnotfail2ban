# Journald parser

The journald parser is a special parser using the systemd API for content.

It does this to avoid reading files like `/var/log/auth.log`, for two reasons:
1. The date formats aren't universal, making parsing them a fucking pain in the ass
2. Newer versions of debian [may or may not use the file at all](https://github.com/fail2ban/fail2ban/issues/3645)

Using the systemd API ensures that whatever format the files may be in, dnf2b gets machine-interpretable data.

## Options

* **idMethod** [enum("syslog", "systemd_unit")]: The process identification method to use. The meanings should probably be documented in detail, but that's a problem for future me
* **pattern** [regex, optional]: A regex used to extract a message from within the message. This is only required if the message stored in journald contains logger-specific information that should be disregarded. **Note:** The parser is fairly lax, and if the regex doesn't match, the raw message is forwarded directly. This means that this is a supplemental extraction aid, and not a log filter. The regex supports the following case-senstiive named groups (syntax: `(?<GroupName>regex1234)`):
    * **Msg** [required]: Used to capture the actual log message
    * **IP** [optional]: Used to capture the IP. Should only be used if the IP is not part of the message itself

### Example
```
{
    "type": "journalctl",
    "idMethod": "syslog"
}
```

#### Message within the message

In certain cases, the output from a process may contain logger information that isn't part of the message itself. For example, dnf2b's own log entries look like this:
```
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.619] [info] Daemon is live and watching for evil shit
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.619] [info] Watching 1 file for changes
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.618] [info] Parser not found for sshd; initialising...
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.614] [info] dnf2b-v6-blacklist already exists. Flushing set...
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.612] [info] dnf2b-v4-blacklist already exists. Flushing set...
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.611] [info] FORWARD -> dnf2b already exists in ip6tables
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.609] [info] INPUT -> dnf2b already exists in ip6tables
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.608] [info] Injecting FORWARD and INPUT rules
Dec 20 20:39:18 nova dnf2b[235520]: [2023-12-20 20:39:18.543] [info] ip6tables already initialised. Purging chain
# Note: the next message is an outlier, as it's the output of a subprocess,
# and not dnf2b proper. As mentioned earlier, if the pattern 
# regex does not match, the raw message is passed directly to
# the enabled filters.
Dec 20 20:39:18 nova dnf2b[235531]: ip6tables: Chain already exists.
```

In this case, when using the journald parser, the message retrieved from journald is
```
[2023-12-20 20:39:18.608] [info] Injecting FORWARD and INPUT rules
```

Which isn't quite there yet. This is where the pattern option comes in. For example, using:
```json
{
    "type": "journald",
    "idMethod": "syslog",
    "pattern": "^\\[\\S+ \\S+\\] [\S+] (?<Msg>.*)$"
}
```

The message sent to the filters becomes
```
Injecting FORWARD and INPUT rules
```
instead. This is meant to simplify filter creation, as the alternative would be to stick the standard log format in front of every pattern, which is unnecessary.

However, for example
```
ip6tables: Chain already exists.
```
Does not match this regex. It'll be forwarded as-is to the filters instead. Unlike the file parsers, the regex is not used as a filter as much as an extra parser.

**Note:** Unlike the file parser, the subset groups are much more limited. You can only extract the IP and the actual message content, as everything else is retrieved with much more accuracy from journald itself, without pesky parsing-related difficulties.

