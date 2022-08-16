# Terminology


## Bouncer

Bouncers are in charge of taking action against violations. The core logic takes care of matching and overall detection of bad actors, and send a signal to the bouncers to take care of the problem.

As an aside, dnf2b does not currently offer fine-grained control over bouncers. All bouncers are notified of violations in any watcher. This may change in the future if dnf2b gains traction, and there's interest in implementing this.

Until then, one bouncer may accidentally take over work others have. For instance, a firewall bouncer will replace all other bouncers until granular config is in place.

## Communicator

A communicator is in charge of sending alerts when the system overall detects activity above user-defined thresholds. These can be used if you, for instance, want to set up an alert if 50 or more IPs are banned in the last hour, or if other statistics engines hit their targets. See the documentation on communicators for more information.

## Watcher

A bunch of metadata specifying a process to keep an eye on.
