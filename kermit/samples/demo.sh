#!/bin/bash
#
# License: 3-clause BSD *except for the soft font* - that has no particular
#	   license specified.
#
# This is a quick script to show off some of Kermit 95s terminal features.
# For everything to work right, the GUI version of K95 3.0 beta 8 is required,
# and K95 should be set to use the "k95" terminal type (SET TERM TYPE K95).
# It assumes an 80x24 terminal with the status line on (these are the defaults
# for the k95 terminal type), but it should work for larger sizes too. It won't
# attempt to turn the status line off if it wasn't on when the script was run.
#
# This script should also display correctly on a VT520 terminal with the
# following exceptions:
#  * The unicode bullet points will show as some other character
#  * No colour at all
#  * No crossed-out attribute
#
# In most other terminal emulators, the output will appear fairly broken due to
# missing or incorrectly implemented features, especially those from the VT420.
# In particular the following tends to cause trouble:
#  * The VT320 host-programmable status line is rarely supported.
#  * VT420 rectangular area operations are often not supported or buggy
#  * VT420 paging is almost never supported, and when it is there can be bugs
#    like not treating margins as a per-page setting
#  * VT420 macros are almost never supported
#  * 24-bit indexed colors - while Kermit 95 supports both formats, this script
#    just uses the more standards-compliant format which not all terminals
#    support.
#  * Blinking text attribute - often not supported (because its annoying)
#  * Double-height lines - often not supported
#

# Top Banner
VERSION=" 3 . 0  B E T A  8"
BANNER_FMT="  K E R M I T - 9 5 \x1b[3m%s\x1b[0m\n"

# Features - some aren't supported by K95 yet, and none of them are available
#            in releases prior to beta 8.
F_TRUE_COLOR=1     # New in beta 8
F_STRIKETHROUGH=1  # New in beta 8
F_RULED_LINES=0    # -- not supported -- | When turning one of these on, check
F_EXTENDED_UL=0    # -- not supported -- | a gap still appears above the VT420
F_SOFT_FONT=0      # -- not supported -- | line in non-paged terminals
F_VT420_FEATURES=1 # Rectangular area operations mostly present but buggy in
                   # version 2.1 (2002), Text macros and paging new in beta 8

# which line is the VT420 bullet point on?
VT420_LINE=7

# Eventually: "PCTERM, VTNT, win32 and emacs keyboard modes"
KB_MODES="PCTERM and VTNT direct keyboard modes"
#        "|------Max Length-----------------------------|"

SPACE=6

# Clear the screen
printf '\x1b[2J'

# Normal attributes
printf '\x1b[0m'

# DECOM off
printf '\x1b[?6l'

# Clear margins
printf '\x1b[r'

# Go to 1,1
printf '\x1b[1;1f'

# Lines 1 and 2: - the heading
printf '\x1b#3'  # DECDHL ON - Top Half
printf "$BANNER_FMT" "$VERSION"
printf '\x1b#4'  # DECDHL ON - Bottom Half
printf "$BANNER_FMT" "$VERSION"

# IF we support ruled lines, put one under the heading.
if [ "$F_RULED_LINES" = "1" ]; then
	# Start column 2, 78 columns wide (columns 2-79)
	# On line 2, only one line high
	# Only bottom border
	printf '\x1b[1;2;78;2;1,r'
fi

# Line 3 - some space
printf '\n'

# Line 4 - Standard Attributes
printf ' \u25CF \x1b[3mOptional\x1b[0m true attribute support:'
printf ' \x1b[2mDim\x1b[0m'
printf ' \x1b[1mBold\x1b[0m'
printf ' \x1b[3mItalic\x1b[0m'
printf ' \x1b[4mUnderline\x1b[0m'
if [ "$F_STRIKETHROUGH" = "1" ]; then
	printf ' \x1b[9mCrossed-out\x1b[0m'
fi
printf ' \x1b[5mBlink\x1b[0m'
printf '\n'

