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
# This script should also display correctly on a VT520 or VT420 terminal with
# the following exceptions:
#  * No colour at all
#  * No crossed-out or italic attributes
# When run on a VT320 or VT220 (assuming $TERM is set properly), features not
# supported by those terminals will be turned off.
#
# In most other terminal emulators, the output will appear fairly broken due to
# missing or incorrectly implemented features, especially those from the VT420.
# In particular the following tends to cause trouble:
#  * The VT320 host-programmable status line is rarely supported.
#  * VT420 rectangular area operations are often not supported or buggy.
#    Failure to implement DECRARA properly tends to be quite visible.
#  * VT420 paging is almost never supported, and when it is there can be bugs
#    like not treating margins as a per-page setting
#  * VT420 macros are almost never supported
#  * A lot of terminals still enforce L/R margins when outside the top/bottom
#    margins when they shouldn't.
#  * 24-bit indexed colors - while Kermit 95 supports both formats, this script
#    just uses the more standards-compliant format which not all terminals
#    support.
#  * Blinking text attribute - often not supported (because its annoying)
#  * Double-height lines - often not supported
#

# Probe the terminal? All this really does at the moment is is disable some
# stuff if it detects K95 3.0 beta 7 or older, or show a warning to expect
# borkage if some other terminal. It can take a while though if the terminal
# doesn't respond to ENQ, XTVERSION, DA and/or DA2
PROBE_TERM=1

# How long to wait for responses from the terminal. Larger numbers for slower
# connections, but will also introduce significant delays if the terminal simply
# doesn't respond.
TIMEOUT=5

# TODO: Do a few DECRQM, DECRQSS and DA queries to see what the situation is.
#       - Probe for colour support
#       - Probe for supported attributes
#       - Probe for 24-bit colour support
#       - Probe for DECLRMM support
#       - Probe for status line support
#       - Number of lines and columns per page
#       List missing things on the warning screen.
#       Maybe add test mode that instead of producing a K95 feature list, tests
#         out detected features and lists them.
# TODO: Play with some of the VT525 colour modes? Perhaps cycle through some
#       different palettes?
# TODO: Restore the status line to its previous state at the end

IS_K95="true"
IS_OLD_K95="false"
PROBLEMS="false"
PROBLIST="\n"

