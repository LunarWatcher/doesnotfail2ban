# Dnf2b

Because fail2ban does often fail to ban.

## Requirements

* A C++20 compiler
* A Linux server

## Current status

At this time, dnf2b is considered barely deployable, and barely functional. It has the bare minimum features required to function, but lacks many important features, and has a very thin ruleset. See also the Caveats section, which will be true even when features catch up and dnf2b becomes stable, and this notice disappears.

## Caveats

Due to this being a significantly less established project than f2b, it's lacking in filters. Other alternatives offer a better out-of-the-box experience. The primary goal of this project is to provide a more stable system, at least on systems I'm able to test.

## Installing

In case you're fine with the warnings, installing has been conveniently placed in a script that you can find under `scripts/install.sh`. These commands will not be included separately in the readme, because they're already in that script file. TL;DR: it's `mkdir build && cd build && cmake .. && make && sudo make install`.

When you've read the file (as one always should with random scripts on the internet), you can run the commands by hand, or use
```bash
curl -L https://raw.githubusercontent.com/LunarWatcher/doesnotfail2ban/master/scripts/install.sh | bash
```

To automatically download and run the script.

## License?

MIT; see the LICENSE file.
