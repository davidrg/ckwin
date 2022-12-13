# SSH Support in C-Kermit for Windows

C-Kermit for Windows now includes built-in SSH support based on libssh[^1]. This
is still a work-in progress so not all SSH commands supported by Kermit 95 are
available in CKW at this time and bugs are likely. If you find a bug, please
report it!

To get started, type `help ssh` at the kermit prompt but note that not all
commands are implemented yet! Full documentation for the built-in SSH client
as delivered in Kermit 95 v2.1.3 is available here: 
https://kermitproject.org/k95manual/sshclien.html - SSH differences between
Kermit 95 and C-Kermit for Windows are discussed later in this document.

For convenience C-Kermit currently looks for the known_hosts file as well as 
your public and private keys in `%USERPROFILE%\.ssh`. This directory is also
used by the version of OpenSSH bundled with Windows 10+ so if you were
previously using that your keys and known hosts should be picked up
automatically by C-Kermit.

## Known Issues

* When connecting to modern linux hosts with the linux terminal type, you'll
  likely need to set your remote charset to utf8. You can do this with: 
  ```
  set term type linux
  set term remote utf8
  ```
  For convenience, you can just uncomment these lines in the default
  `k95custom.ini` file included in the CKW distribution so that they're run 
  every time you start C-Kermit.
* If you find your session disconnecting when left idle, try enabling the
  heartbeat feature with the `set ssh heartbeat-interval` command.
* Connecting through proxy servers is not currently supported

## Differences From Kermit 95

The `set ssh v2 authentication` command only works to turn methods on or off at 
this time, it does not affect the order in which authentication methods are 
attempted. This may be corrected in a future release.

The GUI dialogs for the `ssh key` commands have also been adjusted a little. 
When GUI dialogs are enabled, these commands will use a standard Windows file
dialog rather than having you type in a full pathname if a filename was not
specified as part of the command.

### New Command Options
These commands are unchanged aside from having some new options. Some options
that were supported in Kermit 95 may have been removed.

```
set ssh v2 hostkey-algorithms {ecdsa-sha2-nistp256, ecdsa-sha2-nistp384, ecdsa-sha2-nistp521, rsa-sha2-256, rsa-sha2-512, ssh-ed25519}
set ssh v2 macs {hmac-sha1-etm@openssh.com, hmac-sha2-256, hmac-sha2-256-etm@openssh.com, hmac-sha2-512, hmac-sha2-512-etm@openssh.com, none}
```

### New Commands
The following commands are new to C-Kermit for Windows and so not documented in
the Kermit 95 manual.

```
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
The following SSH commands and options have been removed because they are
obsolete and not supported by libssh:

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

### Not Yet Implemented (or removed)
The following commands have not been implemented _yet_. The intention is to
eventually implement as many of these as possible but this comes down to what
libssh will or will not support. Some will likely be removed entirely in future
releases. At this time all of these commands just return an error or do nothing.

```
SSH [OPEN] /X11-FORWARDING: {on,off}
SSH ADD
    LOCAL-PORT-FORWARD local-port host port
    REMOTE-PORT-FORWARD remote-port host port
SSH AGENT    
    ADD identity-file
    DELETE identity-file
    LIST
        /FINGERPRINT
SSH CLEAR
    LOCAL-PORT-FORWARD
    REMOTE-PORT-FORWARD
SET SSH
    AGENT-FORWARDING {ON,OFF}
    CHECK-HOST-IP {ON,OFF}
    DYNAMIC-FORWARDING {ON,OFF}
    GATEWAY-PORTS {ON,OFF}
    IDENTITY-FILE filename
    PRIVILEGED-PORT {ON,OFF}
    QUIET {ON,OFF}
    V2 AUTO-REKEY {ON,OFF}
    X11-FORWARDING {ON, OFF}
    XAUTH-LOCATION filename
```

## Supported Authentication Methods

At this time password, public key and keyboard interactive authentication are 
implemented and work.

There is not yet support for using ssh agents or GSSAPI (Kerberos)
authentication but as both of these are supported by the ssh backend used by
C-Kermit support for these may appear in a future release.

[^1]: https://libssh.org