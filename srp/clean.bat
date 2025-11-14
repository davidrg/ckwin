if exist include\*.h del include\*.h
nmake /f srp.mak CFG="Release" CLEAN
nmake /f tconf.mak CFG="Release" CLEAN
nmake /f krypto.mak CFG="Release" CLEAN