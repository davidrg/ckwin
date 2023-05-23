#
# CKW make-distribution script for cross-builds with GCC
#
# OpenSSL is assumed to live in ../../openssl/openssl-${OPENSSL_VERSION}/
# so make sure you set the OPENSSL_VERSION environment variable!
#

# Set this to where the mingw runtime libraries live. These paths work for
# ubuntu, they may not work for other distributions.
if file nt/cknker.exe | grep --quiet x86-64 ; then
	# 64bit build
	MINGW_RTL_PATH=/usr/lib/gcc/x86_64-w64-mingw32/10-posix/
else
	# 32bit build
	# TODO: Or not x86 at all.
	MINGW_RTL_PATH=/usr/lib/gcc/x86-w32-mingw32/10-posix/
fi	

mkdir ckwin
pushd ckwin
mkdir -p docs/manual
mkdir -p keymaps
mkdir -p public
mkdir -p scripts
mkdir -p users

# Collect binaries
mv ../nt/*.exe ./
mv ../nt/*.dll ./
mv ../win95/*.exe ./
cp ../../../openssl/openssl-${OPENSSL_VERSION}/*.dll ./
# cp ../libssh/libssh-${{env.LIBSSH_VERSION}}/build/src/*.dll ./
mv cknker.exe k95.exe

# Collect runtime libraries.
[ -f "${MINGW_RTL_PATH}libstdc++-6.dll" ] && cp ${MINGW_RTL_PATH}libstdc++-6.dll libstdc++-6.dll
[ -f "${MINGW_RTL_PATH}libgcc_s_seh-1.dll" ] && cp ${MINGW_RTL_PATH}libgcc_s_seh-1.dll libgcc_s_seh-1.dll

# And other files
cp ../cknker.exe.manifest k95.exe.manifest
cp ../k95g.exe.manifest ./
cp ../k95d.cfg ./
cp ../k95.ini ./
cp ../k95custom.ini ./
cp ../welcome.txt ./
cp ../hostmode.bat ./
cp ../../../COPYING ./
[ -f "../../../openssl/openssl-${OPENSSL_VERSION}/LICENSE.txt" ] && cp ../../../openssl/openssl-${OPENSSL_VERSION}/LICENSE.txt COPYING.openssl
[ -f "../../../openssl/openssl-${OPENSSL_VERSION}/LICENSE" ] && cp ../../../openssl/openssl-${OPENSSL_VERSION}/LICENSE COPYING.openssl
# cp ../libssh/libssh-${{env.LIBSSH_VERSION}}/COPYING COPYING.libssh
# cp ../doc/ssh-readme.md ssh-readme.txt

# Documentation
cp ../../../doc/manual/ckwin.htm docs/manual/ckwin.htm
cp ../hostmode.txt docs/

# Keymaps
cp ../vt220.ksc keymaps/
cp ../keycodes.txt keymaps/
cp ../keymaps-readme.txt keymaps/readme.txt
# Normally we'd actually run CKW at this point and get it to dump out its default keymap, but that would require WINE.

# Public
cp ../hostuser.txt public/

# Scripts
cp ../apage.ksc scripts/
cp ../autotel.ksc scripts/
cp ../iksdpy.ksc scripts/
cp ../login.ksc scripts/
cp ../host.ksc scripts/
cp ../hostcom.ksc scripts/
cp ../hostmdm.ksc scripts/
cp ../hostmode.ksc scripts/
cp ../hosttcp.ksc scripts/
cp ../npage.ksc scripts/
cp ../recover.ksc scripts/
cp ../review.ksc scripts/
cp ../rgrep.ksc scripts/
cp ../host.cfg scripts/
cp ../scripts-readme.txt scripts/readme.txt

# Users
cp ../hostmode-greeting.txt users/greeting.txt
cp ../hostmode-help.txt users/hostmode.txt

# CA Certs
wget -O ca_certs.pem https://curl.se/ca/cacert.pem
wget -O ca_certs.license https://www.mozilla.org/media/MPL/2.0/index.815ca599c9df.txt

popd