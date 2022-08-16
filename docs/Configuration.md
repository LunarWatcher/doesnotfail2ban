# Configuration

## The Wizard

The wizard bootstraps your configuration with a set of watchers, and tries to detect available bouncers as well. However, the wizard isn't perfect, and may not find something you actually have.

Get started with `sudo dnf2b wizard`

Note that you can re-run the wizard at a later time. Your existing config is preserved, as long as it doesn't conflict with the addition or removal of a bouncer or watcher.

## General structure and file location
The file used by dnf2b is `/etc/dnf2b/config.local.json`. The wizard automatically creates this file. If you skip the wizard, you have eto copy `config.json` over to `config.local.json`. `config.json` is never used on the live instance, and exists mostly to preserve the defaults.

This also means `config.json` is overridden every time dnf2b is updated. `config.local.json` is not.

```json
{

    "core": {
        "control": {
            ...
        },
        "stats": {
            ...
        }
    },
    "watchers": [
        ...
    ],
    "bouncers": [
        ...
    ],
    "communicators": [
        ...
    ],
    "excempt": [
        ...
    ]
}
```

## Core

### Control

### Stats

## Watchers

Syntax:

```json
"watchers": [
    {
        "process": "the name of the process this watcher uses. For sshd, for instance, this field should be set to sshd",
        "enabled": true/false,
        "file": "path to the log file, or other thing where logs are acquired. May also be a URL, or any other scheme supported by the parser",
        "parser": "which parser to use. Do not include the file extension. The parser is loaded from /etc/dnf2b/parsers. `.json` is added automatically when looking for the file",
        "port": integer, defines which port the process runs on,
        "limit": integer, ban treshold,
        "filters": ["an array of stings containing which filters to use."]
    }
]
```

A watcher is simply a group of obligatory metadata, combined with a limit and a filter.

The main difference here from f2b is that the process is explicitly and separately defined.


## Bouncers

## Communicators


## Excempt

`excempt` defines a list of IPs and/or IP ranges to completely skip. IPs in this list, or globbed in by ranges in this list, cannot be banned.

Note that `127.0.0.1` and `::1` are in this list by default, and cannot be removed. The list additionally comes with LAN IPs whitelisted by default, though these are not hard-coded, and can be removed.
