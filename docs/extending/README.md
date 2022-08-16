# Extending base features

This directory contains documentation on extending and customizing elementary features, as permitted by the code. This is not automatically a substitute for a feature request. If you, for instance, have a company-private system, but that's exposed to the internet, there aren't any filters built in. There may not be any parsers either, at least not if the logging system doesn't happen to overlap with an existing parser.

You may also have noticed a different pattern that you personally want to ban, or a missing critical filter worthy of a pull request for integration.

What this system doesn't do is let you add brand new types of input, or complex new matches that require caching to work. JSON only gets you so far; the more complicated stuff has to be done in C++, and is not documented here.
