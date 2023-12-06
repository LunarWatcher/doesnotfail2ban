# Configuring dnf2b

The configuration file for dnf2b is `/etc/dnf2b/config.local.json`. This file does not exist by default, and must be created before using dnf2b. A template exists in `/etc/dnf2b/config.json`. It's recommended to base new configurations off this template.

This document contains general information about the configuration system, and how-tos. If you just want to see the available configuration options and their permitted values, see docs/spec/Config.md.

## Configuration ideology

For (former?) f2b users, it's worth noting that dnf2b has a much more global config and handling system. For example, failing a watcher with the iptables bouncer results in an iptables ban on all ports. Additionally, the pool of failures is shared across all watchers. 

Consider two watchers with a limit of 3. If some IP trips the first watcher twice, and then the second watcher once, the fail with the second watcher immediately trips the bouncer, as if that watcher had been tripped 3 times.

Additionally, certain bouncers are global by design. This includes the currentely only bouncer, iptables, as the port-specific bans are protocol-specific, requiring 5 different commands<sup>1</sup> rather than 1 for a global ban.

The design is also intended to allow for other kinds of bouncers, such as notifying a webapp to take spam prevention actions in response to multiple password failures. However, at this time, no such bouncers are implemented, because I have no clue what bouncers are required to work for most webapps with such functionality in the wild.


<sup>1</sup>: Using iptables 1.8.7 (nf_tables) has 5 possible values for protocols: tcp, udp, udplite, sctp, and dccp. While not all services accept all these protocols, a partial block does not a complete port block make.

## Basic setup

As previously mentioned, the easiest way to bootstrap your config is to:
```
cd /etc/dnf2b
sudo cp config.json config.local.json
```

After this, you need to configure your services and bouncers.

### Configuring watchers

Watchers, as the name indicates, watch logs for problems. Sshd is included as an example of this in the template you just copied from. There are a few valid options:

* **id**: required, must be unique. Does not need to correspond to the name of the process
* **process**: optional, except for certain parsers (such as journald). Used to identify the process in logfiles used by multiple processes
* **enabled**: true/false, self-explanatory. Defaults to true, doesn't need to be present
* **parser**: required, defines what parser to use. For ssh, and many other systems, journald is used. Others, such as nginx, use different formats, and therefore have different parsers. Parsers are documented elsewhere.
* **filters**: An array of filters to use. Corresponds to filenames in /etc/dnf2b/filters.
* **limit**: optional number, defaults to core.control.maxAttempts if not provided. Like the global limit, if set to 0 or less, offending IPs are instantly banned.
* **banaction**: Which bouncer to use when the limit has been exceeded (note that, as previosuly mentioned, the counter is global even if the limit is local)


### Configuring bouncers

The global bouncer configuration defines what bouncers should be available for watchers to use, as well as overall config. For a watcher to be able to use a bouncer, the bouncer **must** be added to the global `bouncers` key. For example:

```json
"bouncers": {
    "iptables": {
        "strategy": "DROP"
    }
},
```

In this case, the `iptables` key is made available for the watchers to use. The other valid strategy key for iptables is `REJECT`. However, `DROP` is default to make it more difficult for the blocked party to tell if they have  been caught, or if the service just happened to go down. Whether or not this makes any difference in practice is unclear at best.

### Configuring ban controls

Ban controls are under `core.control`, and contains four keys:

* **maxAttempts**: the number of allowed attempts before banning,
* **banPeriod**: A string containing a ban period. Valid postfixes currently include d (days), w (weeks), and m (months). If set to a number instead, the number is interpreted as days. If negative, the ban is permanent.
* **inc**: The factor the banPeriod is multiplied with for multiple fails. If set to 2 when the ban period is 1 week, that means the first ban is 1 week, the second is two weeks, the third is four, etc.

Additionally, under just `core`, there's an additional key:
* **whitelist**: A list of IPs or CIDR ranges that are excluded. It's STRONGLY recommended you include IPs you frequent here, especially if the IPs are static. If possible, whitelisting the entire `192.168.0.0/16` range (or equivalent) is recommended. If you use SSH and don't whitelist yourself, and you have an aggressive configuration, you risk locking yourself out otherwise. This is a mild annoyance when you're in the same building as the server, but quite a bit worse if cloud use is involved.

### Communicators

TBA
