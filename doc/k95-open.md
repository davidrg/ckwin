Restoring features in the Kermit 95 Open-Source Release
=======================================================

When Kermit 95 was [originally open-sourced](https://www.kermitproject.org/k95sourcecode-orig.html) back in July 2011
some fairly major components were excluded both by accident and due to licensing issues. This included:

* All code required to build the GUI version (K95G)
* X/Y/Z-MODEM support (3rd-party code: 'P')
* The dialer (depends on 3rd-party code: Zinc)
* The SSH subsystem (extremely out of date, possible issues with export regulations)

Since that time, and mostly since 2022, a number of the originally excluded components have been released and
everything else upgraded or rebuilt where possible:

* The code for the GUI version ([this commit](https://github.com/davidrg/ckwin/commit/870906a1e2b623fbf1283057ea25aca10ac8301e))
* The Dialer was open-sourced when it was discovered Zinc had been open-sourced ([this commit](https://github.com/davidrg/ckwin/commit/442aea927cd464d975b472324d2d64b3f0c7b494))
* With the permission of the original author (Jyrki Salmi of Online Solutions Oy), the X/Y/Z-MODEM library was open-sourced ([this commit](https://github.com/davidrg/ckwin/commit/da47177e9c78b9c35ec70a72fa395b613e64f0ba))
* An entirely new SSH subsystem has been built using libssh
* OpenSSL and MIT Kerberos for Windows upgraded to the latest versions

This gets the Open-Source Kermit 95 somewhat closer to equivalence with K95 v2.1.3, though SSH and SSL/TLS features are
now excluded from Windows releases older than XP as thats the minimum OpenSSL and LibSSH can reasonably be built for.

Additionally some features can no longer be supported in standard builds of K95:

* SSHv1: Not supported by LibSSH anymore, and not widely enough deployed to be worth finding an alternative solution
* SRP Authentication: Stanford SRP is no longer maintained and the last release is incompatible with any OpenSSL release still receiving security fixes
* Kerberos IV: No longer supported by current releases of MIT Kerberos for Windows
* SuperLAT and TES32: While the SDK is publicly available, the license on it is unclear

With the exception of SSHv1 support, code for all of these features still exists and CI regularly 
confirms it is still buildable, so it is still possible to produce custom builds of K95 with these
features if needed.