if [ "$PROBE_TERM" = "1" ]; then

  ####### Probe the terminal
  printf '\x1b[2JProbing Terminal...\r\n'

  # As *most* terminals have ENQ disabled (including K95 by default), stick a DA
  # request on the end so we're not waiting for a timeout. Then throw away the
  # DA response.
  # Read it in two goes as we cant rely on the ENQ response not containing 'c'
  read -s -t $TIMEOUT -d '[' -p $'\x05\x1b[c' ENQ_A
  read -s -t $TIMEOUT -d 'c' ENQ_B
  ENQ=$(echo $ENQ_A$ENQ_B | cat -v | awk -F'\^\[' '{ print $1; }')
  DA=$(echo $ENQ_B | cat -v | awk -F'\?' '{ print $2; }')

  # Get secondary device attributes
  read -s -t 5 -d 'c' -r -p $'\e[>c'
  DA2=$(echo $REPLY | cat -v)
  PRODUCT_ID=$(echo $DA2 | awk -F'>|;' '{print $2}')

  # Get XTVERSION. Most stuff supports this too, but just in case we'll do what
  # we did with ENQ and stick a DA request on the end which we'll just throw
  # away.
  read -s -t $TIMEOUT -d '[' -p $'\e[>0q\x1b[c' ENQ_A    # Grab XTVERSION
  read -s -t $TIMEOUT -d 'c' ENQ_B                       # Grab DA
  # Extract XTVERSION if its there
  XTVERSION=$(echo $ENQ_A$ENQ_B | cat -v | awk -F'\^\[P>\||\^\[' '{ print $2; }')

  # If XTVERSION didn't reply, ENQ_A will just contain ESC, so set it to empty.
  if [[ "$(echo $ENQ_A | cat -v)" == "^[" ]]; then
    XTVERSION=""
  fi

  ####### Determine if Kermit 95 or not
  # Figure out if we're dealing with Kermit 95. Starting with 3.0 beta 8, the
  # most reliable way to do that is by looking at what the XTVERSION sequence
  # returns. The alternative is ENQ which is disabled by default.
  if [[ "$XTVERSION" == "Kermit 95"* ]]; then
    # XTVERSION is new in K95 v3.0 beta 8
    IS_K95="true"
    echo "Detected Kermit 95"
  else
    if [[ "$XTVERSION" != "" ]]; then
      # Got an XTVERSION response, and it said we're not dealing with Kermit 95
      IS_K95="false"
      PROBLEMS="true"
    else
      # Didn't get an XTVERSION response - either old K95, or not K95.
      if [[ "$TERM" == *"xterm"* ]]; then
        # Something pretending to be xterm, so not K95
        IS_K95="false"
        IS_OLD_K95="false"
      elif [[ "$TERM" == "vt420" ]]; then
        # Old K95 doesn't have a vt420 emulation
        IS_K95="false"
        IS_OLD_K95="false"
      elif [[ "$TERM" == "vt520" ]]; then
        # Old K95 doesn't have a vt520 emulation either
        IS_K95="false"
        IS_OLD_K95="false"
      elif [[ "$PRODUCT_ID" != "24" ]]; then
        # All released versions of K95 report their Product ID as 24 (VT320)
        IS_K95="false"
        IS_OLD_K95="false"
      else
        IS_OLD_K95="true"
      fi
    fi
  fi

  # See if we have an ENQ response. By default K95 ignores ENQ, but you can turn
  # it on. If its on we should be able to figure out *which* version of K95.
  if [[ "$ENQ" == "" ]]; then
    if [[ $IS_K95 == "true" ]]; then
      if [[ $IS_OLD_K95 == "true" ]]; then
        # We didn't get an ENQ or XTVERSION response. No way to tell if we're
        # dealing with K95 or not, so we'll just assume we are.
        IS_OLD_K95="maybe"
        PROBLEMS="true"
      fi
    fi
  else
    # Parse the ENQ response. K95 should give something like:
    #     K-95 1000415 K95
    # Where "K-95" indicates we're Kermit 95, 1000415 is the C-Kermit version
    # number, and K95 is the selected terminal type.

    # Firstly, check if we're K95
    if [[ "$ENQ" == "K-95"* ]]; then
      IS_K95="true"

      # Next, see if we're running 3.0 beta8 or newer.
      K95_VERSION_L=$(echo $ENQ | awk '{print $2}')
      K95_TERM=$(echo $ENQ | awk -F'_| ' '{print $3}')
      echo "K95 Base Version: $K95_VERSION_L"
      echo "K95 Terminal Type: $K95_TERM"
      if (( $K95_VERSION_L >= 1000415 )); then
        echo "Kermit 95 3.0.0 beta 8 or newer detected"
        IS_OLD_K95="false"

        # Check the terminal type. This script really needs it to be set to 'K95'
        if [[ "$K95_TERM" != *"K95"* ]]; then
          PROBLEMS="true"
          PROBLEM_TERMTYPE="true"
        fi
      else
        echo "Kermit 95 3.0.0 beta 7 or older detected"
        IS_OLD_K95="true"

        # Check the terminal type. For pre-beta8 it really needs to be set to vt320
        if [[ "$K95_TERM" != *"VT320"* ]]; then
          PROBLEMS="true"
          PROBLEM_TERMTYPE="true"
        fi
      fi
    else
      IS_K95="false"
      IS_OLD_K95="false"
      PROBLEMS="true"
    fi
  fi

  ####### Output some basic info we discovered

  echo "ENQ: '$ENQ'"
  echo "XTVERSION: '$XTVERSION'"
  echo "DA1: '$DA'"
  echo "DA2: '$DA2'"
  echo "PRODUCT: $PRODUCT_ID"
  echo "TERM: '$TERM'"
  echo "Is K95: $IS_K95"
  echo "IS Old K95: $IS_OLD_K95"

  if [ "$IS_K95" == "false" ]; then
    PROBLIST="$PROBLIST    -> Not Kermit 95\n"
    PROBLEMS="true"
  fi

  if [[ $PROBLEMS == "true" ]]; then
    printf '\n\nProblems:%b' "$PROBLIST"
    if [[ $IS_OLD_K95 == "maybe" ]]; then
      IS_OLD_K95="true"
      printf ' -> ENQ is disabled and XTVERSION did not respond. Assuming K95 3.0 beta 7 or\n'
      printf '    earlier. You can enable ENQ by entering the following at the command\n'
      printf '    screen to enable this script to detect the version of K95 you''re running:\n'
      printf '\tSET TERMINAL ANSWERBACK ON\n'
    else
      if [[ $IS_K95 == "false" ]]; then
        echo ""
        echo "This script makes completely unnecessary use of a number of VT320 and VT420"
        echo "features that are either missing or buggy in most other terminal emulators."
        echo "As you don't appear to be running Kermit 95 (make sure answerback is enabled"
        echo "if you are), the output probably will not appear as intended. Rest assured that,"
        echo "with the exception of 24-bit color and the italic & crossed-out attributes, this"
        echo "script *does* display correctly on a real VT420 or VT520."
        echo ""
      fi
    fi
    if [[ $PROBLEM_TERMTYPE == "true" ]]; then
      if [[ $IS_OLD_K95 == "true" ]]; then
        echo "-> This script requires your Terminal Type to be set to VT320 for best results."
        echo "   It is currently set to: $K95_TERM"
      else
        echo "-> This script requires your Terminal Type to be set to K95 for best results."
        echo "   It is currently set to: $K95_TERM"
      fi
    fi
    printf '\n'
    read -n 1 -s -r -p "Strike any key to continue..."
  fi
