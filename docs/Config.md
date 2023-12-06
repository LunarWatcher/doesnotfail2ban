# Configuring dnf2b

The configuration file for dnf2b is `/etc/dnf2b/config.local.json`. This file does not exist by default, and must be created before using dnf2b. A template exists in `/etc/dnf2b/config.json`. It's recommended to base new configurations off this template.

This document contains general information about the configuration system, and how-tos. If you just want to see the available configuration options and their permitted values, see docs/spec/Config.md.

## Configuration ideology

For (former?) f2b users, it's worth noting that dnf2b has a much more global config and handling system. For example, failing a watcher with the iptables bouncer results in an iptables ban on all ports. Additionally, the pool of failures is shared across all watchers. 

Consider two watchers with a limit of 3. If some IP trips the first watcher twice, and then the second watcher once, the fail with the second watcher immediately trips the bouncer, as if that watcher had been tripped 3 times.

Additionally, certain bouncers are global by design. This includes the currentely only bouncer, iptables, as the port-specific bans are protocol-specific, requiring 5 different commands<sup>1</sup> rather than 1 for a global ban.

The design is also intended to allow for other kinds of bouncers, such as notifying a webapp to take spam prevention actions in response to multiple password failures. However, at this time, no such bouncers are implemented, because I have no clue what bouncers are required to work for most webapps with such functionality in the wild.


<sup>1</sup>: Using iptables 1.8.7 (nf_tables) has 5 possible values for protocols: tcp, udp, udplite, sctp, and dccp. While not all services accept all these protocols, a partial block does not a complete port block make.

## Configuring bouncers

The global bouncer configuration defines what bouncers should be available for watchers to use, as well as overall config. For a watcher to be able to use a bouncer, the bouncer **must** be added to the global `bouncers` key. For example:

```json
"bouncers": {
    "iptables": {
        "strategy": "DROP"
    }
},
```

In this case, the `iptables` key is made available for the watchers to use. The other valid strategy key for iptables is `REJECT`. However, `DROP` is default to make it more difficult for the blocked party to tell if they have  been caught, or if the service just happened to go down. Whether or not this makes any difference in practice is unclear at best.

