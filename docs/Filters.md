# Filters


## File syntax

```json
{
    "patterns": [
        "A list of ECMAScript-like regex patterns. See https://en.cppreference.com/w/cpp/regex/ecmascript"
    ],
    "insensitive": false
}
```

* **patterns** (required): A list of strings following C++'s modified ECMAScript syntax
* **insensitive**: Whether or not the patterns are case-insensitive. Default: false

## Modifying and creating filters

As an obligatory note, it's strongly recommended not to directly edit existing filters. The install process will happily override them. It's therefore recommended that any modifications or new filters are named using the following convention:

```
hostname-service-optional qualifier.json
```

For example, `yourserver-sshd-aggressive.json`. This is better than simply `sshd-aggressive.json`, as a filter with this name may be introduced later, and result in yours being overwritten when updated.

**Note:** this does not apply to contributions directly to dnf2b's filter list. For contributions to dnf2b, use `service-optional qualifier.json` instead. It's also strongly encouraged to contribute directly to dnf2b with filters to common services, to allow more people to use them.

---

Filters are based purely on the message of the log entry. For example, consider the following `/var/log/auth.log` message:
```
2023-12-06T15:13:47.318578+01:00 nova sshd[24423]: Accepted publickey for olivia from 192.168.0.190 port 40976 ssh2: ED25519 SHA256:<hidden>
```

After being parsed by the journald parser, the message is determined to be "`Accepted publickey for olivia from 192.168.0.190 port 40976 ssh2: ED25519 SHA256:<hidden>`". This is the string that's passed on to specified filters, and when writing filters, this is what you should use to write patterns.

