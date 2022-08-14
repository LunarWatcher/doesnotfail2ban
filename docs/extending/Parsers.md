# Parser definitions

Parsers are defined in JSON, as is tradition with this particular program.

The goal of a parser is incredibly simple: its goal is to take a logline, and extract a time, match it to an applicable process, and extract a message.

Take an (early version of) the journald parser as an example:
```json
{
    "type": "file",
    "pattern": {
        "full": ^"([^ ]+ ){3} [^ ]+ &p[\d]: (.*)$",
        "time": "%b %d %H:%M:%S",
        "groups": {
            "time": 0,
            "message": 1
        }
    }
}
```

We'll get back into what `&p` means later. What happens here is that it parses

## Types

At the moment, there's only one supported type; `file`. In the future, this may be amended to include sockets, or other systems. This is merely an indicator to the dnf2b core that the `file` declared in the watcher is of a specific type, and defines what type of parser to load up, as well as what systems dnf2b has to use to keep on top of new reports.

For future compatibility, this field has to be present, with its value set to `file`, or else.

## Patterns

The pattern object consists of three keys; `full`, `time`, and `groups`.

### `full`

`full` is the complete regex used to match a logline. For now, there isn't multiline support, because I've suffered through enough of that garbage in auto-pairs, and I've yet to see an application where a multi-line check is necessary.

`full` has to contain at least two capturing groups, one for the time and one for the message. There can be more groups if your particular use-case requires or benefits from multiple capturing groups.

#### Special parameters

As you might've noticed from the journald example, it's not pure regex. `&p` is a special parameter. The p is short for "process", and is replaced within the string prior to the regex check.

One of the reasons dnf2b gets away with so few parsers is precisely by separating the parser from the matcher, and this means opening for substantially more flexible parser options, to avoid needing several parsers that all do the same thing, but with one tiny thing tweaked that's unique to that specific process. The lack of message matching in the parser also means we can get away with precisely that.

##### `&p`

Replaced with the name of the process

### `groups`

**Important:** `groups` is zero-indexed and not one-indexed. `0` when used in regex typically refers to the whole match, but this is NOT the case with `groups`. The first capture group is labeled 0; dnf2b takes care of the transforming (adding 1) when processing for ease-of-use.

As previously mentioned, `full` has to contain at least two capturing groups; one for the message, and one for the time. These two are critical and used for various checks internally. However, there are problems related to group usage when this form of regex is used, including:

1. Which is which?
2. What if there are more than two groups?

Simply put, this is solved with the `groups` parameter. This map contains two sub-keys:
```json
"groups": {
    "time": 0,
    "message": 1
}
```

In many cases, time's value is less than that of the message value, which is all down to how time, in the vast majority of cases, is printed prior to the message. In many cases where trivial matching is possible, there's also no need for more than two groups, and consequently using this exact pattern. So unlses you have additional groups, or a message that comes before the time, `groups` is typically defined as shown above.


