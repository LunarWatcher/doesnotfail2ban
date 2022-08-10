# doesnotfail2ban

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
|  | f2b | CrowdSec | dnf2b |
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

### General

|  | f2b | CrowdSec | dnf2b |
| --- | --- | --- | --- |
| Source language | Python | Go | C++17 |
| Config format | `.conf` | YAML | YAML |


## How?

See the docs folder

## License?

MIT; see the LICENSE file.
