# Substitutes

dnf2b offers certain keys that are automatically replaced with various data. There's two types of substitutes; global ones, i.e. substitutes that work in all substitution-enabled systems, and type-specific substitutes, such as substitutes specific to parsers, or specific to matchers.

## Syntax

There's three syntax elements:

* `&k`, where `k` is some supported letter. These are guaranteed to be a single ASCII letter
* `&(multi_byte_key)`, where `multi_byte_key` is some supported key. Note that these are exceptionally rare.
* `&&`, a literal `&`. Due to the use of `&` in substitutions, and the very real use of `&` in real-world scenarios, `&` may appear in regexes. The literal `&` is consequently written as `&&`.

## Global

### `&p`

Replaced with the process, as defined by a watcher.
