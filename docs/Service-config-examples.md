# Service configuration examples

This file contains example watchers for different services. Note that the meaning of the different fields, as well as details about writing your own watchers from scratch, are covered in `docs/Config.md`.

Note that the watchers in this file use all available stock filters. This includes aggressive filters that may or may not be undesirable. You'll usually want to configure the templates here to meet your system needs.

## Sshd
```json
{
    "id": "sshd",
    "process": "sshd",
    "enabled": true,
    "parser": "sshd",
    "filters": ["sshd-bruteforce", "sshd", "sshd-aggressive"],
    "banaction": "iptables"
}
```
## Nginx

```json
{
    "id": "nginx-access",
    "file": "/var/log/nginx/access.log",
    "parser": "nginx-access",
    "enabled": "true",
    "filters": ["nginx-auth", "nginx-badactors", "nginx-nmap"],
    "banaction": "iptables"
}
```
