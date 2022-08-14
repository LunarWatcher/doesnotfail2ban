# Configuration

## Watchers

Syntax:

```json
"watchers": {
    "watcher name": {
        "process": "the name of the process this watcher uses. If not specified, this uses the watcher name instead. This can consequently be omitted if the watcher is named after the process.",
        "enabled": true/false,
        "file": "path to the log file, or other thing where logs are acquired. May also be a URL, or any other scheme supported by the parser",
        "parser": "which parser to use. Do not include the file extension. The parser is loaded from /etc/dnf2b/parsers. `.json` is added automatically when looking for the file",
        "port": integer, defines which port the process runs on,
        "limit": integer, ban treshold,
        "filters": ["an array of stings containing which filters to use."]
    }
}
```

A watcher is simply a group of obligatory metadata, combined with a limit and a filter.
