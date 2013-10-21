:userdoc.
:title.C-Kermit's Modem Support
:docprof toc=12345.
:h1 id=preface.Preface
  ---   PREPUBLICATION OUTLINE/DRAFT   ---
:p.
  Copyright (C) 1995, Frank da Cruz and Christine M. Gianone,
.br
  All rights reserved.
:p.
Most recent update&colon. February 8, 1996
:p.
:hp4.
THIS IS AN UPDATE TO CHAPTER 3, "GETTING CONNECTED", OF "USING C-KERMIT".
:ehp4.
:p.
:h1.Modem Types
When you give Kermit a DIAL or ANSWER command, it operates your modem using
built-in knowledge of the kind of modem you have, or an external driver for
the modem, or else information that you have supplied for a "user-defined"
modem type.  So it's important that you give Kermit the best possible
information about the modem.  Usually, all you have to do is tell Kermit what
type of modem you have, and Kermit does the rest automatically.  The
command is&colon.
:p.
:hp4.
SET MODEM TYPE <name>
.br
:ehp4.
:lm margin=3.
Selects a modem type (or NONE for direct serial connections) from Kermit's
built-in database of modems.  The default modem type is NONE.
:lm margin=1.
:p.
Kermit's built-in knowledge applies to many types of modems.  Table I shows
the list as of this writing.  "SET MODEM TYPE ?" might display additional
names if this documenent is out of date.
:p.
Table I - Modem Types
:table cols='20 20 40'.
:row.
:c.Name               
:c.Characteristics      
:c.Description
:row.
:c.att-dataport       
:c.AT EC DC LF SB       
:c.AT&amp.T Dataport
:row.
:c.att-dtdm                                
:c.
:c.AT&amp.T Digital Terminal Data Module
:row.
:c.att-isn                                 
:c.
:c.AT&amp.T ISN Network
:row.
:c.att-switched-net   
:c.AT                   
:c.AT&amp.T Switched-Network Modems
:row.
:c.att-7300                                
:c.
:c.AT&amp.T 7300 (UNIX PC) Internal Modem
:row.
:c.boca               
:c.AT EC DC LF SB       
:c.Boca modems
:row.
:c.cermetek                                
:c.
:c.Cermetek Info-Mate 212A 1200 bps
:row.
:c.concord                                 
:c.
:c.Concord Condor CDS 220 2400 bps
:row.
:c.df03-ac                                 
:c.
:c.Digital Equipment Corp DF03-AC
:row.
:c.df100-series                            
:c.
:c.Digital Equipment Corp DF100 series
:row.
:c.df200-series                            
:c.
:c.Digital Equipment Corp DF200 series
:row.
:c.digitel-dt22       
:c.ITU                  
:c.Digitel DT-22 ITU-T (CCITT) V.25bis
:row.
:c.gdc-212a/ed                             
:c.
:c.General Data Comm 212A/ED
:row.
:c.hayes-1200         
:c.AT                   
:c.Hayes Smartmodem 1200 &amp. compatibles
:row.
:c.hayes-2400         
:c.AT                   
:c.Hayes Smartmodem 2400 &amp. compatibles
:row.
:c.hayes-high-speed   
:c.AT EC DC LF SB       
:c.Hayes Accura, Optima, or Ultra
:row.
:c.intel              
:c.AT EC DC LF SB       
:c.Intel High-Speed FaxModem
:row.
:c.itu-t-v25bis       
:c.ITU                  
:c.ITU-T (CCITT) V.25bis
:row.
:c.maxtech            
:c.AT EC DC LF SB       
:c.MaxTech XM288EA or GVC FAXModem
:row.
:c.microcom-at-mode   
:c.AT EC DC LF SB       
:c.Microcom in Hayes mode (Deskporte)
:row.
:c.microcom-sx-mode      
:c.EC DC LF SB KS    
:c.Microcom in native SX mode
:row.
:c.motorola-fastalk   
:c.AT EC DC LF SB       
:c.Motorola FasTalk II
:row.
:c.multitech          
:c.AT EC DC LF SB       
:c.Multitech MT1432 Series MultiModem II
:row.
:c.mwave              
:c.AT EC DC LF SB       
:c.IBM Mwave PCMCIA
:row.
:c.none                                    
:c.
:c.Direct serial connection
:row.
:c.old-telebit        
:c.AT EC DC LF SB KS TB 
:c.Telebit T1000, T1500, T2500, ...
:row.
:c.penril                                  
:c.
:c.Penril
:row.
:c.ppi                
:c.AT EC DC LF SB       
:c.Practical Peripherals modems
:row.
:c.racalvadic                     
:c.SB       
:c.Racal-Vadic VA4492E in native mode
:row.
:c.rockwell-v32       
:c.AT EC DC LF SB       
:c.Rockwell V.32 modems
:row.
:c.rockwell-v32bis    
:c.AT EC DC LF SB       
:c.Rockwell V.32bis modems
:row.
:c.rockwell-v34       
:c.AT EC DC LF SB       
:c.Rockwell V.34 modems
:row.
:c.rolm-dcm                                
:c.
:c.Rolm/Siemens/IBM CBX data phone
:row.
:c.supra              
:c.AT EC DC LF SB       
:c.Supra modems
:row.
:c.telebit            
:c.AT EC DC LF SB KS TB 
:c.Telebit T1600, T3000, QBlazer, ...
:row.
:c.unknown                                 
:c.
:c.Unknown modem type
:row.
:c.user-defined                            
:c.
:c.User-defined modem type
:row.
:c.usrobotics         
:c.AT EC DC LF SB       
:c.US Robotics Sportster or Courier
:row.
:c.ventel                                  
:c.
:c.Ven-Tel
:row.
:c.zoltrix            
:c.AT EC DC LF SB       
:c.Zoltrix modems
:row.
:c.zoom               
:c.AT EC DC LF SB       
:c.Zoom modems
:row.
:c.zyxel              
:c.AT EC DC LF SB       
:c.ZyXEL modems
:etable.
:p.
For example, "set modem type supra".  If your modem is not in this list,
perhaps it is compatible with one of the built-in types.  Later on, we'll see
how you can add your own modem type to Kermit, in case you have a modem that
is totally different from all the ones that Kermit knows about.
:p.
As you can see by comparing this table with Table 3-2 on page 53 of "Using
C-Kermit", lots of new modem types have been added since the manual was
published.
:p.
The other big difference is that when you give a SET MODEM TYPE command,
Kermit now fetches all the necessary modem-specific information from its
built-in database and also sets many of its own communication parameters
accordingly.  Thus, in most cases, everything is set up for you automatically
when your choose your modem type, and Kermit works much more smoothly with
modern error-correcting, data-compressing modems.
:p.
You can see how this works by giving a SET MODEM TYPE command and then
viewing the information with the SHOW MODEM command.  Example&colon.
:xmp.
:font facename=Courier size=10x8.
[/usr/olga] C-Kermit>:hp4.set modem type ppi:ehp4.:hp9. ; Tell Kermit the modem type:ehp9.
[/usr/olga] C-Kermit>:hp4.show modem:ehp4.:hp9.         ; View the modem-specific parameters:ehp9.
:p.
 Modem type&colon. ppi
 Practical Peripherals V.22bis or higher with V.42 and V.42bis
