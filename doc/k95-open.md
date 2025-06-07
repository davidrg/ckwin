Quick History of Open Source Kermit 95
======================================

When Kermit 95 was [originally open-sourced](https://www.kermitproject.org/k95sourcecode-orig.html) back in July 2011
some fairly major components were excluded both by accident and due to licensing issues. This included:

* All code required to build the GUI version (K95G)
* X/Y/Z-MODEM support (3rd-party code: 'P')
* The dialer (depends on 3rd-party code: Zinc)
* The OpenSSH-derived SSH subsystem (extremely out of date, possible issues with export regulations)

In October 2013 when there appeared to have been no (public at least) effort
towards a usable open-source Kermit 95 distribution, I (David Goodwin) purchased
a copy of Visual C++ 6.0 on ebay, grabbed the code and had a go at building it.
This is when missing code for K95G was discovered. I reached out to 
Frank da Cruz who was able to track it down and get the code to me resulting in
the creation of the github repository and an initial effort to get a freely 
redistributable version of Kermit 95 built.

The registration code and other unavailable things were fairly quickly removed, 
and the C-kermit code was upgraded to the then latest version, and Kermit 95 was
rebranded as C-Kermit for Windows. In early 2014 development efforts stalled due
to the difficulty of replacing the built-in SSH client - I wasn't much of a C
programmer and there appeared to be no easy solutions.

Over the following several years there was the occasional email or github fork,
but no real progress was made until Frank da Cruz go in touch again in 2022 with
[a workaround](https://www.kermitproject.org/k95relay.html) and a request for a
rebranded build of Kermit 95 that he could put on the website.

This build (["C-Kermit for Windows Beta 1"](https://github.com/davidrg/ckwin/releases/tag/10.0beta.04-1))
was quickly produced as it was pretty much just a tidyup of the work that had been
done nearly a decade prior. Once that was out of the way, it appeared that there
might be some easier options to get better SSH support in place. At first, 
Windows 10s ConPTY feature was investigated as Windows now included OpenSSH, 
before discovering libssh2 which provided fairly easy to get integrated. This 
resulted in ["C-Kermit for Windows Beta 2"](https://github.com/davidrg/ckwin/releases/tag/10.0beta.04-2),
the first free release with a built-in (though fairly buggy) SSH client.

Since then, all the originally excluded components have been released, rebuilt
or upgraded:

* The code for the GUI version was recovered by Frank and added in ([this commit](https://github.com/davidrg/ckwin/commit/870906a1e2b623fbf1283057ea25aca10ac8301e))
* The Dialer was open-sourced when it was discovered [Zinc had been open-sourced](http://openzinc.com/) ([this commit](https://github.com/davidrg/ckwin/commit/442aea927cd464d975b472324d2d64b3f0c7b494))
* With the permission of the original author (Jyrki Salmi of [Online Solutions Oy](https://www.online.fi/)), the X/Y/Z-MODEM library was open-sourced ([this commit](https://github.com/davidrg/ckwin/commit/da47177e9c78b9c35ec70a72fa395b613e64f0ba))
* An entirely new SSH subsystem has been built using libssh [starting in July 2022](https://github.com/davidrg/ckwin/commit/84c15909f4c44336ca7fa754edb9e7b9fb3f52df)
* OpenSSL and MIT Kerberos for Windows upgraded to the latest versions - this 
  turned out to be relatively easy thanks to most of the code being shared with 
  C-Kermit where it had been maintained over the last 20 years

This gets the Open-Source Kermit 95 somewhat closer to equivalence with K95 v2.1.3, though SSH and SSL/TLS features are
now excluded from Windows releases older than XP as that's the minimum OpenSSL and LibSSH can reasonably be built for.

Additionally, some features can no longer be supported in standard builds of K95:

* SSHv1: Not supported by LibSSH anymore, and not widely enough deployed to be worth finding an alternative solution
* SRP Authentication: Stanford SRP is no longer maintained and the last release is incompatible with any OpenSSL release still receiving security fixes
* Kerberos IV: No longer supported by current releases of MIT Kerberos for Windows
* SuperLAT and TES32: While the SDK is publicly available, the license on it is unclear

Except for SSHv1 support, code for all of these features still exists and CI regularly 
confirms it is still buildable, so it is still possible to produce a custom build of K95 with these
features if needed.
