: Bourne shell script
##########################################################################
##									##
##  Load a selected set of kermit versions from Columbia host CU20B	##
##									##
##  Original version:	10/17/84					##
##	Rand S. Huntzinger, National Library of Medicine, Bethesda, Md	##
##	Author's Internet address: randy@nlm-vax			##
##									##
##########################################################################
##									##
##	Usage:	getkermit [-v] Kermit-prefix ...			##
##									##
##	where:	-v selects verbose mode (optional).			##
##									##
##		Kermit-prefix is one of the Kermit version prefixes	##
##		used by Columbia to differentiate the different		##
##		versions of Kermit.					##
##									##
##	Examples:							##
##									##
##	    1.	getkermit rt r11					##
##									##
##		This command loads the two RT-11 versions of Kermit	##
##		from CU20B with no feedback to the user's terminal	##
##		except for the startup header.				##
##									##
##	    2.	getkermit -v '' >& log &				##
##									##
##		This one loads all versions of Kermit (a null prefix).	##
##		There is verbose feedback, which the user is directing	##
##		to the file log.  The job is in the background.  This	##
##		is the way I like to run it, since I can look at the	##
##		log to see how the job is progressing.			##
##									##
##########################################################################

cmd=/tmp/getkermit.$$		# Temporary file name
verbose=""			# By default, not verbose
me=`echo $0 | sed 's;.*/;;'`	# My name

echo ""
echo "Load selected Kermit versions from Columbia University"
echo ""

# Test for the optional leading -v (verbose) option

if test x$1 = x-v
then			# -v flag marks verbose
	echo "(verbose option on)"
	verbose="-v"
	shift
fi

# Initialize the ftp control file by writing in the initial commands

cat > $cmd <<!
user anonymous $USER@`hostname`
!

# Loop over the arguments (Kermit distribution prefixes) and build a FTP
# command sequence to retrieve these

if test x$verbose = -v
then
	echo "Building the FTP control file"
fi

while test $# -gt 0
do
	cat >> $cmd <<!
ascii
mget ker:$1*.*
mget ke:$1*.*
tenex
mget kb:$1*.*
!
	shift
done

# End of argument loop, append tail onto ftp command

cat >> $cmd <<!
quit
!

# Create a clean slate directory for the kermit transfer and go there

temp=kerxfer$$
if test x$verbose = x-v
then
	echo Moving to temporary directory $temp
fi

if test -d $temp
then
	echo	Temporary directory $temp already exists.  Aborting...
	exit	1
else
	mkdir $temp
fi
cd $temp
trap\
 "echo User Interrupt, partially processed data in directory $temp; exit 1" 2
trap\
 "echo $me job killed, partially processed data in directory $temp; exit 1" 15

# Run kermit in the new directory

if test x$verbose = x-v
then
	echo	Connecting to CU20B and starting the file transfer.
fi

ftp -i -n $verbose cu20b < $cmd
rm $cmd

# Load complete, convert the TOPS-20 style file names to something which
# look a little better for UNIX to swallow, and put then in the parent
# directory where they really belong.

if test x$verbose = x-v
then
	echo "File transfer complete."
	echo "Coverting TOPS-20 style file names to UNIX form"
fi

for x in *
do
	y=`echo $x | tr A-Z a-z | sed 's;[^>]*.\([^.]*.[^.]*\).*;../\1;' `
	if test x$verbose = x-v
	then
		echo Renaming \"$x\" to \"$y\".
	fi
	mv $x $y
done

# Finally, we can clean up by deleting the temporary directory

rm -rf $temp
if test x$verbose = x-v
then
	echo	$me job completed.
fi