# Line 5 - Extended Underline Attributes
if [ "$F_EXTENDED_UL" = "1" ]; then
	printf ' \u25CF Extended underline styles:'
	printf ' \x1b[4:2mDouble\x1b[0m'
	printf ' \x1b[4:4mDotted\x1b[0m'
	printf ' \x1b[4:5mDashed\x1b[0m'
	printf ' \x1b[4:3mCurly\x1b[0m'
	printf ' \x1b[3;4:3;58:2:240:143:104mWith colour!\x1b[0m\n'

	SPACE="$(($SPACE-1))"
fi

# Line 5 or 6: 24-bit colour
if [ "$F_TRUE_COLOR" = "1" ]; then
	# K95 supports both the correct standards-compliant format (below), and the
	# old incorrect semicolon-delimited color specifier used by some other
	# terminals. To catch out any other terminals not using the standards-
	# compliant form, we set some simple colors too.
	printf ' * Full \x1b[30m'
	printf '\x1b[38:2:0:63:158:82m2'
	printf '\x1b[38:2:0:175:201:147m4'
	printf '\x1b[38:2:0:242:0:72m-'
	printf '\x1b[38:2:0:160:24:149mb'
	printf '\x1b[38:2:0:29:193:136mi'
	printf '\x1b[38:2:0:150:106:138mt '
	printf '\x1b[38:2:0:192:247:226mc'
	printf '\x1b[38:2:0:93:173:247mo'
	printf '\x1b[38:2:0:27:80:160ml'
	printf '\x1b[38:2:0:108:181:108mo'
	printf '\x1b[38:2:0:211:93:57mu'
	printf '\x1b[38:2:0:27:226:107mr'
	printf '\x1b[92m\x1b[38:2:0:164:229:13m!'
	printf '\x1b[0m\n'

	SPACE="$(($SPACE-1))"
fi

# Line 6 or 7
if [ "$F_RULED_LINES" = "1" ]; then
	LINE=6
	if [ "$F_EXTENDED_UL" = "1" ]; then
		LINE=7
	fi

	printf ' \u25CF DECterm Ruled Lines\n'

	# This should put a box around the "Ruled Lines" text.
	printf '\x1b[15;12;10;%d;1,r' $LINE
	SPACE="$(($SPACE-1))"
fi

