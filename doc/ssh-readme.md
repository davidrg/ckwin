# SSH Support in Kermit 95 3.0 for Windows

Kermit 95 3.0 now includes a new built-in SSH support based on libssh[^1]. This
is only available on Windows and is still a work-in progress so not all SSH 
commands supported by Kermit 95 1.1.21 and 2.x are available in K95 3.0 at this
time. Bugs are likely so if you find one, please report it!

To get started, type `help ssh` at the kermit prompt but note that not all
commands are implemented yet! You can see which commands are available using
the '?' key, for example by typing `set ssh ?` or `ssh ?`. Full documentation
for the built-in SSH client as delivered in Kermit 95 v2.1.3 is available here: 
https://kermitproject.org/k95manual/sshclien.html - SSH differences between
Kermit 95 and Kermit 95 are discussed later in this document.

## Known Issues

* When connecting to modern linux hosts with the linux terminal type, you'll
  likely need to set your remote charset to utf8. You can do this with: 
  ```
  set term type linux
  set term remote utf8
  ```
  For convenience, you can just uncomment these lines in the default
  `k95custom.ini` file included in the K95 distribution so that they're run 
  every time you start Kermit 95.
* If you find your session disconnecting when left idle, try enabling the
  heartbeat feature with the `set ssh heartbeat-interval` command.
* Connecting through proxy servers is not currently supported

## Differences From Kermit 95 2.x and 1.1.21

The GUI dialogs for the `ssh key` commands have also been adjusted a little. 
When GUI dialogs are enabled, these commands will use a standard Windows file
dialog rather than having you type in a full pathname if a filename was not
specified as part of the command.

### SSH is now modular!

The SSH backend has been moved out into a DLL that is loaded automatically on
startup if present. On x86 platforms, four DLLs are provided:

| DLL          | Description                                                |
|--------------|------------------------------------------------------------|
| k95sshg.dll  | GSSAPI-enabled SSH backend for Windows Vista and newer.    |
| k95ssh.dll   | Standard SSH backend for Windows Vista and newer           |
| k95sshgx.dll | GSS-API enabled SSH backend for Windows XP and Server 2003 |
| k95sshx.dll  | Standard SSH backend for Windows XP and Server 2003        |

On startup, K95 will attempt each one in order and if any one of them loads
then SSH features will be made available. If none of these DLLs are present or
none of them load successfully for one reason or another, then only a single
`SSH LOAD` command is provided allowing you to specify an alternative DLL to
use to provide SSH features.

As a result of this change, starting Kermit 95 without optional network DLLs
(the `-# 2` command line argument) will result in SSH being unavailable until
the `SSH LOAD` command is used to load a backend DLL.

It is also now possible for alternative SSH implementations not based on libssh
to be provided by implementing a relatively simple DLL interface similar to 
Kermit 95s "Network DLL" interface. This may someday allow SSH to return on 
vintage windows, or SSH to be supported on OS/2.

### New Command Options
These commands are unchanged aside from having some new options. Some options
that were supported in Kermit 95 may have been removed.

```
set ssh v2 hostkey-algorithms {ecdsa-sha2-nistp256, ecdsa-sha2-nistp384, ecdsa-sha2-nistp521, rsa-sha2-256, rsa-sha2-512, ssh-ed25519}
set ssh v2 macs {hmac-sha1-etm@openssh.com, hmac-sha2-256, hmac-sha2-256-etm@openssh.com, hmac-sha2-512, hmac-sha2-512-etm@openssh.com, none}
```

### New Commands
The following commands are new to Kermit 95 and so not documented in
the Kermit 95 manual.

