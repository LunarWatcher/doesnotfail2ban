# Configuring dnf2b

The configuration file for dnf2b is `/etc/dnf2b/config.local.json`. This file does not exist by default, and must be created before using dnf2b. A template exists in `/etc/dnf2b/config.json`. It's recommended to base new configurations off this template.

This document contains general information about the configuration system, and how-tos. If you just want to see the available configuration options and their permitted values, see docs/spec/Config.md.

## Configuration ideology

For (former?) f2b users, it's worth noting that dnf2b has a much more global config and handling system. For example, failing a watcher with the iptables bouncer results in an iptables ban on all ports. Additionally, the pool of failures is shared across all watchers. 

Consider two watchers with a limit of 3. If some IP trips the first watcher twice, and then the second watcher once, the fail with the second watcher immediately invokes the bouncer, as if that watcher had been tripped 3 times.

Additionally, certain bouncers are global by design. This includes the currentely only bouncer, iptables, as the port-specific bans are protocol-specific, requiring 5 different commands<sup>1</sup> rather than 1 for a global ban.

There are definite advantages and disadvantages to this, compared to the very granular configuration you get with f2b. Aside reducing configuration complexity, part of idea is that an attack on one service is an attack against them all. There's no point in separating fails and ban actions if better security can be achieved by blocking all attack surfaces at once. Granted, this may not be optimal for certain public-facing services (particularly when used in a commercial context), but this is also down to the set of bouncers used.

In the future, a goal is to include more bouncers that act on specific services. For example, notifying a webservice about a possible brute-force attempt, and making it show a captcha (with the bouncer naturally just being the notification bit). If this is a use-case you have, please consider opening an issue with technical details about the notification system.

<sup>1</sup>: Using iptables 1.8.7 (nf_tables) has 5 possible values for protocols: tcp, udp, udplite, sctp, and dccp. While not all services accept all these protocols, a partial block does not a complete port block make. Reducing the number of unnecessary iptable rules is a net positive for performance.

## Basic setup

As previously mentioned, the easiest way to bootstrap your config is to:
```
cd /etc/dnf2b
sudo cp config.json config.local.json
```

After this, you need to configure your services and bouncers.

### Configuring watchers

Watchers, as the name indicates, watch logs for problems. Sshd is included as an example of this in the template you just copied from. There are a few valid options:

* **id** [string]: required, must be unique. Does not need to correspond to the name of the process
* **process** [string]: optional, except for certain parsers (such as journald). Used to identify the process in logfiles used by multiple processes
* **enabled** [bool]: true/false, self-explanatory. Defaults to true, doesn't need to be present
* **parser** [string]: required, defines what parser to use. For ssh, and many other systems, journald is used. Others, such as nginx, use different formats, and therefore have different parsers. Parsers are documented elsewhere.
* **filters** [string]: An array of filters to use. Corresponds to filenames in /etc/dnf2b/filters.
* **limit** [number]: optional number, defaults to core.control.maxAttempts if not provided. Like the global limit, if set to 0 or less, offending IPs are instantly banned.
* **banaction** [string]: Which bouncer to use when the limit has been exceeded (note that, as previosuly mentioned, the counter is global even if the limit is local)


### Configuring bouncers

Bouncers are in charge of taking action against IPs that reach the limit and get banned.

The global bouncer configuration defines what bouncers should be available for watchers to use, as well as overall config. For a watcher to be able to use a bouncer, the bouncer **must** be added to the global `bouncers` key. For example:

```json
"bouncers": {
    "iptables": {
        "ipset": true
    }
},
```

In this case, the `iptables` key is made available for the watchers to use, and it's also instructed to use `ipset` instead of storing each ban as an iptable rule. Note that `ipset` needs to be installed separately for this to work.

Some bouncers, including iptables, do not have mandatory configuration options. See the documentation for the bouncer you want to use to learn more about the available (and if applicable, mandatory) configuration options.

### Configuring ban controls

Ban controls are under `core.control`, and contains four keys:

* **maxAttempts** [Integer]: the number of allowed attempts before banning. Note that the watchers have individual limits as well, meaning this represents the default for watchers without an explicit limit
* **banPeriod** [NDuration]: A string containing a ban period. If set to a number instead, the number is interpreted as days. If negative, the ban is permanent.
* **banIncrement** [Integer]: The factor the banPeriod is multiplied with for multiple fails. If set to 2 when the ban period is 1 week, that means the first ban is 1 week, the second is two weeks, the third is four, etc.
* **forgetAfter** [Duration]: How long it takes before fails are forgiven. To avoid excessive data storage, this has to be a finite value. Hwoever, for those that never want to forgive fails, setting this value high (for example multiple years) has the same effect.

Additionally, under just `core`, there's an additional key:
* **whitelist**: A list of IPs or CIDR ranges that are excluded. It's STRONGLY recommended you include IPs you frequent here, especially if the IPs are static. If possible, whitelisting the entire `192.168.0.0/16` range (or equivalent if your router uses one of the other two localhost ranges instead; refer to your router's DHCP config for more information) is recommended. If you use SSH to access the server and don't whitelist yourself, and you have an aggressive configuration, you risk locking yourself out if you don't add a whitelist entry. This is a mild annoyance when you're in the same building as the server, but quite a bit worse if cloud use is involved (though there are other ways around this, such as switching to a different IP).

### Communicators

Communicators are an optional feature that serve configurable notifications about various events. 

## Advanced configuration

### Support for non-builtins

The only limitation for non-builtin services is the parsers. As long as dnf2b can parse your log files or other kind of output, you can usually write filters for that output.[^1]

As an obligatory reminder, if you're writing filters for common services to fill a gap in the built-in filters, a pull request is appreciated.

[^1]: Emphasis on usually. Log formats with multiline output (i.e. logging the problem on one line and the source on a new, separate line that isn't part of the message) are is much trickier to parse, and by extension, write rules for. This is particularly true in multithreaded environments, where message order isn't guaranteed. Newlines within a log message can also cause problems, but this is more a much bigger problem with the file parser than the journald parser, due to how journald data is accessed. 


