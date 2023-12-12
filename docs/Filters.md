# Filters


## File syntax

```json
{
    "patterns": [
        "A list of PCRE2 patterns"
    ],
    "insensitive": false
}
```

* **patterns** (required): A list of strings following C++'s modified ECMAScript syntax
* **insensitive**: Whether or not the patterns are case-insensitive. Default: false

### Reserved capture group names

DNF2B uses a few internal capture groups. The regex for the groups are primarily exposed through variables like `${dnf2b.ip}`; see the [Special keys](#special-keys).

These may use special capture groups to extract content, and use named groups to do so. These group names are:

* `IP`

These group names must NOT be used within filters, unless writing explicit capture groups to replace the built-in groups, for whatever reason (which, as an aside, is strongly discouraged).

## Filter location and search system

All filters are stored in `/etc/dnf2b/filters/`.

By default, filters are searched in the following order:

1. `/etc/dnf2b/filters/{filtername}.json`
2. `/etc/dnf2b/custom/{subdirectory}/filters/{filtername}.json`, where `{subdirectory}` is determined by iterating the folders in the custom directory.

Due to this, custom directories do not have the same priority as the built-in filters in the event of a collision. Due to this, there are two different ways to declare a filter in a watcher:

1. `filtername` - uses the priority order. Errors out if the filter doesn't exist anywhere.
2. `@subdirectory/filtername` - looks for `filtername` **only** in `/etc/dnf2b/custom/{subdirectory}/filters`; generates an error if the folder or filter doesn't exist, even if a filter with the same name exists elsewhere.



## Adding third-party filters

Third-party filter directories can be added by cloning the GitHub repo into `/etc/dnf2b/custom`. General copypasta procedure:
```sh
# Only required if the folder doesn't already exist
sudo mkdir -p /etc/dnf2b/custom

cd /etc/dnf2b/custom
sudo git clone --depth=1 https://github.com/github/repo-name
```

## Modifying and creating filters

As an obligatory note, it's strongly recommended not to directly edit existing filters. The install process will happily override them. It's therefore recommended that any modifications or new filters are named using the following convention:

```
hostname-service-optional qualifier.json
```

For example, `yourserver-sshd-aggressive.json`. This is better than simply `sshd-aggressive.json`, as a filter with this name may be introduced later, and result in yours being overwritten when updated.

**Note:** this does not apply to contributions directly to dnf2b's filter list. For contributions to dnf2b, use `service-optional qualifier.json` instead. It's also strongly encouraged to contribute directly to dnf2b with filters to common services, to allow more people to use them.

---

Filters are based purely on the message of the log entry. For example, consider the following `/var/log/auth.log` message[^1]:
```
2023-12-06T15:13:47.318578+01:00 nova sshd[24423]: Accepted publickey for olivia from 192.168.0.190 port 40976 ssh2: ED25519 SHA256:<hidden>
```

After being parsed by the journald parser, the message is determined to be "`Accepted publickey for olivia from 192.168.0.190 port 40976 ssh2: ED25519 SHA256:<hidden>`". This is the string that's passed on to specified filters, and when writing filters, this is what you should use to write patterns.

If you wanted to match this particular line, you could write a filter file containing:
```json
{
    "patterns": [
        "^Accepted publickey for \\S+ from ${dnf2b.ip} port \\d+ ssh2: \S+ \S+$"
    ]
}
```


[^1]: it's worth noting that using /var/log/auth.log is discouraged due to the unreliable date format. The journald parser exists to fill this gap, though the message it returns is still identical to the example. 

### Special keys

| Key | Usage | Notes |
| --- | ----- | --- |
| `${dnf2b.ip}` | Used to signal an IP in the log output that should be used for ban purposes. This key is only required if the parser doesn't extract an IP from outside the message. | This key under the hood is just a `\S+` search, so the IP must be properly delimited by the rest of the regex. |

