# Journald parser

The journald parser is a special parser using the systemd API for content.

It does this to avoid reading files like `/var/log/auth.log`, for two reasons:
1. The date formats aren't universal, making parsing them a fucking pain in the ass
2. Newer versions of debian [may or may not use the file at all](https://github.com/fail2ban/fail2ban/issues/3645)

Using the systemd API ensures that whatever format the files may be in, dnf2b gets machine-interpretable data.

## Options

* **idMethod** [enum("syslog", "systemd_unit")]: The process identification method to use. The meanings should probably be documented in detail, but that's a problem for future me

### Example
```
{
    "type": "journalctl",
    "idMethod": "syslog"
}
```

