# Why not use fail2ban?

As an obligatory note, this is a highly subjective and observational document. Your mileage may vary; if you're happy with fail2ban, keep using it.

## (partly?) no longer applicable: Python 2 

Up until as recently as 16.06.2023, [fail2ban was Python 2-based](https://github.com/fail2ban/fail2ban/commit/226a59445a8046b7f86c3bc072be1d78555ccdb0), with Python 3 support previously hinging on 2to3 to provide the very thin Python 3 support. With distros phasing out Python 2, this left 2to3 as the only real option for using it.

Another thing I had happen once was fail2ban quietly dying because I fucked around with my Python install. I'm not entirely clear on whether or not 2to3 or some other part of fail2ban is the source of this, however, and I don't feel like trying again. This resulted in some [not so fun stuff](https://media.discordapp.net/attachments/809850465537884170/997988883675492372/unknown.png).


## Failing to ban

This is one of the more ironic points, but fail2ban often lives up to its name in the wrong way. 

Certain things cannot be fully blamed on fail2ban; out-of-date filters and changes to systems resulting in different log formats happens to all systems. Particularly the apt version is exposed to this, as it's heavily out of date. Dnf2b is no exception to this. However, even seemingly with the right filters enabled, it occasionally blatantly ignored matches for reasons I'm not going to pretend to understand. This also applied to native services, so this is completely unrelated to the docker problem mentioned in the README.

## Permabans aren't permabans

Granted, there's configuration for this, but due to f2b's [storage machanics](https://serverfault.com/a/604236/569995), permabans risk not persisting by default.

## Final words

From what I've seen on the internet, I'm not the only one who has had problems with f2b. An alternative already exists ([CrowdSec](https://www.crowdsec.net)), but it follows a very different system to fail2ban and by extension, dnf2b. Again, if f2b works for you, use it. If it ain't broke, etc.
