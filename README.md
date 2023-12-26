# Dnf2b

Because fail2ban does often fail to ban.

## Requirements

* A C++20 compiler
* A Linux server
* Systemd development package (`libsystemd-dev` on Debian)

## Current status

At this time, dnf2b is considered barely deployable, and barely functional. It has the bare minimum features required to function, but lacks many important features, and has a very thin ruleset. See also the Caveats section, which will be true even when features catch up and dnf2b becomes stable, and this notice disappears.

**WARNING:** Due to dnf2b being in an early development phase and subject to lots of refactoring, there's information that's out of date, information that is written in advance and not yet true, and things that are going to change breakingly. 

* [x] Core setup
* [x] Data storage
* [x] Unbanning
* [x] Incremental ban durations
* [x] Thread safety stuff
* [ ] Fully usable CLI
    * [ ] Config and filter validation
    * [x] Filter wizard
    * [ ] Filter debugger
* [x] External rule repository support
* [ ] Proper documentation
* [ ] Base rules for supported services
    * [x] sshd
    * [ ] nginx
* [x] Journald API integration


## Caveats

### Filter availability
Due to this being a significantly less established project than f2b, it's lacking in filters. Other alternatives offer a better out-of-the-box experience. The primary goal of this project is to provide a more stable system, at least on systems I'm able to test.

### Docker and iptables

Because [Docker hates firewalls](https://docs.docker.com/network/packet-filtering-firewalls/#docker-and-ufw) (oversimplified), there's a significant chance that dnf2b's iptable rules will be ignored when forwarding to docker containers.

This is not dnf2b's fault; it's Docker's fault. Due to how Docker works[^1], it can (and happily will) bypass other rules, including UFW and general iptable rules. In fact, this problem also affects fail2ban, and requires [explicit config](https://serverfault.com/a/1044788/569995) to get it to behave. 

dnf2b does not make any attempt to insert itself before docker. Attempting to guarantee insertion prior to docker is an exercise in unnecessary pain. If docker either starts before dnf2b, or restarts after dnf2b has started, dnf2b's rule will be treated identically to 

As a result, it's strongly recommended that you either:

1. Don't use docker for exposed services; and/or
2. Use a reverse proxy (such as nginx) to forward to the docker containers. By doing so, you have a non-docker layer between the user and your docker containers, and this layer actually respects iptable rules. Note that for this to work, nginx **cannot** be installed as a docker container, or you're right back to the same problem of exposing docker containers

Alternatively, you can also fuck around with iptables and hope you manage to get a rule that docker doesn't overwrite. This is non-trivial to do, and personally, I found it orders of magnitude easier to just not use docker, and use nginx for the few services that are docker-centric. Your mileage may vary, though.

It is theoretically possible to fix this by adding a rule to the `DOCKER-USER` chain, but it involves a bunch of crap that I have no clue how work, and the alternatives to figuring out how to apply a firewall to docker traffic (i.e. reverse proxy and not using docker) are significantly easier.


### Docker and dnf2b

Without using a reverse proxy (nginx logs are great for a lot of the detections) that itself is not dockerised, you need to provide a way for dnf2b to access the logs from your docker containers for the various services.

This is left as an exercise to the reader, as the process for doing so varies from service to service. You can in theory use the [journald logging driver](https://docs.docker.com/config/containers/logging/journald/), but this is untested and will likely have multiple bugs.

Dnf2b itself is not available as a docker container, and there is no official support for running dnf2b within a docker container. If you try to do so anyway, you're on your own.

## Installing

In case you're fine with the warnings, installing has been conveniently placed in a script that you can find under `scripts/install.sh`. These commands will not be included separately in the readme, because they're already in that script file. TL;DR: it's `mkdir build && cd build && cmake .. && make && sudo make install`.

When you've read the file (as one always should with random scripts on the internet), you can run the commands by hand, or use
```bash
curl -L https://raw.githubusercontent.com/LunarWatcher/doesnotfail2ban/master/scripts/install.sh | bash
```

To automatically download and run the script. This will download the source, compile it, and install it to /opt/dnf2b, along with copying the etc folder into /etc (along with a systemd service for your convenience).

Note that before the service can be started, `config.local.json` needs to be created. This is documented in docs/Config.md.

After you're done, you can delete the source directory, unless you want to keep it for possibly quicker builds.

After configuring, you can run dnf2b with:
```bash
sudo systemctl start dnf2b 
```

### Optional: running tests

Dnf2b comes with several tests that can be run as well. To run them, run `make test`. 

Note that, by default, integration tests are not included. To get integration tests, `-DINTEGRATION=ON` has to be passsed to CMake. These are disabled by default, as the tests involve writing to, among other things, journald[^2]. Due to journald quirks, the written logs cannot be trivially erased without also erasing other logs, which is so nuclear  that this is not automatically performed.

If you don't want stuff written to your logs, do not run integration tests.

### Adding to the PATH

Due to the install folder being in `/opt/dnf2b/` by default, dnf2b's binary won't be added to the PATH automatically. If you don't want to type `/opt/dnf2b/bin/dnf2b` as the command if you want to access the CLI interface, you can run
```bash
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

[^1]: Docker prepends its rules to iptables. I can't tell if ufw does too (though I half doubt it), but I did try prepending the dnf2b rules in FORWARD and INPUT, and Docker happily placed itself in front of them when it (re)started. This would be a lot easier to work around if Docker didn't try to put itself in front of other firewall rules, but here we are.
[^2]: This might not matter for you in practice, as a couple lines of logs being written usually isn't going to mean shit, but it's disabled by default to avoid problems in small and hypothetical scenarios.
