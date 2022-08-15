# SSH Support in C-Kermit for Windows

C-Kermit for Windows now includes limited built-in SSH support based on 
libssh[^1]. The code is currently very new and likely still has bugs - if you 
find one please report it!

To get started, type `help ssh` at the kermit prompt but note that not all
commands are implemented yet!

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
  For convenience, you can create a file called `k95custom.ini` in the same
  directory as k95g.exe and place these commands there so that they're run
  automatically every time you start C-Kermit.
* The keyboard interactive authentication method doesn't seem to work so it's 
  disabled by default for now. See the *Supported Authentication Methods*
  section for more details.


## Supported Commands
Only the following commands are implemented at this time. More will come in
the future. Commands not listed below either do nothing or return an error
at this time.

```
SSH [OPEN] host [port]
   host: hostname or IP to connect to
   port: port name or number to connect on.
   /COMMAND:command
	   Command to execute instead of your default shell
   /USER:user
	   Defaults to \v(userid)
   /PASSWORD:pass
   /VERSION:{1,2}
	   Just reports an error if version is 1 (SSH-1 not supported)
   /SUBSYSTEM:name
	   Implemented though doesn't appear to be working at the moment
SET SSH			
   COMPRESSION {ON,OFF}
	   Value stored in ssh_cmp
   STRICT-HOST-KEY-CHECK {ASK, ON, OFF}
   USE-OPENSSH-CONFIG {ON,OFF}
   V2 AUTHENTICATION {EXTERNAL-KEYX, GSSAPI, HOSTBASED, KEYBOARD-INTERACTIVE, PASSWORD, PUBKEY, SRP-GEX-SHA1}
	   The following values are implemented: KEYBOARD-INTERACTIVE, PASSWORD, PUBKEY
	   The following are ignored: EXTERNAL-KEYX, GSSAPI, SRP-GEX-SHA1
	       (GSSAPI is processed but GSSAPI Auth isn't implemented yet)
   V2 GLOBAL-KNOWN-HOSTS-FILE filename
   V2 USER-KNOWN-HOSTS-FILE filename
   VERBOSE level
	   Report Errors - Verbosity Level. Range 0-7. Value stored in ssh_vrb
   SSH VERSION {1, 2, AUTOMATIC}
	   Just reports an error if version is 1 (SSH-1 not supported)
SHOW SSH
    Shows ssh settings
```

## Supported Authentication Methods

At this time password and public key authentication are implemented and work.

Keyboard interactive authentication is also implemented but didn't work when
tested against OpenSSH 8.4p1 Debian-5deb11u1. If you want to try it out anyway,
you can enable *only* keyboard interactive authentication by entering
`set ssh v2 auth keyb` at the kermit prompt before starting your session.

If you want to enable keyboard interactive authentication alongside password
and public key, enter `set ssh v2 auth keyb pass pub` instead.

[^1]: https://libssh.org