```
SSH LOAD filename
  This command is only available when no SSH backend DLL was loaded on 
  startup, either due to there being no compatible DLL available or due to
  the loading of optional network libraries being disabled via command line
  parameter. 
 
  This command takes one or more DLL filenames separated by a semicolon (;)
  which will attempted in order. The first DLL that loads successfully will
  enable all SSH commands and be used for all SSH operations until Kermit is
  restarted.

SSH REMOVE LOCAL-PORT-FORWARD local-port
  Removes the local port forward with the specified local-port from
  the local port forwarding list. This has no effect on any active 
  connection.

SSH REMOVE REMOTE-PORT-FORWARD remote-port",
  Removes the remote port forward with the specified remote-port from
  the remote port forwarding list. This has no effect on any active
  connection.

set ssh v2 key-exchange-methods {CURVE25519-SHA256,
     CURVE25519-SHA256@LIBSSH.ORG, DIFFIE-HELLMAN-GROUP1-SHA1,
     DIFFIE-HELLMAN-GROUP14-SHA1, DIFFIE-HELLMAN-GROUP14-SHA256,
     DIFFIE-HELLMAN-GROUP16-SHA512, DIFFIE-HELLMAN-GROUP18-SHA512,
     DIFFIE-HELLMAN-GROUP-EXCHANGE-SHA1,
     DIFFIE-HELLMAN-GROUP-EXCHANGE-SHA256, ECDH-SHA2-NISTP256,
     ECDH-SHA2-NISTP384, ECDH-SHA2-NISTP521 }
  Specifies an ordered list of Key Exchange Methods to be used to generate
  per-connection keys. The default list is:

    curve25519-sha256 curve25519-sha256@libssh.org ecdh-sha2-nistp256
    ecdh-sha2-nistp384 ecdh-sha2-nistp521 diffie-hellman-group18-sha512
    diffie-hellman-group16-sha512 diffie-hellman-group-exchange-sha256
    diffie-hellman-group14-sha256 diffie-hellman-group14-sha1
    diffie-hellman-group1-sha1 ext-info-c}
```

### Removed Commands and Options
The following SSH commands and options are obsolete and not supported by 
libssh, so they have now been hidden and will just return an error when 
entered.

```
set ssh version 1
set ssh {kerberos4, kerberos5, krb4, kerb5, k4, k5}
set ssh v1
set ssh v2 authentication {external-keyex, hostbased, srp-gex-sha1}
set ssh v2 ciphers {arcfour, blowfish-cbc, cast128-cbc, rijndael128-cbc, rijndael192-cbc, rijndael256-cbc}
set ssh v2 macs {hmac-md5, hmac-md5-96, hmac-ripemd160, hmac-sha1-96}
ssh key v1
ssh key display /format:ietf
ssh v2 rekey
```

### Not Yet Implemented
The following commands have not been implemented _yet_. The intention is to
eventually implement as many of these as possible but this comes down to what
libssh will or will not support. At this time all of these commands are hidden
and just return an error when entered.

```
SSH AGENT    
    ADD identity-file
    DELETE identity-file
    LIST
        /FINGERPRINT
SET SSH
    AGENT-FORWARDING {ON,OFF}
    CHECK-HOST-IP {ON,OFF}
    DYNAMIC-FORWARDING {ON,OFF}
    GATEWAY-PORTS {ON,OFF}
    PRIVILEGED-PORT {ON,OFF}
    QUIET {ON,OFF}
    V2 AUTO-REKEY {ON,OFF}
    XAUTH-LOCATION filename
```

## Supported Authentication Methods

At this time password, public key, gssapi-mic and keyboard interactive 
authentication are implemented and work.

There is not yet support for using ssh agents but as this is supported by the 
ssh backend used by Kermit 95 support for these may appear in a future release.

### Using gssapi-mic (Kerberos) authentication

To use GSSAPI authentication, you must [Download and Install Kerberos for Windows from MIT](http://web.mit.edu/kerberos/dist/index.html).
When this is installed, Kermit 95 will automatically load the
GSSAPI-enabled backend (k95sshg.dll or k95sshgx.dll) on startup.

GSSAPI authentication is not currently supported on non-x86 builds of K95 as
current versions of Kerberos for Windows are only available for x86.

### Using SSH on Windows XP
Libssh 0.10.6 includes a fix for the security vulnerability 
[CVE-2023-6004](https://nvd.nist.gov/vuln/detail/CVE-2023-6004), but the fix breaks compatibility with Windows XP resulting
in the error "The procedure entry point if_nametoindex could not be located in 
the dynamic link library IPHLPAPI.DLL."

As a result, Kermit 95 now includes separate SSH backends for
Windows XP: `k95sshx.dll` and `k95sshgx.dll`. These SSH backends revert the 
CVE-2023-6004 fix allowing them to operate correctly on Windows XP. One of these
will be used automatically on Windows XP and Server 2003 if they are present. 

CVE-2023-6004 has a low severity rating and impacts the ProxyCommand and 
ProxyJump features which are not currently used by Kermit 95 or supported on 
Windows yet. So undoing the security fix is unlikely to cause any problems in
practice, but still not a great idea. Where security is a concern you should
consider upgrading to a supported version of Windows which is able to run the
normal version of libssh. Alternatively, you can delete `k95sshx.dll` and 
`k95sshgx.dll` to disable SSH support on Windows XP.

[^1]: https://libssh.org