fi

# Function to get the line the cursor is on
function curline {
    read -s -t $TIMEOUT -d 'R' -r -p $'\e[?6n' RESP

    if [[ "$RESP" == "" ]]; then
      # Guess this terminal doesn't support DECXCPR.
      echo 1
    else
      echo $RESP | cat -v | awk -F'\?|;' '{print $2}'
    fi
}

# Top Banner
VERSION=" 3 . 0  B E T A  8 "
BANNER_FMT="  K E R M I T - 9 5 \x1b[3m%s\x1b[0m\n"

# Features - some aren't supported by K95 yet, and none of them are available
#            in releases prior to beta 8.
F_STATUS_LINE=1    # New in 1.1.8 (November 1996)
F_TRUE_COLOR=1     # New in beta 8
F_STRIKETHROUGH=1  # New in beta 8
F_RULED_LINES=0    # -- not supported -- | When turning one of these on, check
F_EXTENDED_UL=0    # -- not supported -- | a gap still appears above the VT420
F_SOFT_FONT=0      # -- not supported -- | line in non-paged terminals

# VT420 features:-
# K95 Version 2.1 (2002) supported most rectangular area operations, but the
# implementation is buggy. Text macros, paging, and DECLRMM are new in v3.0 b8.
F_VT420_FEATURES=1
F_DECLRMM=1        # L/R Margins
F_PAGING=1         # Lack of Paging really breaks this script, so it can be
                   # turned off separately as its rarely implemented.

# Eventually: "PCTERM and win32 direct keyboard modes"
KB_MODES="PCTERM direct keyboard mode"
#        "|------Max Length-----------------------------|"


