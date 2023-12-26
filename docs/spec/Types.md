# Types

## Standard types

The majority of the standard types will not be documented here, unless there's a reason to. Standard types are generally self-explanatory, and used everywhere in both software development, and in system configuration.

However, some types abstract certain implementation details. This particularly applies to number types. In case there's ever a need for the exact limits, these are the underlying types used for numeric values:

* Integer: 64-bit integer
* Float: Double precision float

Additionally, there are certain meta types based on base types:

* Enum: strings with a restricted number of allowed values. See the field documentation for the specific allowed values.

## Duration

A duration is a string or a number representing a, shock, duration of time. Strings consist of a number with a duration format marker, which must be one of the following:

| Abbreviation | Meaning |
| --- | --- |
| `d` | Day |
| `w` | Week |
| `m` | Month, but with an asterisk. Due to how C++ internally handles months, it's not a month in the expected 28-31 days (though often fixed to 30 days in software), but rather, it's the average length of a month. It's still approximately a month, but if you're expecting exactly 30 days, you'll be disappointed. |

Hour may be added in the future, if there's any interest for it. Please open an issue (if one doesn't already exist) if you need support for hours (preferably with an explanation of what for).

Briefly explained, the format is interpreted in one of three  ways:
* **If number:** Interpreted as the number of days
* **If string, followed by a duration format marker:** Interpreted in accordance to the provided duration marker (see the table).
* **If string, not followed by a duration format marker:** Interpreted as a malformed number, i.e. interpreted as the number of days. Will produce a warning.

As a general rule, a duration does not allow negative values. Negative valued input in Duration fields is labelled as the specialised type NDuration.

## NDuration (Negative Duration)

A NDuration is the same as a Duration, but allows negative values that have a special meaning. For example, `core.control.banPeriod` is an NDuration, and if the provided value is negative, that's interpreted as the ban being permanent. See the documentation for Duration for the core format when the duration value is positive.

The exact side-effect of a NDuration will usually be disabling a feature, or making the duration permanent. See the documentation for the specific value for the exact effect of negative values.

## Regex

Regex fields are strings that contain regex patterns. Note that dnf2b uses [PCRE2](https://www.pcre.org/current/doc/html/pcre2syntax.html). [Regex101](https://regex101.com/) supports this syntax, and can be used to preview or test patterns.

**Warning:** Because the patterns are stored in JSON, which interprets characters with a backslash in front of it as a special character, all backspaces have to be properly escaped. If you're unsure how to do this, it's recommended to use `dnf2b filter-wizard`
