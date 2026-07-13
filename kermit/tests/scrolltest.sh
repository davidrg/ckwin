#!/bin/bash
#
# This is a script for exercising smooth-scroll in Kermit 95. It supports
# scrolling the whole screen, or an area between some combination of
# margins in either the forwards (text moving up the screen from the bottom)
# or reverse (text moving down the screen from the top) direction.
#
# It also makes use of the Ruled Lines extension to highlight the scroll
# region, when set. Corners of scroll regions are also marked with 'X'.
#
# It should also work on other terminals, though some features this script can
# exercise are rarely supported even when smooth-scroll itself is.
#
# Ruled Lines
#    Compatible with WRQ Reflection, K95 and maybe the DEC VT382. DECterm does
#    implement this extension too, but this script makes no effort to work
#    around its various bugs.
# Smooth-Scrolling when the left and right margins are set
#    No terminal or terminal emulator from DEC appears to support this. When
#    left/right margins are set the test will run as though smooth scroll was
#    disabled on those terminals.
#
#

# 1 = on, 0 = off
STBM=1     # Set top and bottom margins
LRMM=1	   # Set left and right margins
LINES=1    # Draw lines on the margins?
REVERSE=1  # Reverse Direction?

# The number of lines output before and after the blinking double height line
# is set based on the height of the scroll region, but you can limit it for
# debugging to a smaller number
LEAD_MAX_HEIGHT=80
TRAIL_MAX_HEIGHT=80

# Position of the margins, when set
TOP_MARGIN=5
BOTTOM_MARGIN=20
LEFT_MARGIN=20
RIGHT_MARGIN=59

# Dimensions of the screen. They're not queried so the output of this script
# can be saved to a file.
SCREEN_HEIGHT=24
SCREEN_WIDTH=80


# Or a pre-prepared test: TEST_NUM=4 ./scrolltest.sh
case $TEST_NUM in
	1)  REVERSE=0; STBM=0; LRMM=0; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # Screen FWD
	2)  REVERSE=0; STBM=1; LRMM=0; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # T/B FWD
	3)  REVERSE=0; STBM=0; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # L/R FWD
	4)  REVERSE=0; STBM=1; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # Sware FWD
	5)  REVERSE=0; STBM=1; LRMM=1; TOP_MARGIN=1; BOTTOM_MARGIN=20; LEFT_MARGIN=1;  RIGHT_MARGIN=59;; # Top Left Corner FWD
	6)  REVERSE=0; STBM=1; LRMM=1; TOP_MARGIN=1; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=80;; # Top Right Corner FWD
	7)  REVERSE=0; STBM=1; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=24; LEFT_MARGIN=1;  RIGHT_MARGIN=59;; # Bottom Left Corner FWD
	8)  REVERSE=0; STBM=1; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=24; LEFT_MARGIN=20; RIGHT_MARGIN=80;; # Bottom Right Corner FWD
	9)  REVERSE=1; STBM=0; LRMM=0; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # Screen REV
	10) REVERSE=1; STBM=1; LRMM=0; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # T/B REV
	11) REVERSE=1; STBM=0; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # L/R REV
	12) REVERSE=1; STBM=1; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=59;; # Square RV
	13) REVERSE=1; STBM=1; LRMM=1; TOP_MARGIN=1; BOTTOM_MARGIN=20; LEFT_MARGIN=1;  RIGHT_MARGIN=59;; # Top Left Corner REV
	14) REVERSE=1; STBM=1; LRMM=1; TOP_MARGIN=1; BOTTOM_MARGIN=20; LEFT_MARGIN=20; RIGHT_MARGIN=80;; # Top Right Corner REV
	15) REVERSE=1; STBM=1; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=24; LEFT_MARGIN=1;  RIGHT_MARGIN=59;; # Bottom Left Corner REV
	16) REVERSE=1; STBM=1; LRMM=1; TOP_MARGIN=5; BOTTOM_MARGIN=24; LEFT_MARGIN=20; RIGHT_MARGIN=80;; # Bottom Right Corner REV
esac

printf '\x1b[2J'	# Clear screen
printf '\x1b[,t'	# Clear ruled lines
printf '\x1b[?4h' 	# Smooth Scroll On

# 80 columns by default, narrower if LRMM on.
WIDTH=$SCREEN_WIDTH
H_CENTER=$((SCREEN_WIDTH / 2))
V_CENTER=$((SCREEN_HEIGHT / 2))