:p.
 Modem carrier&colon.          auto
 Modem capabilities&colon.     AT SB EC DC HWFC SWFC
 Modem maximum-speed&colon.    115200 bps
 Modem error-correction&colon. on
 Modem compression&colon.      on
 Modem speed-matching&colon.   off (interface speed is locked)
 Modem flow-control&colon.     auto
 Modem kermit-spoof&colon.     off
 Modem escape-character&colon. 43 (= "+")
:p.
MODEM COMMANDs (* = set automatically by SET MODEM)&colon.
:p.
 * Init-string&colon.          ATQ0X4N1S37=0S82=128\{13}
 * Dial-command&colon.         ATD%s\{13}
 * Compression on&colon.       ATS46=2\{13}
 * Compression off&colon.      ATS46=0\{13}
 * Error-correction on&colon.  AT&amp.Q5S36=7S48=7\{13}
 * Error-correction off&colon. AT&amp.Q0S36=0S48=128\{13}
 * Autoanswer on&colon.        ATS0=1\{13}
 * Autoanswer off&colon.       ATS0=0\{13}
 * Hangup-command&colon.       ATQ0H0\{13}
 * Hardware-flow&colon.        AT&amp.K3\{13}
 * Software-flow&colon.        AT&amp.K4\{13}
 * No-flow-control&colon.      AT&amp.K0\{13}
 * Pulse&colon.                ATP\{13}
 * Tone&colon.                 ATT\{13}
