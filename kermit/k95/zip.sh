#!/usr/bin/bash
h=~kermit/k95source
z=~kermit/archives/k95source.zip
cd $h || exit 1
pwd
chmod 644 * || exit 1
chgrp kermit * || exit 1
chmod +x zip.sh || exit 1
lynx -dump http://kermit.columbia.edu/k95sourcecode.html > 00README.TXT
rm -f $z ./*.~*~
zip -l $z * -x *.ico *.dat || exit 1 # Zip text files
zip $z *.ico *.dat || exit 1         # Add binary files
cd ~kermit/archives || exit 1
chmod 644 k95source.zip || exit 1
cd ~kermit && ls -ldga archives/k95source.zip
echo
echo Zipfile contents:
echo
unzip -t archives/k95source.zip
echo
echo Done.
exit 0