if [[ $IS_K95 == "true" ]]; then
  if [[ $IS_OLD_K95 == "true" ]]; then
    # Kermit 95 3.0 beta 7 and earlier did not support these things
    VERSION=$K95_VERSION_L
    F_TRUE_COLOR=0
    F_STRIKETHROUGH=0
    F_RULED_LINES=0
    F_EXTENDED_UL=0
    F_SOFT_FONT=0
    F_DECLRMM=0
    # Rectangular area features should be present but a little buggy
    F_PAGING=0
  fi
elif [[ $TERM == "vt320" ]]; then
    F_VT420_FEATURES=0
elif [[ $TERM == "vt220" ]]; then
    F_VT420_FEATURES=0
    F_STATUS_LINE=0
    F_SOFT_FONT=0  # TODO: Current font is incompatible
fi

if [ "$F_VT420_FEATURES" != "1" ]; then
  F_PAGING=0
  F_DECLRMM=0
fi

# which line is the VT420 bullet point on?
# Start out at 6 (2 for header, 1 blank line, attributes, VT320, then VT420)
# then add one for each extra feature turned on that displays ahead of the VT420
# line.
VT420_LINE=$((6 + F_TRUE_COLOR + F_RULED_LINES + F_EXTENDED_UL + F_SOFT_FONT))

function declrmm_off {
  # DECLRMM off
  printf '\x1b[?69l'

  # Make the header double-height again.
  printf '\x1b[1H\x1b#3\x1b[2H\x1b#4'
}

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
printf ' * \x1b[3mOptional\x1b[0m true attribute support:'
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
	printf ' * Extended underline styles:'
	printf ' \x1b[4:2mDouble\x1b[0m'
	printf ' \x1b[4:4mDotted\x1b[0m'
	printf ' \x1b[4:5mDashed\x1b[0m'
	printf ' \x1b[4:3mCurly\x1b[0m'
	printf ' \x1b[3;4:3;58:2:240:143:104mWith colour!\x1b[0m\n'
fi

# Line 5 or 6: 24-bit colour
if [ "$F_TRUE_COLOR" = "1" ]; then
	# K95 supports both the correct standards-compliant format (below), and the
	# old incorrect semicolon-delimited color specifier used by some other
	# terminals. To catch out any other terminals not using the standards-
	# compliant form, we set some simple colors too.

  if [ "$F_DECLRMM" = "1" ]; then
    # Make this line double-height. Its not long enough to wrap, but it will
    # just look odd. If the terminal really supports DECLRMM, then this will be
    # undone later when its toggled on briefly.
    printf '\x1b#3'
  fi

	printf ' * Full \x1b[30m'
	printf '\x1b[38:2::63:158:82m2'
	printf '\x1b[38:2::175:201:147m4'
	printf '\x1b[38:2::242:0:72m-'
	printf '\x1b[38:2::160:24:149mb'
	printf '\x1b[38:2::29:193:136mi'
	printf '\x1b[38:2::150:106:138mt '
	printf '\x1b[38:2::192:247:226mc'
	printf '\x1b[38:2::93:173:247mo'
	printf '\x1b[38:2::27:80:160ml'
	printf '\x1b[38:2::108:181:108mo'
	printf '\x1b[38:2::211:93:57mu'
	printf '\x1b[38:2::27:226:107mr'
	printf '\x1b[92m\x1b[38:2:0:164:229:13m!'
	printf '\x1b[0m\n'
fi

# Line 6 or 7
if [ "$F_RULED_LINES" = "1" ]; then
  LINE=$(curline)
	printf ' * DECterm Ruled Lines\n'

	# This should put a box around the "Ruled Lines" text.
	printf '\x1b[15;12;10;%d;1,r' $LINE
fi