if [ "$LRMM" = "1" ]; then
	WIDTH=$((RIGHT_MARGIN-LEFT_MARGIN +1))
	H_CENTER=$((LEFT_MARGIN + WIDTH / 2))
else
	LEFT_MARGIN=1
	RIGHT_MARGIN=$SCREEN_WIDTH
fi

if [ "$STBM" = "1" ]; then
	HEIGHT=$((BOTTOM_MARGIN-TOP_MARGIN +1))
	V_CENTER=$((TOP_MARGIN + HEIGHT / 2))
else
	HEIGHT=$SCREEN_HEIGHT
	TOP_MARGIN=1
	BOTTOM_MARGIN=$SCREEN_HEIGHT
fi

if [ "$LINES" = "1" ]; then
	# Put a box in the middle to show it doesn't jiggle during the smooth
	# scroll in LRMM mode, but scrolls normally when not in LRMM mode.
	printf '\x1b[15;%d;1;%d;1,r' $H_CENTER $V_CENTER
fi

if [ "$STBM" = "1" ]; then
	# Top/Bottom margins
	printf '\x1b[%d;%dr' $TOP_MARGIN $BOTTOM_MARGIN

	PRE_TOP_MARGIN=$((TOP_MARGIN - 1))
	POST_BOTTOM_MARGIN=$((BOTTOM_MARGIN + 1))

	# Label them, blinkingly
	printf '\x1b[5m'
	if (( TOP_MARGIN > 1 )); then
		printf '\x1b[%d;%dHTop Margin' $PRE_TOP_MARGIN $((H_CENTER - 5))
	fi
	printf '\x1b[%d;%dHBottom Margin' $POST_BOTTOM_MARGIN $((H_CENTER - 6))
	printf '\x1b[m'

	# Line from the top to the margin
	if (( TOP_MARGIN > 2 )); then
		printf '\x1b[1;%dHScreen Top' $((H_CENTER-5))
		for (( i = 2; i < TOP_MARGIN - 2; i++ )); do
			printf '\x1b[%d;%dH|' $i $H_CENTER
		done
		if (( TOP_MARGIN > 3 )); then
			printf '\x1b[%d;%dH+' $(( TOP_MARGIN - 2 )) $H_CENTER
		fi
	fi

	# Line from the bottom to the margin
	if (( BOTTOM_MARGIN < SCREEN_HEIGHT - 1)); then
		printf '\x1b[%d;%dH+' $(( BOTTOM_MARGIN + 2 )) $H_CENTER
		for (( i = BOTTOM_MARGIN + 3; i < SCREEN_HEIGHT; i++ )); do
			printf '\x1b[%d;%dH|' $i $H_CENTER
		done
		printf '\x1b[%d;%dHScreen Bottom' $SCREEN_HEIGHT $((H_CENTER-6))
	fi
else
	PRE_TOP_MARGIN=$TOP_MARGIN
	POST_BOTTOM_MARGIN=$BOTTOM_MARGIN
fi

#printf '\x1b[1;3HHEIGHT=%d,WIDTH=%d' $HEIGHT $WIDTH
#printf '\x1b[2;3HT=%d,B=%d' $TOP_MARGIN $BOTTOM_MARGIN
#printf '\x1b[3;3HL=%d,R=%d' $LEFT_MARGIN $RIGHT_MARGIN