if [ "$F_SOFT_FONT" = "1" ]; then

	printf ' \u25CF VT220 '

	# Download the Jetpac font by Paul Flo Williams
	# https://vt100.net/dec/vt320/fonts (no license stated)
	# This is really a VT320 font, not a VT520 font like we *should* be
	# using.
	printf '\x1bP0;1;1;0;0;2;0;0;{P???????www?????/???????JJJ?????;
	????KKK???KKK??/???????????????;??__ooo__ooo__?/??CCNNNCCNNNCC?;
	??__oOOwwOOO???/????DDDNNDDFAA?;??OwgwO?_oWG???/????GKEB@CMIMC?;
	????OwgggWO?__?/??EFNHGGLFAMLH?;?????GGKCC?????/???????????????;
	?????_owGCC????/?????BFNGOO????;?????CCGwo_????/?????OOGNFB????;
	???___?oo?___??/???AAA@FF@AAA??;???????ooo?????/???@@@@FFF@@@@?;
	???????????????/????__oo[[KK???;???????????????/????@@@@@@@@@??;
	???????????????/??????KKKK?????;????????_ow[KC?/???OW[MFB@?????;
	??wwGGGGGGGGww?/??NNGGGGGGNNNN?;???????ww??????/?????NNNN??????;
	??GGGGGGGGGGww?/??NNNNHHHHHHHH?;??GGGGGGGGww???/??HHHHHHHHNNNN?;
	??ww????????ww?/??@@@@@@@@NNNN?;??wwGGGGGGGGGG?/??HHHHHHHHNNNN?;
	??wwGGGGGGGGGG?/??NNNNHHHHHHNN?;??GGGGGGGGGGww?/??????????NNNN?;
	????wwGGGGww???/??NNNNHHHHHHNN?;??wwGGGGGGGGww?/??HHHHHHHHNNNN?;
	??????oooo?????/??????KKKK?????;???????oooo????/???__oo[[KK????;
	??????__oOWGG??/????@@BAECKGG??;????_________??/????AAAAAAAAA??;
	???GGWOo__?????/???GGKCEAB@@???;???OOWGGGGwwo??/??????IIJ@@????;
	??owwKCssS[{wo?/??BFFKGHJIIJJ@?;????wwGGGGww???/??NNNN@@@@@@NN?;
	??wwGGGGGGww???/??NNNNHHHHHHNN?;??wwGGGGGGGGWW?/??NNNNGGGGGGKK?;
	??wwGGGGGGGGww?/??NNNNGGGGGGNN?;??wwGGGGGGGGGG?/??NNNNHHHHHHHH?;
	??wwGGGGGGGGGG?/??NNNN@@@@@@@@?;??wwGGGGGGGGWW?/??NNNNGGGGHHNN?;
	??ww????????ww?/??NNNN@@@@@@NN?;??????ww???????/??????NNNN?????;
	??????????ww???/??KKGGGGGGNNNN?;??ww??????ww???/??NNNN@@@@@@NN?;
	??ww???????????/??NNNNGGGGGGGG?;?wwGGGGGwwGGGww/?NNNN???NN???NN;
	??wwGGGGGGGGww?/??NNNN??????NN?;??wwGGGGGGwwww?/??NNGGGGGGGGNN?;
	??wwGGGGGGGGww?/??NNNN@@@@@@@@?;??wwGGGGGGGGww?/??NNGGGGGGNNNN?;
	??wwGGGGGGww???/??NNNN@@@@@@NN?;??wwGGGGGGGGWW?/??LLHHHHHHNNNN?;
	??GGGGGwwGGGGG?/???????NNN?????;??ww????????ww?/??NNGGGGGGNNNN?;
	??Ww_??????_wW?/???@FNNMGGMF@??;?ww?????ww???ww/?NNNNGGGNNGGGNN;
	??ww????????ww?/??MMNN@@@@@@MM?;??ww????????ww?/??@@@@NNNN@@@@?;
	??WWGGGGGGGGww?/??NNNNHHHHHHLL?;?????wwwwGG????/?????NNNNGG????;
	???GWwwo_??????/??????@BFNMKG??;?????GGwwww????/?????GGNNNN????;
	????_owWGWwo_??/???????????????;???????????????/??GGGGGGGGGGGG?;
	?????CCKGG?????/???????????????;??__________???/??MMIIIIIINNMM?;
	??ww__________?/??NNMMGGGGGGNN?;??____________?/??NNMMGGGGGGLL?;
	??__________ww?/??NNMMGGGGGGNN?;??____________?/??NNMMIIIIIIJJ?;
	?????wwGGGGGG??/???@@NNNN@@????;??____________?/??nnggggggmm~~?;
	??ww__________?/??NNMM??????NN?;??????gg???????/??????NNMM?????;
	??????????gg???/??oo______~~}}?;??ww??????__???/??NNMMAAAABBMM?;
	?????ww????????/?????NNNN??????;??____________?/??NNMM??NN??NN?;
	??____________?/??NNMM??????NN?;??____________?/??NNMMGGGGGGNN?;
	??____________?/??~~wwGGGGGGNN?;??____________?/??NNGGGGGGww~~?;
	??____________?/??NNMM?????????;??____________?/??JJIIIIIIMMMM?;
	????ww______???/????NNMMGGGG???;??__????????__?/??NNGGGGGGMMNN?;
	??__????????__?/???@BEMMKKEB@??;??__????__??__?/??NNMMGGNNGGNN?;
	??__????????__?/??LLMMAAAAAALL?;??__????????__?/??nnmmgggggg~~?;
	??____________?/??MMMMIIIIIIJJ?;???????owGKCC??/????@@@EMGWOO??;
	???????{{{?????/???????^^^?????;???CCKGwo??????/???OOWGME@@@???;
	??_owWWwo__wwW?/??@@@???@@@@???\x1b\\'

	# Switch to ISO-2002 mode
	printf '\x1b%%@'

	# Switch to the soft character set
	printf '\x1b(P'

	# Output some text in the soft character set
	printf 'soft character sets\n'

	# Switch back to US ASCII
	# (this doesn't seem to work in Windows Terminal 1.22.11141.0)
	printf '\x1b(B'

	# And back to UTF-8 mode
	printf '\x1b%%G'

	SPACE="$(($SPACE-1))"
fi

printf ' \u25CF VT320 host-programmable status line (see the bottom of the terminal)\n'

# This section is a bit mean. It uses a bunch of VT420 features (macros, pages,
# copy/erase rectangle, change attributes in rectangle). Any terminal not
# supporting all of these *correctly* will end up making a mess.
if [ "$F_VT420_FEATURES" = "1" ]; then
  # Define a text macro with ID 0. If it works, "text macros" should
  # appear in the list of features!
  printf '\x1bP0;0;0!ztext macros, \x1b\\'

  # We'll store a backup copy of the "KERMIT-95" heading at this location
  BACKUP_AREA_TOP_LINE=22
  BACKUP_AREA_BOT_LINE=24

  # Try out some rectangular area operations. Back up a chunk of the
  # double-height header by copying it elsewhere.
  printf '\x1b[1;1;2;40;1;%s;10;1$v' $BACKUP_AREA_TOP_LINE

  # Save cursor, go to line 1 and output some stuff, restore cursor
  printf '\x1b7\x1b[1Hrectangular area operations are\n'
  printf 'rectangular area operations are\n---->not supported<----\x1b8'

  # Switch to page 2, switch off DECOM, clear margins and go to line 5, column 5
  printf '\x1b[U\x1b[?6l\x1b[r\x1b[6;5H'

  # Output some text, some of which we'll copy to page 1 later
  printf ' , page memory: if your terminal had it, you would not see this line'
  # If the terminal doesn't support paging, the above line will be dumped over
  # the top of the '24-bit colour' line (second bullet point), though other
  # breakage caused by the margins might prevent this from being visible

  # Set a top margin. Margins are per-page, so the only thing that this should
  # affect is the coordinates we have to supply to DECCRA on page 2. If the
  # terminal incorrectly applies the margins to all pages then a few blank
  # lines will appear before the VT420 bullet point, and for some reason a few
  # things from above the margin may disappear
  printf '\x1b[?6h\x1b[4r'

  # TODO: Set a left margin on page 2 when we support them, and update the
  # DECCRA that copies data from there

  # Switch back to page 1, and go to the VT420 line
  printf '\x1b[V\x1b[%sH' $VT420_LINE

  # Output the VT420 features list, leaving gaps that we'll fill with DECCRA
  printf ' \u25CF VT420 \x1b[0*z not supported-> \x1b[5moperations,\x1b[0m            \n'

  # Copy the text we put on page 2 over to page 1 using DECCRA. Coordinates on
  # the source page are affected by the margins set on that page.
  printf '\x1b[3;6;3;18;2;%s;50;1$v' $VT420_LINE

  # And copy the text we put in line 1 too
  printf '\x1b[1;1;1;16;1;%s;23;1$v' $VT420_LINE

  # Then restore the bit of line 1 we overwrote earlier
  printf '\x1b[%s;10;%s;40;1;1;1;1$v' $BACKUP_AREA_TOP_LINE $BACKUP_AREA_BOT_LINE

  # And wipe the temp copy
  printf '\x1b[%s;10;%s;70$z' $BACKUP_AREA_TOP_LINE $BACKUP_AREA_BOT_LINE

  # Stop "operations" from blinking.
  printf '\x1b[%s;40;%s;50;0$r' $VT420_LINE $VT420_LINE

  # The feature list should look something like this if the terminal supports
  # all required features:
#        1         2         3         4         5         6         7         8
#2345678901234567890123456789012345678901234567890123456789012345678901234567890
#* VT420 text macros, rectangular area operations, page memory

  SPACE="$(($SPACE-1))"
fi

printf ' \u25CF Dozens of other emulations:\tADDS25\tADM3A\tADM5\tAIXTERM\tANNARBOR'
printf '\n\tANSI-BBS\tAT386\tBA80\tBETERM\tDG200\tDG210\tDG217\tHEATH19'
printf '\n\tHFT\tHP2621A\tHPTERM\tHZ1500\tIBM3151\tLINUX\tQANSI\tQNX\tSCOANSI'
printf '\n\tSNI-97801\tSUN\tTTY\tTVI910+\tTVI925\tTVI950\tVC404\tVIP7809'
printf '\n\tVT52\tVT100\tVT102\tVT220\tVT320\tVTNT\tWY30\tWY50\tWY60'
printf '\n\tWY160\tWY360\t\t\x1b[3m(completeness varies, see manual for details)\x1b[0m'
printf '\n'

printf ' \u25CF Full keyboard remapping, plus %s\n' "$KB_MODES"
printf ' \u25CF Connect via SSH, Serial, Modem, Named Pipe, Telnet, Rlogin, PTY, RFC2271...\n'
printf ' \u25CF Fully scriptable with Kermit script and REXX\n'
printf ' \u25CF In-band file transfer with complete Kermit protocol implementation '
printf 'including\n   server mode, plus X/Y/Z-MODEM\n'
printf ' \u25CF Available for 32bit and 64bit Microsoft Windows on all CPU '
printf 'architectures,\n   with reduced feature builds for vintage Windows '
printf 'and 32bit IBM OS/2 Systems\n'

# -------------------------------------------------------------------
# NOTE: No more space for more lines of text if all feature variables
# 	(eg, $F_TRUE_COLOR) are enabled up the top
# -------------------------------------------------------------------

# Status line type to host-writable
printf '\x1b[2$~'

# Move to the status line
printf '\x1b[1$}'

# Move to the start of the status line
printf '\x1b[0`'

# Output some demo text to the status line
if [ "$F_TRUE_COLOR" = "1" ]; then
	for ((i = 255 ; i > 0; i -= 37)); do printf '\x1b[48:2:0:0:0:%dm ' $i; done
	printf '\x1b[0mHost-programmable status line!'
	for ((i = 0 ; i <= 255; i += 6)); do printf '\x1b[48:2:0:0:0:%dm ' $i; done
else
	printf ' >>> Host-programmable status line! <<<'
fi

printf '\x1b[0m'

# Move back to the terminal screen
printf '\x1b[0$}'

# Put a blank line between the feature list and the "press any key" prompt if
# there is space available to do so.
if [ $SPACE -gt 0 ]; then
	printf '\n'
fi

# Make the prompt blink, and center it a little, so its easier to spot at the
# bottom of the feature list. Important if there isn't any visual separation
# between it and the list. And turn the cursor off.
printf '\x1b[5m\t\t\t'

# Cursor off
printf '\x1b[?25l'

read -n 1 -s -r -p "Strike any key to continue..."
# Blinking off
printf '\x1b[0m\n'

# Cursor back on
printf '\x1b[?25h'

# Status line back to indicator, and ensure attributes are back off.
printf '\x1b[1$~\x1b[0m\x1b[25h'

if [ "$F_SOFT_FONT" = "1" ]; then
	# Erase font buffers
	printf '\x1bP0;0;2{ @\x1b\\'
fi

# DECOM off
printf '\x1b[?6l'

# Clear margins
printf '\x1b[r'

# Go to the bottom line, as clearing the margins will have put the cursor at 1,1
# which is a double-height line.
printf '\x1b[24H'

# Done!