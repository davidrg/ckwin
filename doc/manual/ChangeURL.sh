#!/usr/bin/bash
#
# Wed Mar 30 11:02:23 2011
# Use this on the Kermit website when some often-used URL chages.
# Example
#
cd /tmp/fdc || mkdir /tmp/fdc || exit 1
pwd
cd ~/web/k95manual || exit 1

for i in *.html; do
   grep '.htm"' $i > /dev/null || continue
   echo -n $i...
   cat $i | sed -e 's|.htm"|.html"|g' > /tmp/fdc/_x || exit 1
   cp -p $i $i.backup || exit 1
   mv /tmp/fdc/_x $i || exit 1
   chmod 644 $i || exit 1
   echo OK
done
exit