:font facename=default size=0x0.
:exmp.
:p.
Each of these items is explained below, and each of them can be changed
by you if necessary with the appropriate commands.
:p.
The SHOW MODEM command (and the modem table above) include a compact listing
of the modem's capabilities.  These are listed in Table II.
:p.
Table II - Modem capabilities
:table cols='5 50'.
:row.
:c.AT    
:c.Hayes AT command set and responses
:row.
:c.ITU   
:c.ITU-T (CCITT) V.25bis command set and responses
:row.
:c.SB    
:c.Speed buffering (interface speed can be locked)
:row.
:c.EC    
:c.Error correction (MNP or V.42/LAPM)
:row.
:c.DC    
:c.Data compression (MNP or V.42bis)
:row.
:c.HWFC  
:c.Hardware flow control (RTS/CTS) (listed in Table I as LF)
:row.
:c.SWFC  
:c.Local software flow control (Xon/Xoff) (listed in Table I as LF)
:row.
:c.KS    
:c.Kermit spoofing (modem itself runs Kermit protocol)
:row.
:c.TB    
:c.Made by Telebit (used internally for simplification)
:etable.
:hp8.
Note&colon. LF stands for Local Flow control; that is, modem can engage in
flow control with the computer it is directly connected to.
:ehp8.

:h1.Important Settings
Look again at this portion of the SHOW MODEM listing (line numbers added)&colon.
:ol compact.
:li.Modem error-correction&colon. on
:li.Modem compression&colon.      on
:li.Modem speed-matching&colon.   off (interface speed is locked)
:li.Modem flow-control&colon.     auto
:eol.
:p.
When you give a SET MODEM TYPE command&colon.
:ol.
:li.If Kermit knows that the modem is capable of error-correction,
Kermit will tell the modem to use it.

:li.If Kermit knows that the modem is capable of data-compression,
Kermit will tell the modem to use it.

:li.If Kermit knows that the modem is capable of speed-buffering,
Kermit will ASSUME the modem has been configured to use it, and
will, therefore, not change its own interface speed in response to
CONNECT speed reports from the modem.

