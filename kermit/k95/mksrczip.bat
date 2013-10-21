echo on
zip -r -9 %1.zip * -x *.ob* -x *.exe -x *.dll -x *.ilk -x *.p* -x comp.out.* -x *.zip -x *.vcp -x bugs/* -x *.bsc -x *.pdb -x rs*.ini -x *.mdp