if [ "$LRMM" = "1" ]; then
	# Left/Right margins
	printf '\x1b[?69h'	# DECLRMM on
	printf '\x1b[%d;%ds' $LEFT_MARGIN $RIGHT_MARGIN

	PRE_LEFT_MARGIN=$((LEFT_MARGIN-1))
	POST_RIGHT_MARGIN=$((RIGHT_MARGIN+1))

	# Label the margins, blinkingly
	printf '\x1b[5m'
	if (( LEFT_MARGIN >= 14 )); then
		printf '\x1b[%d;%dHLeft Margin ->' $V_CENTER $((LEFT_MARGIN-14))
	elif (( LEFT_MARGIN >= 3 )); then
		printf '\x1b[%d;%dH->' $V_CENTER $((LEFT_MARGIN-2))
	elif (( LEFT_MARGIN > 1 )); then
		printf '\x1b[%d;%dH>' $V_CENTER $PRE_LEFT_MARGIN
	fi
	if (( RIGHT_MARGIN < SCREEN_WIDTH )); then
		printf '\x1b[%d;%dH<- Right Margin' $V_CENTER $POST_RIGHT_MARGIN
	fi
	printf '\x1b[m'

	# Mark the corners of the screen
	if (( LEFT_MARGIN > 1 && TOP_MARGIN > 1 )); then
		printf '\x1b[HX'
	fi
	if (( RIGHT_MAGIN < SCREEN_WIDTH && TOP_MARGIN > 1 )); then
		printf '\x1b[1;%dHX' $SCREEN_WIDTH
	fi
	if (( LEFT_MARGIN > 1 && BOTTOM_MARGIN < SCREEN_HEIGHT )); then
		printf '\x1b[%dHX' $SCREEN_HEIGHT
	fi
	if (( RIGHT_MARGIN < SCREEN_WIDTH && BOTTOM_MARGIN < SCREEN_HEIGHT )); then
		printf '\x1b[%d;%dHX' $SCREEN_HEIGHT $SCREEN_WIDTH
	fi
else
	LEFT_MARGIN=1
	RIGHT_MARGIN=80
	PRE_LEFT_MARGIN=$LEFT_MARGIN
	POST_RIGHT_MARGIN=$RIGHT_MARGIN
fi

if [ "$STBM$LRMM" = "11" ]; then
	if [ "$LINES" = "1" ]; then
		if (( LEFT_MARGIN > 1 || RIGHT_MARGIN < 80 )); then
			printf '\x1b[15;%d;%d;%d;%d,r' $LEFT_MARGIN $WIDTH $TOP_MARGIN $HEIGHT
		fi

		# Draw a box around the scroll region. This has to be done
		# on the outside border of the scroll region as the lines
		# within the scroll region are sliding upwards
		if (( TOP_MARGIN > 1 )); then
			printf '\x1b[1;%d;%d;%d;1,r' $LEFT_MARGIN $WIDTH $PRE_TOP_MARGIN     # Top
		fi
		printf '\x1b[4;%d;%d;%d;1,r' $LEFT_MARGIN $WIDTH $POST_BOTTOM_MARGIN # Bottom
		if (( LEFT_MARGIN > 1 )); then
			printf '\x1b[2;%d;1;%d;%d,r' $PRE_LEFT_MARGIN $TOP_MARGIN $HEIGHT    # Left
		fi
		printf '\x1b[8;%d;1;%d;%d,r' $POST_RIGHT_MARGIN $TOP_MARGIN $HEIGHT  # Right
	fi

	# Mark the corners too
	if (( TOP_MARGIN > 1 )); then
		if (( LEFT_MARGIN > 1 )); then
			printf '\x1b[%d;%dHXX' $PRE_TOP_MARGIN $PRE_LEFT_MARGIN
		fi
		if (( RIGHT_MARGIN < SCREEN_WIDTH )); then
			printf '\x1b[%d;%dHXX' $PRE_TOP_MARGIN $RIGHT_MARGIN
		fi
	fi
	if (( LEFT_MARGIN > 1 )); then
		printf '\x1b[%d;%dHX' $TOP_MARGIN $PRE_LEFT_MARGIN
		printf '\x1b[%d;%dHX' $BOTTOM_MARGIN $PRE_LEFT_MARGIN
	fi
	if (( RIGHT_MARGIN < SCREEN_WIDTH - 1 )); then
		printf '\x1b[%d;%dHX' $TOP_MARGIN $POST_RIGHT_MARGIN
		printf '\x1b[%d;%dHX' $BOTTOM_MARGIN $POST_RIGHT_MARGIN
	fi
	if (( BOTTOM_MARGIN < SCREEN_HEIGHT - 1 )); then
		if (( LEFT_MARGIN > 1 )); then
			printf '\x1b[%d;%dHXX' $POST_BOTTOM_MARGIN $PRE_LEFT_MARGIN
		fi
		if (( RIGHT_MARGIN < SCREEN_WIDTH )); then
			printf '\x1b[%d;%dHXX' $POST_BOTTOM_MARGIN $RIGHT_MARGIN
		fi
	fi

	HOME="\x1b[${TOP_MARGIN};${LEFT_MARGIN}H"
