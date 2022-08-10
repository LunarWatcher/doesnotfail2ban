# Jails and bouncers

Fail2ban revolves around a concept of jails; each jail has a process to monitor, along with relevant checks, and each jail is separate from everything else.

CrowdSec, on the other hand, uses a single, unified "jail", through bouncers. Each bouncer does a thing to block an IP in some way. The firewall bouncer blocks it entirely, while other bouncers rcover other areas, such as banning from a specific service. However, if an IP is banned, it's banned everywhere.

I prefer CrowdSec's approach, which is why dnf2b uses a similar bouncer model. It's also less data-intensive, which has advantages:

1. There's a single database used for all analytical purposes, compared to n different databases for n jails. Also means a single index for keeping track of unban dates, if applicable
2. The ban is fully separate from the jail; if an IP is deemed to be in violation of a central index, in the case of a multi-service device, it gets yeeted on its ass in all IPs, and it's handled by dedicated ban processors, all of which are separately enabled or disabled.*
3. Easier to code, easier to maintain.
4. Less work to unban yourself if you fucked up and got youself banned. Y'know, assuming you still have access somehow, for instance through hardware.

However, this is all at the expense of granular control. You can't just ban an IP from your SSH server, and still grant them access to a website, or an SSH tarpit (or other tarpit). However, when website bouncers get involved, you could have a website bouncer that only requires a captcha, and configure the firewall ban in such a way that it only blocks access to certain ports.

*: Will be anyway, if the project ever gets enough traction to gain more bouncers.
