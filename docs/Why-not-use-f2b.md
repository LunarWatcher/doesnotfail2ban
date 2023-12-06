# Why not use fail2ban?

As an obligatory note, this is a highly subjective and observational document. Your mileage may vary; if you're happy with fail2ban, keep using it.

## (partly?) no longer applicable: Python 2 

Up until as recently as 16.06.2023, [fail2ban was Python 2-based](https://github.com/fail2ban/fail2ban/commit/226a59445a8046b7f86c3bc072be1d78555ccdb0), with Python 3 support previously hinging on 2to3 to provide the very thin Python 3 support. With distros phasing out Python 2, this left 2to3 as the only real option for using it.

Another thing I had happen once was fail2ban quietly dying because I fucked around with my Python install. I'm not entirely clear on whether or not 2to3 or some other part of fail2ban is the source of this, however, and I don't feel like trying again. This resulted in some [not so fun stuff](https://media.discordapp.net/attachments/809850465537884170/997988883675492372/unknown.png).


## Failing to ban

This is one of the more ironic points, but fail2ban often lives up to its name in the wrong way. 

A significant number of log entries slipped past f2b, because the apt version is _grossly_ out of date. Admittedly, this is more the fault of the debian philosophy than f2b. However at the time I used it, its use of Python 2 also made updating it manually a massive pain in the ass.

Certain things cannot be fully blamed on fail2ban; out-of-date filters and changes to systems resulting in different log formats happens to all systems. Dnf2b is no exception to this. However, even seemingly with the right filters enabled, it occasionally blatantly ignored matches for reasons I'm not going to pretend to understand.
