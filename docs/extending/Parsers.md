# Parser definitions

Parsers are defined in JSON, as is tradition with this particular program.

The goal of a parser is incredibly simple: its goal is to take a logline, and extract a time, match it to an applicable process, and extract a message.

Take an (early version of) the journald parser as an example:
```json
{
    "type": "file",
    "multiprocess": true,
    "pattern": {
        "full": "^([^ ]+ ){3} ([^ ]+) ([^ ]+)[\d]: (.*)$",
        "time": "%b %d %H:%M:%S",
        "groups": {
            "time": 0,
            "host": 1,
            "process": 2,
            "message": 3
        }
    }
}
```


## Pipelines (or rather, lack thereof)

Parsers do not support a JSON-driven parser pipeline, with multiple steps. Each parser is expected to take a logline, and spew out a timestamp and a message. The result is directly passed on to the filters.

## Types

At the moment, there's only one supported type; `file`. In the future, this may be amended to include sockets, or other systems. This is merely an indicator to the dnf2b core that the `file` declared in the watcher is of a specific type, and defines what type of parser to load up, as well as what systems dnf2b has to use to keep on top of new reports.

For future compatibility, this field has to be present, with its value set to `file`, or else.

## multiprocess

If `multiprocess` is true, `process` has to be in `groups`, and in the output overall.

If it's false, `process` is optional, but it also restricts the parser source to a single consumer. Multiple processes using a single-process parser will all be notified about the same lines, and all associated watchers will run through all the associated matches.

## Patterns

The pattern object consists of three keys; `full`, `time`, and `groups`.

### `full`

`full` is the complete regex used to match a logline. For now, there isn't multiline support, because I've suffered through enough of that garbage in auto-pairs, and I've yet to see an application where a multi-line check is necessary.

`full` has to contain at least two capturing groups, one for the time and one for the message. There can be more groups if your particular use-case requires or benefits from multiple capturing groups.

### `groups`

**Important:** `groups` is zero-indexed and not one-indexed. `0` when used in regex typically refers to the whole match, but this is NOT the case with `groups`. The first capture group is labeled 0; dnf2b takes care of the transforming (adding 1) when processing for ease-of-use.

As previously mentioned, `full` has to contain at least two capturing groups; one for the message, and one for the time. These two are critical and used for various checks internally. However, there are problems related to group usage when this form of regex is used, including:

1. Which is which?
2. What if there are more than two groups?

Simply put, this is solved with the `groups` parameter. This map contains two sub-keys:
```json
"groups": {
    "time": 0,
    "host": 1,
    "process": 2,
    "message": 3
}
```

In many cases, time's value is less than that of the message value, which is all down to how time, in the vast majority of cases, is printed prior to the message. In many cases where trivial matching is possible, there's also no need for more than two groups, and consequently using this exact pattern. So unlses you have additional groups, or a message that comes before the time, `groups` is typically defined as shown above.

`host` and `process` are optional, but these disable `&h` and `&p` for filters. Note that `process` is only optional if `multiprocess` is false. This is for process identification reasons.
