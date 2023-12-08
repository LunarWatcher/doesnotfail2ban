# Iptables bouncer

## Options

* **strategy**: `DROP` or `REJECT`; used to define the action ban action. Defaults to DROP
* **ipset**: Determines whether or not the bouncer should use `ipset`. This will simplify rules, and may help performance compared to letting the number of rules grow large, but requires an additional package alongside iptables.