:li.If the modem is capable of local flow-control, Kermit configures it
to use the same kind of flow as Kermit itself is set to use.
:eol.
:p.
Kermit does NOT, however, set any of the following automatically&colon.
:ol compact.
:li.Communication device
:li.Speed of the communication device
:li.Parity or terminal bytesize
:li.Duplex
:eol.
:p.
So you should take care to ensure they are set appropriately.  Kermit does
its best to catch mistakes or possible mismatches, however.  For example,
you can't DIAL or CONNECT until after you have chosen a communication device
with SET PORT or SET LINE.  You also can't DIAL without first specifying a
modem type.  Furthermore, if you try to DIAL on a modem whose maximum speed
(according to Kermit's database) is less that the current speed of the
device, Kermit prints a warning message.

:h1.The Carrier Signal
The modem is connected to the computer (perhaps conceptually) with a bunch of
wires, as explained in Appendix II, "A Condensed Condensed Guide to Serial
Communications", "Using C-Kermit".  One of these wires conveys the "carrier"
signal from the modem to the computer.  Modems are supposed to turn this
signal on only when they have a connection to another modem, and leave it off
at other times.
:p.
Some computers are set up to refuse to communicate through a serial port
unless there is a carrier signal coming in.  This is the paradox&colon. You have to
be able to communicate with the serial port in order to give the dialing
command to the modem, but if you haven't dialed yet, there is no carrier
signal.
:p.
Kermit handles this situation as follows.  By default (unless you tell Kermit
otherwise), it commands the computer NOT to require carrier during dialing,
but to require it at (most) other times, such as during terminal emulation.
This allows Kermit (on most computers, but not all) to detect when the
connection was hung up by the other computer, and to pop back to command mode
automatically.
:p.
But if you try to communicate through a port that is not receiving the carrier
signal in any way other than by giving a DIAL command, then, depending on the
underlying computer and operating system, you are very likely to get an error
message.  This can happen, for example, if you attempt to CONNECT to the modem
and type commands at the modem's command processor.
:p.
The command that controls this sort of thing is&colon.
:p.
:hp4.
SET MODEM CARRIER { AUTO, ON, OFF }
:ehp4.
.br
:lm margin=3.
The default setting, AUTO, means not to require carrier during DIAL or
ANSWER commands, but to require it during terminal emulation.  OFF means
not to require carrier at all, so (for example) you can CONNECT to the
modem and type AT commands.  ON means to require it at all times.
:lm margin=1.
:p.
:h1.Changing Things
The remaining SET MODEM commands let you adapt Kermit to unusual circumstances
or to modems that it does not know about.  Give them AFTER the SET MODEM TYPE
command, to override the values that are picked up from the database.
:p.
:hp4.
SET MODEM SPEED-MATCHING { ON, OFF }  
:ehp4.
.br
:lm margin=3.
Kermit assumes that if your modem is capable of speed buffering, then it it
is configured to do so, and therefore keeps its interface speed constant, no
matter what connection speed is negotiated.  On the other hand, Kermit
assumes that if your modem (according to Kermit's database) is NOT capable
of speed buffering, then its interface speed will change according to the
connection speed, and so therefore Kermit will also have to change its own
interface speed to match.  When these assumptions prove incorrect, Kermit's
speed will not match the modem's, and you will see only garbage.  Give this
command to adapt Kermit to how the modem actually behaves.
:lm margin=1.
:p.
:hp4.
SET MODEM ERROR-CORRECTION { ON, OFF }
:ehp4.
.br
:lm margin=3.
When this is ON and the modem has EC capability, Kermit sends the MODEM
COMMAND ERROR-CORRECTION ON string to the modem as part of the dialing
process.  When this is OFF and the modem has EC capability, Kermit sends
the MODEM COMMAND ERROR-CORRECTION OFF string.  Sometimes when an
error-correcting modem dials another modem that can't do error-correction,
the negotiations confuse the other modem so badly that the connection
can't be made.  Use SET MODEM ERROR-CORRECTION OFF to disable your modem's
error correction feature prior to dialing.
:lm margin=1.
:p.
:hp4.
SET MODEM COMPRESSION { ON, OFF }
:ehp4.
.br
:lm margin=3.
Works similarly to MODEM ERROR-CORRECTION { ON, OFF }.  Data compression
is almost always beneficial when it is done by the modem, at least as long
as you have an effective means of local flow control (preferably RTS/CTS)
between your computer and the modem.  If adequate flow control is lacking,
or if the compression negotiations confuse the other modem, use this
command to disable data compression.
:lm margin=1.
:p.
:hp8.
WARNING&colon. Certain modems come in two different models; one with
error-correction and data compression built in, and the other (normally a
cheaper model) without these features.  If you have one of the latter (which
usually include the abbreviation "RPI" or the word "controllerless" in the
technical specifications) but Kermit's database does not know this, then
dialing will fail when Kermit tries to enable these features.  To use these
modems at all, you will have to tell Kermit to&colon.
:xmp.
:font facename=Courier size=10x8.
SET MODEM ERROR-CORRECTION OFF
SET MODEM COMPRESSION OFF
:font facename=default size=0x0.
:exmp.
:p.
and possibly also&colon.
:xmp.
:font facename=Courier size=10x8.
SET MODEM SPEED-MATCHING ON
:font facename=default size=0x0.
:exmp.
:p.
(Also see examples below.)
:ehp8.
:p.  
:hp4.
SET MODEM FLOW-CONTROL { AUTO, NONE, RTS/CTS, XON/XOFF }
:ehp4.
.br
:lm margin=3.
This tells Kermit how to configure the modem's local flow-control feature,
if any.  AUTO, the default, tries to "do the right thing" based on a
combination of the modem's capabilities (from Kermit's internal modem
database) and Kermit's FLOW-CONTROL setting.  The other options let you
override the automatic procedure, for example, when you are running Kermit
on a version of UNIX that does not let you SET FLOW RTS/CTS, but which,
nevertheless, lets you have RTS/CTS flow control by using a "special" device
name, e.g. "set line /dev/cufa", in which case Kermit did not know that
RTS/CTS could be used.  So, for example, if Kermit's FLOW-CONTROL is
NONE, but you tell Kermit to SET MODEM FLOW-CONTROL RTS/CTS, then Kermit
will send the modem its built-in MODEM COMMAND HARDWARE-FLOW string.
:lm margin=1.
:p.
:hp4.
SET MODEM HANGUP-METHOD { MODEM-COMMAND, RS232-SIGNAL }
:ehp4.
.br
:lm margin=3.
Tells Kermit which hangup method to use with the modem&colon. the appropriate
modem command (e.g. ATH0) or by maninpulating RS-232 signals (e.g.  turning
off DTR).  MODEM-COMMAND is the default when the modem database includes a
command for hanging up, otherwise the RS232-SIGNAL method is used.
:lm margin=1.
:p.
:hp4.
SET MODEM ESCAPE-CHARACTER <number>
:ehp4.
.br
:lm margin=3.
For Hayes compatibles, Kermit normally uses plus-plus-plus as the escape
sequence.  It uses this to hang up the modem.  The escape sequence is
supposed to bring us back to the modem's command processor, and then we give
the modem-specific hangup comand, such as ATH0.  However, when the answering
modem on the other end has not been configured correctly to ignore the
escape sequence, the escape sequence is sent to the host, echoes back, and
the remote answering modem pops back to command mode, with no way to put it
back online.  If it is not in your power to reconfigure the answering modem,
then your only recourse is to use this command to change the calling modem's
escape character to something different.  <number> is the ASCII value of the
character to be used.  For Hayes compatibles, we use the guard time, three
copies of the escape character, and guard time again.  For others
(e.g. Microcom or Racal Vadic in native mode) we just send the escape
character.  This command is also handy for disabling the escape character
altogether, e.g. SET MODEM ESC 128, in case you have a TIES modem and you
need to make it transparent to all character sequences.
:lm margin=1.
:p.
:hp4.
SET MODEM KERMIT-SPOOF { ON, OFF }
:ehp4.
.br
:lm margin=3.
This command is used with modems that have a built-in "Kermit spoof",
in which the modem actually executes the Kermit protocol itself.  A matching
modem (e.g. Telebit) is required on the other end.  Normally Kermit can
transfer files faster without the modem's help, by using sliding windows,
long packets, etc, so the modem's Kermit spoof (if any) is disabled by
default.  Use SET MODEM KERMIT-SPOOF ON to enable the modem's Kermit spoof.
:lm margin=1.
:p.
:h1.Adding a New Modem Type

The following commands let you define a new modem type for Kermit.
:p.
:hp4.
SET MODEM TYPE USER-DEFINED [ <name> ]
:ehp4.
.br
:lm margin=3.
The optional <name> identifies one of the built-in modem types.  The
default <name> is UNKNOWN, which has no commands.  All the characteristics
of the built-in modem type are copied to the user-defined type.  For
example, if you are adding a high-speed modem that uses the Hayes command
set, choose a modem of that sort (e.g.  HAYES-HIGH-SPEED, PPI, etc.
:lm margin=1.
:p.
Then you can use the following commands&colon.
:xmp.
:font facename=Courier size=10x8.
SET MODEM NAME
SET MODEM CAPABILITIES
SET MODEM MAXIMUM-SPEED
:font facename=default size=0x0.
:exmp.
:p.
to define its basic characteristics, and then&colon.
:xmp.
:font facename=Courier size=10x8.
SET MODEM COMMAND
:font facename=default size=0x0.
:exmp.
:p.
to change any of its commands that differ from those of the built-in modem
that you chose as a model.  These commands are now described in detail&colon.
:p.
:hp4.
SET MODEM NAME <text>
:ehp4.
.br
:lm margin=3.
Changes the descriptive name of the modem (effects SHOW MODEM only).
:lm margin=1.
:p.
:hp4.
SET MODEM MAXIMUM-SPEED <bits-per-second>
:ehp4.
.br
:lm margin=3.
Tell Kermit what the modem's maximum *interface* speed is, in case the
database is wrong or you are adding a new modem type.  Note&colon. currently,
this item has no real effect, other than to cause warning messages if
Kermit's speed is higher than the modem's maximum speed when you give a
DIAL command.
:lm margin=1.
:p.
:hp4.
SET MODEM DIAL-COMMAND <string>
:ehp4.
.br
:lm margin=3.
This is the command used by the modem to dial a call.  The <string>
must contain "%s" (without the quotes) to show where the phone number
must be placed.  For example, for a Hayes modem, you would use
"set modem dial-command ATD%s\13".
:lm margin=1.
:p.
:hp4.
SET MODEM CAPABILITIES [ <item> [ <item> [ ... ] ] ]
:ehp4.
.br
:lm margin=3.
Tells Kermit the modem's capabilities in case the database is wrong or you
are adding a new modem type.  The <items> may be entered as shown in Table
II, or with their full names ("error-correction", etc), which will be
listed for you if you type "set modem capabilities ?".  AT must be set for
any modem that uses the Hayes AT command set and responses; otherwise
Kermit won't be able to read the responses; similarly for V25-COMMANDS
(ITU) for V.25bis modems.
:lm margin=1.
:p.
If your user-defined modem does not use Hayes word-result, Hayes
digit-result, or V.25bis completion codes, then Kermit decides whether a
call has completed or failed based on whether the carrier signal comes on
within the DIAL TIMEOUT interval.
:p.
Now you must tell Kermit the modem-specific commands to use for each
modem function it must control.  This is done with the following command&colon.
:p.
:hp4.
SET MODEM COMMAND ....
:ehp4.
.br
:lm margin=3.
Tells Kermit the modem-specific command for various configurations and
actions, in case the database is wrong or you are adding a new modem type.
Control characters can be included in modem command strings by using
backslash notation, such as "\13" for carriage return.
:lm margin=1.
:p.
Here are the SET MODEM COMMAND commands&colon.  
:p.
:hp4.
SET MODEM COMMAND AUTOANSWER ON <string>
:ehp4.
.br
:lm margin=3.
The command string that puts the modem into autoanswer mode, i.e.
that makes it wait for an incoming call.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND AUTOANSWER OFF <string>
:ehp4.
.br
:lm margin=3.
The command string that takes the modem out of autoanswer mode, i.e.
that puts in in originate mode for making calls.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND COMPRESSION ON <string>
:ehp4.
.br
:lm margin=3.
The command string that enables the modem to negotiate data compression
with the other modem.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND COMPRESSION OFF <string>
:ehp4.
.br
:lm margin=3.
The command string that turns off the modem's data compression feature
and disables data-compression negotiation with the other modem.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND DIAL-MODE-STRING <string>
.br
SET MODEM COMMAND DIAL-MODE-PROMPT <string>
:ehp4.
.br
:lm margin=3.
Command (if any, normally none) used to put modem into dialing mode,
and the prompt (if any, normally none) issued by the modem when it is
in dialing mode.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND ERROR-CORRECTION ON <string>
:ehp4.
.br
:lm margin=3.
The command string that enables the modem to negotiate error correction
with the other modem.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND ERROR-CORRECTION OFF <string>
:ehp4.
.br
:lm margin=3.
The command string that turns off the modem's error correction feature
and disables error-correction negotiation with the other modem.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND HANGUP-COMMAND <string>
:ehp4.
.br
:lm margin=3.
The command string that tells the modem to hang up the telephone connection.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND HARDWARE-FLOW <string>
:ehp4.
.br
:lm margin=3.
The command string that enables local hardware (RTS/CTS) flow control
in the modem.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND SOFTWARE-FLOW <string>
:ehp4.
.br
:lm margin=3.
The command string that enables local software (Xon/Xoff) flow control
in the modem.  This should be used only if RTS/CTS is not available.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND NO-FLOW-CONTROL <string>
:ehp4.
.br
:lm margin=3.
The command string to disable local flow control in the modem.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND INIT-STRING <string>
:ehp4.
.br
:lm margin=3.
The command string that should be used to initialize the modem.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND PULSE <string>
:ehp4.
.br
:lm margin=3.
The command string that sets the modem's dialing method to pulse.
:lm margin=1.
:p.
:hp4.
SET MODEM COMMAND TONE <string>
:ehp4.
.br
:lm margin=3.
The command string that sets the modem's dialing method to tone.
:lm margin=1.
:p.
For Hayes compatibles, the MODEM INIT-STRING should contain Q0 to ensure the
modem gives result codes for commands.  They can be either numeric or word
result codes; Kermit handles both automatically.  You can also make the modem
echo (E1) or not echo (E0) commands -- Kermit should work either way.  The
init string should contain all commands that are not affected by other SET
MODEM settings, particularly the command to enable BREAK transparency, the
command to enable modulation negotiation, a command to enable a fairly high
level of dial result codes (e.g. X4).
:p.
Example 1&colon. Best Data 14400 bps Fax Modem V.32bis/V.17
:xmp.
:font facename=Courier size=10x8.
SET MODEM USER-DEFINED PPI                ; Use PPI as the template
SHOW MODEM                                ; To see what you've got
:font facename=default size=0x0.
:exmp.
:p.
Now make any needed changes&colon.
:xmp.
:font facename=Courier size=10x8.
SET MODEM NAME Best Data 1442FTX          ; or whatever
;; SET MODEM CAPABILITIES ...             ; (not needed, same as PPI)
SET MODEM MAXIMUM-SPEED 57600             ; or whatever, if necessary
SET MODEM COMMAND INIT ATF1N1W1Y0&amp.C1&amp.D2X4S37=0\13  ; init-string
:p.
SET MODEM COMMAND HARD AT&amp.K3\\G1\13       ; hardware flow on
SET MODEM COMMAND SOFT AT&amp.K4\\G1\13       ; software flow on
SET MODEM COMMAND NO-F AT&amp.K0\\G0\13       ; no flow control
SET MODEM COMMAND ERROR ON  AT&amp.Q6\\N3\13  ; error-correction enabled
SET MODEM COMMAND ERROR OFF AT&amp.Q0\\N1\13  ; error-correction disabled
SET MODEM COMMAND COMPRESS ON  AT%C3\13   ; compression enabled
SET MODEM COMMAND COMPRESS OFF AT%C0\13   ; compression disabled
:font facename=default size=0x0.
:exmp.
:p.
Example 2&colon. SupraExpress 144i Fax/Modem
:p.
This one is an RPI model, and so lacks error correction and data
compression, but the commands and other features are the same as the
built-in Supra type&colon.
:xmp.
:font facename=Courier size=10x8.
SET MODEM-TYPE USER-DEFINED SUPRA
SET MODEM NAME SupraExpress 144i Fax/Modem RPI
SET MODEM CAPABILITIES AT SB HWFC SWFC
:font facename=default size=0x0.
:exmp.
:p.
or if speed-buffering doesn't work (since, in general, it depends on an
error-correcting protocol between the modems)&colon.
:xmp.
:font facename=Courier size=10x8.
SET MODEM CAPABILITIES AT HWFC SWFC
:font facename=default size=0x0.
:exmp.
:p.
By removing the EC and DC capabilities, we prevent Kermit from sending
any EC- or DC-related commands to the modem, which, in this model, would
cause an error.

:h1.APPENDIX I - Modem Command List
:sl compact.
:li.SET MODEM TYPE <name>
:li.SET MODEM CARRIER { AUTO, ON, OFF }
:li.SET MODEM CAPABILITIES { AT, DC, EC, HWFC, ITU, KS, SB, SWFC, TB }
:li.SET MODEM MAXIMUM-SPEED { 1200, 2400, ..., 57600, 115200, ... }
:li.SET MODEM ERROR-CORRECTION { ON, OFF }
:li.SET MODEM COMPRESSION { ON, OFF }
:li.SET MODEM SPEED-MATCHING { ON, OFF }
:li.SET MODEM FLOW-CONTROL { AUTO, NONE, RTS/CTS, XON/XOFF }
:li.SET MODEM KERMIT-SPOOF { ON, OFF }
:li.SET MODEM ESCAPE-CHARACTER <number>
:li.
:li.SET MODEM COMMAND AUTOANSWER ON <string>
:li.SET MODEM COMMAND AUTOANSWER OFF <string>
:li.SET MODEM COMMAND COMPRESSION ON <string>
:li.SET MODEM COMMAND COMPRESSION OFF <string>
:li.SET MODEM COMMAND ERROR-CORRECTION ON <string>
:li.SET MODEM COMMAND ERROR-CORRECTION OFF <string>
:li.SET MODEM COMMAND HANGUP-COMMAND <string>
:li.SET MODEM COMMAND HARDWARE-FLOW <string>
:li.SET MODEM COMMAND SOFTWARE-FLOW <string>
:li.SET MODEM COMMAND NO-FLOW-CONTROL <string>
:li.SET MODEM COMMAND INIT-STRING <string>
:li.SET MODEM COMMAND PULSE <string>
:li.SET MODEM COMMAND TONE <string>
:esl.

:h1.APPENDIX II - Differences Between New and Old Modem Related Commands

This applies to the changes that occurred in C-Kermit 5A(192) and other
programs based upon it.
:ol.
:li.SET MODEM <name> became SET MODEM TYPE <name>.  SET MODEM <name> is still
accepted, but ?-help doesn't work; completion, and editing don't work
very well.

:li.Many additional modem types were added, mostly newer high-speed models.

:li.SET MODEM TYPE <name> fills in numerous dial-, modem-, and communications
parameters from the modem database automatically.  Ditto for the -m <name>
command-line option.

:li.The former SET DIAL command was divided into SET MODEM and SET DIAL.
Items that related to modems, but not to dialing, became SET MODEM
commands.  Items that related to dialing, but not modems, remained SET
DIAL commands.  Most of the old SET DIAL forms, however, are still
accepted.
:eol.
:p.   
Here is the list of old and new modem-related commands.  All of the old
commands are still accepted, and work as documented in "Using C-Kermit", but
they are invisible -- i.e. they are not revealed by "?" or "help set modem"
or "help set dial".
:p.
:table cols='40 40'.
:row.
:c.New commands&colon.                          
:c.Equivalent old commands, if any&colon.
:row.
:c.SET MODEM TYPE                         
:c.SET MODEM
:row.
:c.SET MODEM CARRIER                      
:c.SET CARRIER
:row.
:c.SET MODEM CAPABILITIES
:row.
:c.SET MODEM DIAL-COMMAND                 
:c.SET DIAL DIAL-COMMAND
:row.
:c.SET MODEM MAXIMUM-SPEED
:row.
:c.SET MODEM ERROR-CORRECTION             
:c.SET DIAL MNP
:row.
:c.SET MODEM COMPRESSION                  
:c.SET DIAL MNP
:row.
:c.SET MODEM SPEED-MATCHING               
:c.SET DIAL SPEED-MATCHING
:row.
:c.SET MODEM FLOW-CONTROL
:row.
:c.SET MODEM KERMIT-SPOOF                 
:c.SET DIAL KERMIT-SPOOF
:row.
:c.SET MODEM ESCAPE-CHARACTER 
:row.
:c.SET MODEM COMMAND AUTOANSWER
:row.
:c.SET MODEM COMMAND COMPRESSION
:row.
:c.SET MODEM COMMAND ERROR-CORRECTION
:row.
:c.SET MODEM COMMAND HANGUP-COMMAND
:row.
:c.SET MODEM COMMAND HARDWARE-FLOW
:row.
:c.SET MODEM COMMAND SOFTWARE-FLOW
:row.
:c.SET MODEM COMMAND NO-FLOW-CONTROL
:row.
:c.SET MODEM COMMAND INIT-STRING
:row.
:c.SET MODEM COMMAND PULSE
:row.
:c.SET MODEM COMMAND TONE
:etable.
:p.
There are also many new SET DIAL commands, which are described in the
section on dialing.
:euserdoc.