elif [ "$STBM" = "1" ]; then
	if [ "$LINES" = "1" ]; then
		# Draw a box around the scroll region. This has to be done
		# on the outside border of the scroll region as the lines
		# within the scroll region are sliding upwards
		printf '\x1b[1;1;%d;%d;1,r' $SCREEN_WIDTH $PRE_TOP_MARGIN      # Top
		printf '\x1b[4;1;%d;%d;1,r' $SCREEN_WIDTH $POST_BOTTOM_MARGIN  # Bottom
	fi

	# Mark the corners too
	printf '\x1b[%dHX\x1b[%d;%dHX' $PRE_TOP_MARGIN $PRE_TOP_MARGIN $SCREEN_WIDTH
	printf '\x1b[%dHX\x1b[%d;%dHX' $POST_BOTTOM_MARGIN $POST_BOTTOM_MARGIN $SCREEN_WIDTH

	HOME="\x1b[${TOP_MARGIN}H"
elif [ "$LRMM" = "1" ]; then
	if [ "$LINES" = "1" ]; then
		# Draw a box around the scroll region. This has to be done
		# on the outside border of the scroll region as the lines
		# within the scroll region are sliding upwards
		printf '\x1b[2;%d;1;1;%d,r' $PRE_LEFT_MARGIN $BOTTOM_MARGIN   # Left
		printf '\x1b[8;%d;1;1;%d,r' $POST_RIGHT_MARGIN $BOTTOM_MARGIN # Right
	fi

	# Mark the corners of the margin
	printf '\x1b[1;%dHX' $PRE_LEFT_MARGIN
	printf '\x1b[1;%dHX' $POST_RIGHT_MARGIN
	printf '\x1b[%d;%dHX' $BOTTOM_MARGIN $PRE_LEFT_MARGIN
	printf '\x1b[%d;%dHX' $BOTTOM_MARGIN $POST_RIGHT_MARGIN

	HOME="\x1b[1;${LEFT_MARGIN}H"
else
	HOME='\x1b[H'
fi

function nextline() {
	if [ "$REVERSE" = "1" ]; then
		printf $HOME
		printf '\x1bM'   # Reverse Index to insert a line
		printf $HOME
	else
		printf '\r\n'
	fi
}


function dolines() {
	for (( i = 1; i <= $1; i++ )); do
		if [ "$REVERSE" = "1" ]; then
			printf $HOME
			printf '\x1bM'   # Reverse Index
		fi

		printf "%03d" $i
		for (( j = 4; j <= $WIDTH; j++ )); do
			printf 'A'
		done

		printf '\r\n'
	done
}

# Enlarge it a little so that the double-height line doesn't immediately
# appear at the bottom.
HEIGHT=$(( HEIGHT + 10 ))

printf $HOME

if (( $HEIGHT > $LEAD_MAX_HEIGHT )); then
	dolines $LEAD_MAX_HEIGHT
else
	dolines $HEIGHT
fi

# A blinking line. Double-height if not LRMM
printf '\x1b[5m'
if [ "$LRMM" = "0" ]; then
	if [ "$REVERSE" = "1" ]; then
		printf $HOME
		printf '\x1bM\x1b#4'
		for (( j = 1 ; j <= WIDTH/2; j++ )); do
			printf 'A'
		done
		printf $HOME
		printf '\x1bM\x1b#3'
		for (( j = 1 ; j <= WIDTH/2; j++ )); do
			printf 'A'
		done
	else
		printf '\x1b#3'
		for (( j = 1 ; j <= WIDTH/2; j++ )); do
			printf 'A'
		done
		printf '\n\x1b#4'
		for (( j = 1 ; j <= WIDTH/2; j++ )); do
			printf 'A'
		done
		printf '\n'
	fi
else
	if [ "$REVERSE" = "1" ]; then
		printf $HOME
		printf '\x1bM'
	fi

	for (( j = 1 ; j <= $WIDTH; j++ )); do
		printf 'A'
	done
	printf '\r\n'
fi

printf '\x1b[m'

if (( $HEIGHT > $TRAIL_MAX_HEIGHT )); then
	dolines $TRAIL_MAX_HEIGHT
else
	dolines $HEIGHT
fi

printf '\x1b[?4;69l'   # Smooth scroll, DECLRMM off
printf '\x1b[r\x1b[s'  # Margins gone
printf '\x1b[23H'
