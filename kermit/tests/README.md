Kermit 95 Tests
---------------

## vttest.ksc - VTTEST automation
The `vtest.ksc` script automates as much of VTTEST as possible, stepping through
all of its menus running every test that doesn't:

 * require printer output
 * require keyboard input
 * require mouse input
 * affect only the window (title, position, state, etc)

For each test that is run, it records an image of the terminal buffer at the
completion of the test plus a checksum of the terminal buffer, resulting in an
HTML file containing nearly 8,000 image files. It takes a bit over half an hour
to run the full test.

### Patching vttest
At the time of writing (2026-04-17), vttest uses the 
[DECALN](https://davidrg.github.io/ckwin/dev/ctlseqs.html#decaln) escape 
sequence during some of the VT420 Rectangular Area Operations tests to fill the 
screen with the letter 'E', in some cases more than once.

This is a problem on the VT520 and in Kermit 95 as DECALN clears margins and
resets [DECLRMM](https://davidrg.github.io/ckwin/dev/ctlseqs.html#decvssm).
vttest does not expect this, and so does not restore DECLRMM and the margins
after using DECALN causing interactions with margins not to be tested correctly.

It should be noted that vttest likely works a little better on a VT420 as that
terminal _does not_ reset DECLRMM; this is likely a bug given:
* STD 070 resets top and bottom margins
* The chapter in STD 070 predate the DECLRMM feature
* The newer VT520 _does_ reset DECLRMM

Additionally, the DECALN problem can be avoided if the rectangular area
operations tests are done with Color enabled as in this case vttest does not
use DECALN.

A patch is provided, [vttest-decaln.patch](vttest-decaln.patch), to enable the
monochrome Rectangular Area Operations tests to work correctly.