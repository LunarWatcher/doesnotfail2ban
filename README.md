# Dnf2b

Because fail2ban does often fail to ban.

## Requirements

* A C++20 compiler
* A Linux server
* Systemd development package (`libsystemd-dev` on Debian)

## Current status

At this time, dnf2b is considered barely deployable, and barely functional. It has the bare minimum features required to function, but lacks many important features, and has a very thin ruleset. See also the Caveats section, which will be true even when features catch up and dnf2b becomes stable, and this notice disappears.

* [x] Core setup
* [x] Data storage
* [x] Unbanning
* [x] Incremental ban durations
* [ ] Date tracking to avoid excessive rescanning of files (and to better deal with logfile cycling)
* [ ] Thread safety stuff
* [ ] Fully usable CLI
* [ ] External rule repositories
* [ ] Proper documentation
* [ ] Base rules for supported services


## Caveats

Due to this being a significantly less established project than f2b, it's lacking in filters. Other alternatives offer a better out-of-the-box experience. The primary goal of this project is to provide a more stable system, at least on systems I'm able to test.

## Installing

In case you're fine with the warnings, installing has been conveniently placed in a script that you can find under `scripts/install.sh`. These commands will not be included separately in the readme, because they're already in that script file. TL;DR: it's `mkdir build && cd build && cmake .. && make && sudo make install`.

When you've read the file (as one always should with random scripts on the internet), you can run the commands by hand, or use
```bash
curl -L https://raw.githubusercontent.com/LunarWatcher/doesnotfail2ban/master/scripts/install.sh | bash
```

To automatically download and run the script. This will download the source, compile it, and install it to /opt/dnf2b, along with copying the etc folder into /etc (along with a systemd service for your convenience).

Note that before the service can be started, `config.local.json` needs to be created. This is documented in docs/Config.md.

After you're done, you can delete the source directory, unless you want to keep it for quicker builds or something thin like that.

After configuring, you can run dnf2b with:
```
sudo systemctl start dnf2b 
```

### Adding to the PATH

Due to the install folder being in `/opt/dnf2b/` by default, dnf2b's binary won't be added to the PATH automatically. If you don't want to type `/opt/dnf2b/bin/dnf2b` as the command if you want to access the CLI interface, you can run
```
sudo ln -s /opt/dnf2b/bin/dnf2b /usr/local/bin
```

Or, of course, adding `/opt/dnf2b/bin` to your PATH.

### Getting started

See docs/Config.md for help with the configuration.

The docs are still very new. If they don't make sense, lack information, or you otherwise can't figure out how to make dnf2b do what you want, you can [browse existing or start a new discussion](https://github.com/LunarWatcher/doesnotfail2ban/discussions) here on GitHub.

### Updating

If you've done no changes to the default files (i.e. files installed by dnf2b, NOT files such as config.local.json, or custom files added alongside the default files), just re-run the `curl` command from the installation section. If you kept the cloned directory, you can also run `git pull && cd build && make -j $(nproc) && sudo make -j $(nproc) install` manually. This may not update the dependencies, however.

If you've made changes to default files, you get to back up everything you've done first. There's currently no easy way around this at this time.


## License?

MIT; see the LICENSE file.
