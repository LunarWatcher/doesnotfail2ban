# WARNING: Currently incomplete

Log reading and banning has not been hooked up yet. Beyond installing and running dummy commands and debug output, this project isn't yet ready to be deployed on real servers. Come back later!

If you decide to ignore the warning not to use dnf2b until it's ready and use it anyway, note that everything is unstable, and can and will change without notice until it's release-ready. A release will be published when the project is usable.

# doesnotfail2ban

## TL;DR:

I need a system that works, because the internet is a dangerous place. Fail2ban failed 2 provide that, forcing me to switch. I switched to CrowdSec, and while I certainly see the advantages of crowd-sourced (and consequently pre-emptive) bans, it failed to provide me with the config and flexibility I need for the type of protection I need.

## Why?

Because fail2ban is flaky at best, and crowdsec doesn't seem to either provide the coverage or configurability I need to insta-ban offending IPs from my server. Also because it turns out that rolling my own is easier than finding functional and maintained options with easy configuration. Who knew?

## What?

* Fail2ban but functional, and primarily intended (and at the time of the initial release, exclusively) for SSH protection, because all my services are protected behind SSH, making protecting it my highest priority
    * ... but still theoretically expandable to support anything
* Data-centric, because I love graphs and data analysis
* Linux-based, because if you're running a Windows server, you're already doing something wrong
* Similar structure to CrowdSec in terms of code structure; there's the core, detectors, parsers, and bouncers. However, the bouncers and parsers have been integrated closer into the code, because I don't feel like defining an entire shared object API to integrate external components. The detector scripts are purely regex and other config, and consequently stored separately.
* ... but all of that aside, functional. Clear configuration systems, and up-to-date documentation are core goals both CrowdSec and _especially_ fail2ban fail at.

## Differences

Note that this primarily applies when dnf2b starts reaching maturity; prior to that, this table shows what's planned

### Features
|  | [f2b](https://github.com/fail2ban/fail2ban) | [CrowdSec](https://github.com/crowdsecurity/crowdsec) | dnf2b |
| --- | --- | --- | --- |
| Public cloud-based | ❌ | ✔️ | ❌ -- though a sync protocol is considered, this cannot and will not match CrowdSec because hosting limitations. Distant future regardless |
| Configurable | ✔️ | ~ | ✔️  |
| Easily configurable | ~ -- not the most intuitive setup, several config options are out of date with its own documentation | ❌ | ✔️ |
| Reliable | ❌ | ~ | ✔️ |
| Data-centric | ❌ | ✔️ -- limited data available, pricing plans further reduce data availability | ✔️ -- and fully configurable|
| Alerts | ❌ | ✔️| ✔️ -- several configuration options available as well |
| Maintained | ~ -- heavily reduced activity in recent years, PRs and issues piling up | ✔️ | ✔️ -- though not necessarily with continuous development, because I have limited resources. |
| Health checks | ? | ❌ | ✔️ |
| Easily debuggable | ❌ | ❌ | ✔️ |
| Doesn't rely on a deprecated Python tool to be runnable on Python 3 | ❌ | ✔️ |  ✔️ | 

### General

|  | f2b | CrowdSec | dnf2b |
| --- | --- | --- | --- |
| Source language | Python | Go | C++17 |
| Config format | `.conf` | YAML | JSON |


## How?

See the docs folder

## License?

MIT; see the LICENSE file.