if [ "$F_SOFT_FONT" = "1" ]; then

	printf ' * VT220 '

  # TODO: Use an alternate font for VT220

	# Download the Jetpac font by Paul Flo Williams
	# https://vt100.net/dec/vt320/fonts (no license stated)
	# This is a VT320 font, converted to a 10x16 VT520 font by j4james
	printf '\x1bP0;1;1;10;0;2;16;0{ @????oo????/????NN????/????@@????;
	??[[??[[??/??????????/??????????;??__??__??/?r~~rr~~r?/??@@??@@??;
	??__oo__??/?Bfc~~c{W?/????@@????;?_O_??oO??/?@Ap[FoGo?/??@@???@??;
	??_oOO_???/?w|FAeXsB?/??@@@@??@?;???O[K????/??????????/??????????;
	????oWK???/???N~_????/?????@B???;???KWo????/????_~N???/???B@?????;
	????__????/??PI~~IP??/??????????;????__????/??CC~~CC??/??????????;
	??????????/?????ooo??/???CEFB@??;??????????/??CCCCCC??/??????????;
	??????????/????ooo???/????@@@???;??????o{K?/???o{NB???/??BB??????;
	?oOOOOOOo?/?~????{{~?/?@@@@@@@@?;????oo????/???{~~????/???@@@????;
	?OOOOOOOo?/?{{{CCCCF?/?@@@@@@@@?;?OOOOOoo??/?CCCCC~~{?/?@@@@@@@@?;
	?o??????o?/?FCCCC{{~?/??????@@@?;?oOOOOOOO?/?FCCCC{{{?/?@@@@@@@@?;
	?oOOOOOOO?/?~{{CCCC{?/?@@@@@@@@?;?OOOOOOOo?/??????{{~?/??????@@@?;
	??ooOOoo??/?{~~CCFF{?/?@@@@@@@@?;?oOOOOOOo?/?FCCCC{{~?/?@@@@@@@@?;
	????___???/????rrr???/????@@@???;????___???/????rrr???/??CEFB@???;
	?????_oO??/??CMZp_???/??????@@??;??????????/??IIIIII??/??????????;
	??Oo_?????/???_pZMC??/??@@??????;??_oOOo_??/????gkFB??/????@@????;
	?_w[kk{w_?/?N~ofNGNF?/???@@@@@??;??ooOOoo??/?{~~CCFF{?/?@@@????@?;
	?oOOOOoo??/?~{{CCFF{?/?@@@@@@@@?;?oOOOOOOo?/?~{{????o?/?@@@@@@@@?;
	?oOOOOOOo?/?~{{????~?/?@@@@@@@@?;?oOOOOOOO?/?~{{CCCCC?/?@@@@@@@@?;
	?oOOOOOOO?/?~{{CCCCC?/?@@@??????;?oOOOOOOo?/?~{{??CC{?/?@@@@@@@@?;
	?o??????o?/?~{{CCCC~?/?@@@????@?;????o?????/????~{????/????@@????;
	??????oo??/?o????~~{?/?@@@@@@@@?;?o????oo??/?~{{CCFF{?/?@@@????@?;
	?o????????/?~{{??????/?@@@@@@@@?;ooOOOoOOoo/~~{??~??~~/@@@??@??@@;
	?oOOOOOOo?/?~{{????~?/?@@@????@?;?oOOOOooo?/?~????BB~?/?@@@@@@@@?;
	?oOOOOOOo?/?~{{CCCCF?/?@@@??????;?oOOOOOOo?/?~????{{~?/?@@@@@@@@?;
	?oOOOOoo??/?~{{CCFF{?/?@@@????@?;?oOOOOOOo?/?vCCCC{{{?/?@@@@@@@@?;
	?OOOooOOO?/????~~????/????@@????;?o??????o?/?~????{{~?/?@@@@@@@@?;
	?o??????o?/?B^{w?w^B?/???@@@@???;oo???o??oo/~~{??~??~~/@@@@@@@@@@;
	?o??????o?/?z{{CCCCz?/?@@@????@?;?o??????o?/?FCC{{CCF?/????@@????;
	?oOOOOOOo?/?{{{CCCCv?/?@@@@@@@@?;??oooOO???/??~~~?????/??@@@@@???;
	??oo_?????/???BN}w_??/??????@@??;???OOooo??/?????~~~??/???@@@@@??;
	???_oo_???/??BB??BB??/??????????;??????????/??????????/?@@@@@@@@?;
	???K[O????/??????????/??????????;??????????/?zJJJJ~~w?/?@@@@@@@@?;
	?o????????/?~zzBBBB~?/?@@@@@@@@?;??????????/?~zzBBBBv?/?@@@@@@@@?;
	????????o?/?~zzBBBB~?/?@@@@@@@@?;??????????/?~zzJJJJN?/?@@@@@@@@?;
	???oOOOO??/??C~{{C???/???@@@????;??????????/?~BBBBzz~?/?LLLLLLLN?;
	?o????????/?~zzBBBB~?/?@@@????@?;????O?????/????~w????/????@@????;
	??????OO??/??????~~w?/?MKKKKNNN?;?o????????/?~wwGGNNw?/?@@@????@?;
	???o??????/???~{{????/???@@@????;??????????/?~zzB~BB~?/?@@@?@??@?;
	??????????/?~zzBBBB~?/?@@@????@?;??????????/?~zzBBBB~?/?@@@@@@@@?;
	??????????/?~BBBBBB~?/?NNN@@@@@?;??????????/?~BBBBBB~?/?@@@@@NNN?;
	??????????/?~zzBBBBB?/?@@@??????;??????????/?NJJJJzzz?/?@@@@@@@@?;
	??oo??????/??~~zBBB??/??@@@@@@??;??????????/?~????ww~?/?@@@@@@@@?;
	??????????/?BKwwowKB?/????@@????;??????????/?~ww?~??~?/?@@@@@@@@?;
	??????????/?vwwGGGGv?/?@@@????@?;??????????/?~ww????~?/?LLLLLLLN?;
	??????????/?zzzJJJJN?/?@@@@@@@@?;????owKC??/??AA~x????/?????@BA??;
	????{{????/????~~????/????BB????;??CKwo????/????x~AA??/??AB@?????;
	??ooo_?oo?/?FF?BFEF@?/??????????\x1b\\'

	# Switch to ISO-2002 mode
	printf '\x1b%%@'

	# Switch to the soft character set
	printf '\x1b( @'

	# Output some text in the soft character set
	printf 'soft character sets\n'

	# Switch back to US ASCII
	printf '\x1b(B'

	# And back to UTF-8 mode
	printf '\x1b%%G'
fi

if [ "$F_STATUS_LINE" = "1" ]; then
  printf ' * VT320 host-programmable status line (see the bottom of the terminal)\n'
fi

# Do a bit of pre-work for the VT420 line - we'll come back to it later
FRA_1=""
FRA_2=""
FRA_3=""
if [ "$F_VT420_FEATURES" = "1" ]; then
  # Reverse video on - we'll undo it later.
  printf '\x1b[7m'

  if [ "$F_DECLRMM" = "1" ]; then
    # Turn on DECLRMM - this will kill the double-height stuff, but that's OK;
    # we'll restore it later
    printf '\x1b[?69;6h'

    # Set L/R margins and go home
    printf '\x1b[63;80s\x1b[%dr\x1b[H' $VT420_LINE
    printf ', L/R margins'

    # DECOM back off, L/R margins cleared
    printf '\x1b[?6l\x1b[s\x1b[r'
  fi
  if [ "$F_PAGING" = "1" ]; then
    # Switch to page 2, switch off DECOM, clear margins and go to line 6, col 5
    printf '\x1b[U\x1b[?6l\x1b[r\x1b[8;5H'

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

    L_COL=6
    R_COL=18
    if [ "$F_DECLRMM" = "1" ]; then
      # Set a left margin too
      printf '\x1b[4s'
      L_COL=3
      R_COL=15
    fi

    # Switch back to page 1
    printf '\x1b[V'
  fi

  # Skip over the VT420 line for now
  printf '\x1b[%sH' $((VT420_LINE + 1))

  FRA_1="  No"
  FRA_2=" DECFRA"
  FRA_3="  :("

  # Mark what comes next as protected
  printf '\x1b[1"q'
fi

printf ' * Dozens of other emulations:\tADDS25\tADM3A\tADM5\tAIXTERM\tANNARBOR'
printf '\n\tANSI-BBS\tAT386\tBA80\tBETERM\tDG200\tDG210\tDG217\tHEATH19'
printf '\n%s\tHFT\tHP2621A\tHPTERM\tHZ1500\tIBM3151\tLINUX\tQANSI\tQNX\tSCOANSI' $FRA_1
printf '\n%s\tSNI-97801\tSUN\tTTY\tTVI910+\tTVI925\tTVI950\tVC404\tVIP7809' $FRA_2
printf '\n%s\tVT52\tVT100\tVT102\tVT220\tVT320\tVTNT\tWY30\tWY50\tWY60' $FRA_3
printf '\n\tWY160\tWY370\t\t\x1b[3m(completeness varies, see manual for details)\x1b[0m'
printf '\n'

# Now go back and do the VT420 line.
# This section is a bit mean. It uses a bunch of VT420 features (macros, pages,
# copy/erase rectangle, change attributes in rectangle). Any terminal not
# supporting all of these *correctly* will end up making a mess.
if [ "$F_VT420_FEATURES" = "1" ]; then
  # Go to the VT420 line.
  printf '\x1b[%sH' $VT420_LINE

  # DECSCA - Turn off protected
  printf '\x1b["q'

  # Put some garbage in the section above with DECFRA
  printf '\x1b[92;%s;1;%s;8$x' $((VT420_LINE+2)) $((VT420_LINE+7))

  # Define a text macro with ID 0. If it works, "text macros" should
  # appear in the list of features!
  printf '\x1bP0;0;0!ztext macros, \x1b\\'

  # Output the VT420 features list, leaving gaps that we'll fill with DECCRA
  printf ' * VT420 \x1b[0*z not supported-> \x1b[5moperations\x1b[0m            \n'

  if [ "$F_PAGING" = "1" ]; then
    # Copy the text we put on page 2 over to page 1 using DECCRA. Coordinates on
    # the source page are affected by the margins set on that page.
    #printf '\x1b[3;%s;3;%s;2;%d;50;1$v' $L_COL $R_COL $VT420_LINE
    printf '\x1b[5;%s;5;%s;2;%d;50;1$v' $L_COL $R_COL $VT420_LINE
    #            T L  B R  P T  L  P
    #              Source   | Dest
  fi

  if [ "$F_DECLRMM" = "1" ]; then
    # DECLRMM back off
    #printf '\x1b[?69l'

    # Make the header double-height again. +7 for the number of lines of other
    # emulations.
    #printf '\x1b[1H\x1b#3\x1b[2H\x1b#4'
    declrmm_off
  fi

  # We'll store a backup copy of the "KERMIT-95" heading at this location
  BACKUP_AREA_TOP_LINE=22
  BACKUP_AREA_BOT_LINE=24

  # Try out some rectangular area operations. Back up a chunk of the
  # double-height header by copying it elsewhere.
  printf '\x1b[1;1;2;40;1;%s;10;1$v' $BACKUP_AREA_TOP_LINE

  # Save cursor, go to line 1 and output some stuff, restore cursor
  printf '\x1b7\x1b[1Hrectangular area operations are\n'
  printf 'rectangular area operations are\n--->not supported properly<---\x1b8'

  # And copy the text we put in line 1 too
  printf '\x1b[1;1;1;16;1;%s;23;1$v' $VT420_LINE

  # Then restore the bit of line 1 we overwrote earlier
  printf '\x1b[%s;10;%s;40;1;1;1;1$v' $BACKUP_AREA_TOP_LINE $BACKUP_AREA_BOT_LINE

  # And wipe the temp copy with DECERA
  printf '\x1b[%s;10;%s;70$z' $BACKUP_AREA_TOP_LINE $BACKUP_AREA_BOT_LINE

  # Stop "operations" from blinking with DECCARA
  printf '\x1b[%s;40;%s;50;0$r' $VT420_LINE $VT420_LINE

  # Reverse video off
  printf '\x1b[m'

  # And get rid of the reverse with DECRARA! Lets use DECSACE too, as we're not
  # dealing with a nice rectangle.
  printf '\x1b[*x'  # DECSACE - stream of characters
  printf '\x1b[%s;51;%s;78;7$t' $VT420_LINE $((VT420_LINE+7))
  printf '\x1b[2*x' # DECSACE - back to rectangles

  # Wipe out the slashes too, with DECSERA
  printf '\x1b[%s;1;%s;80${' $((VT420_LINE+1)) $((VT420_LINE+7))

  # And clear the background, as DECSERA won't do it
  printf '\x1b[%s;1;%s;8;7$t' $((VT420_LINE+2)) $((VT420_LINE+7))

  printf '\x1b[%sH' $((VT420_LINE+7))

  # The feature list should look something like this if the terminal supports
  # all required features:
#        1         2         3         4         5         6         7         8
#2345678901234567890123456789012345678901234567890123456789012345678901234567890
#* VT420 text macros, rectangular area operations, page memory, L/R margins

fi

printf ' * Full keyboard remapping, plus %s\n' "$KB_MODES"
printf ' * Connect via SSH, Serial, Modem, Named Pipe, Telnet, Rlogin, PTY, RFC2271...\n'
printf '\x1b7\x1b[7mThis should not be indented ->\x1b[m   * Fully scriptable with Kermit script and REXX'

# L/R margins shouldn't be obeyed when below the bottom margin, so doing this
# will have no effect...
if [ "$F_DECLRMM" = "1" ]; then
  printf '\x1b[?6l'                # DECOM off
  printf '\x1b[?69h'               # DECLRMM on
  printf '\x1b[1;10r\x1b[20;60s'   # Margins set
  printf '\x1b[%s;1H' $((VT420_LINE+9))
else
  printf '\x1b8'
fi

printf '\x1b[32P'               # Fix indenting of scriptability line

if [ "$F_DECLRMM" = "1" ]; then
  declrmm_off
  printf '\x1b[%s;1H' $((VT420_LINE+9))
fi
printf '\x1b[m'
printf '\x1bE'   # Next line


printf ' * In-band file transfer with complete Kermit protocol implementation '
printf 'including\n   server mode, plus X/Y/Z-MODEM\n'
printf ' * Available for 32bit and 64bit Microsoft Windows on all CPU '
printf 'architectures,\n   with reduced feature builds for vintage Windows '
printf 'and 32bit IBM OS/2 Systems\n'

# -------------------------------------------------------------------
# NOTE: No more space for more lines of text if all feature variables
# 	(eg, $F_TRUE_COLOR) are enabled up the top - would have to use
#   DECCOLM to get more space. Or switch between multiple pages on a
#   timer.
# -------------------------------------------------------------------

if [ "$F_STATUS_LINE" = "1" ]; then
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
fi

# Put a blank line between the feature list and the "press any key" prompt if
# there is space available to do so.
if [ $(curline) -lt 23 ]; then
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

# DECOM and DECLRMM off
printf '\x1b[?6;69l'

# Clear margins
printf '\x1b[r\x1b[s'

# Go to the bottom line, as clearing the margins will have put the cursor at 1,1
# which is a double-height line.
printf '\x1b[24H'

# Done!