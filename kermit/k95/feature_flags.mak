###############################################################################
#### Feature Flags ############################################################
###############################################################################
# This makefile processes feature flags (CKF_*) and adds whatever necessary
# preprocessor definitions are required to turn those features on and off.
#
# The results are stored in four macros:
#   DISABLED_FEATURES       Optional features that have been turned OFF
#   DSIABLED_FEATURE_DEFS   The preprocessor definitions to turn those features
#                           off (eg, -DNO_ENCRYPTION)
#   ENABLED_FEATURES        Optional features that have been turned ON
#   ENABLED_FEATURE_DEFS    The preprocessor definitions to turn those features
#                           on (eg, -DZLIB)
#
# The supported feature flags are:
#   Flag           Default    Description
#   CKF_ZLIB       no         ZLIB support
#   CKF_SSL        no         SSL support
#   CKF_SSH        no         libssh support (built-in SSH)
#   CKF_CONPTY     no         Windows PTY support.
#   CKF_DEBUG      yes        Debug logging - on by default
#   CKF_BETATEST   yes        Set to no to do a release build
#   CKF_NO_CRYPTO  no         Disable all cryptography
#   CKF_XYZ        no         X/Y/Z MODEM (Relies on the 'P' library)
#   CKF_MOUSEWHEEL yes        Support for the the mouse wheel
#
# The following flags are set automatically:
#   CKF_SSH     Turned off when targeting OS/2 or when building with OpenWatcom
#   CKF_CONPTY  Turned on when building with MSC >= 192
#   CKF_SSL     Turned off always (SSL support doesn't currently build)
#   CKF_LOGIN   Turned off when building with Visual C++ 5.0 or older
#   CKF_NTLM    Turned off when building with Visual C++ 5.0 or older
#
# All other flags should be set prior to starting the build, for example:
#   set CKF_DEBUG=no
#   mkg.bat
#------------------------------------------------------------------------------

!message Processing feature flags...

# Network Connections are always supported. We only put it here because
# the Watcom nmake clone can't handle empty macros so we need *something* here.
ENABLED_FEATURES = Network-Connections
ENABLED_FEATURE_DEFS = -DNETCONN

DISABLED_FEATURES = SuperLAT DECnet Kerberos SRP Telnet-Encryption CryptDLL
DISABLED_FEATURE_DEFS = -DNO_KERBEROS -DNO_SRP -DNO_ENCRYPTION

!if "$(PLATFORM)" == "NT"
WIN32_VERSION=0x0400

!if "$(CMP)" == "OWCL"
# No built-in SSH support for OpenWatcom (yet)
CKF_SSH=no
!endif

!if ($(MSC_VER) >= 192)
# ConPTY on Windows 10+ requires a Platform SDK from late 2018 or newer.
# So we'll only turn this on automatically when building with Visual C++ 2019 or
# later.
CKF_CONPTY=yes
!endif

!if ($(MSC_VER) <= 110)
# The Platform SDK shipped with Visual C++ 5.0 (Visual Studio 97) and earlier
# doesn't include the necessary headers (security.h, ntsecapi.h, etc) for this
# feature.
CKF_LOGIN=no
CKF_NTLM=no

# Or for scroll wheel support
CKF_MOUSEWHEEL=no

!endif

!if ($(MSC_VER) < 100)
# The Platform SDK shipped with Visual C++ 2.0 lacks quite a lot of stuff
# compared to Visual C++ 4.0 so there is a special target for this level of
# windows.
!message Visual C++ 2.0: setting target to Windows NT 3.50 API level.
CKT_NT31=yes
CKT_NT350=yes
!endif

!if ($(MSC_VER) < 90)
# Visual C++ 1.0 (32-bit edition) and the Win32 SDK only support the APIs
# provided in Windows NT 3.1
CKT_NT31=yes
!endif

# For all versions of windows *EXCEPT* Windows NT 3.1 and 3.50, the target
# minimum version is defined as whatever the compiler happens to support.
# For Windows NT 3.1 and 3.50 the API differences are enough missing APIs
# to require a special macro to exclude references to them. This allows
# NT 3.50 and 3.1 to be targeted with both Visual C++ and OpenWatcom.

!if "$(CKT_NT350)" == "yes"
# These features are available on NT 3.50 but not on NT 3.1
# -> These may appear if/when work to port to NT 3.1 is done.
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DCKT_NT350
!endif

!if "$(CKT_NT31)" == "yes"
# These features are not available on Windows NT 3.50
CKF_TAPI=no
CKF_RICHEDIT=no
CKF_TOOLBAR=no
CKF_LOGIN=no
CKF_NTLM=no
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DCKT_NT31
RC_FEATURE_DEFS = $(RC_FEATURE_DEFS) /dCKT_NT31
!endif

!else

# OS/2 gets NetBIOS support!
CKF_NETBIOS=yes

# And does not get mouse wheel support (not implemented)
CKF_MOUSEWHEEL=no

!if ("$(CMP)" == "OWCL") || ("$(CMP)" == "OWCL386")
# But not when building with OpenWatcom. At the moment it causes Kermit/2 to
# crash on startup at ckonbi.c:152
!message Turning NetBIOS support off - OpenWatcom builds just crash with it enabled.
CKF_NETBIOS=no
!endif

!if "$(CKF_SSH)" == "yes"
!message Target platform is OS/2 - forcing SSH off (not supported)
# No built-in SSH support for OS/2 (yet)
CKF_SSH=no
!endif
!endif


# Other features that should one day be turned on via feature flags once we
# figure out how Fto build them and get any dependencies sorted out.
#
# SFTP:
#   Turn on with -DSFTP_BUILTIN
#   Requires: reimplementing with libssl
# Kerberos:
#   Turn off with: -DNO_KERBEROS
#   Requires: An antique version of MIT Kerberos for Windows. Should be
#             converted to using Heimdal Kerberos
# NetBIOS:
#   Turn on with: -DCK_NETBIOS
#   Requires: ?
# SRP:
#   Turn off with: -DNO_SRP
#   Optionally: -DPRE_SRP_1_7_3
# SuperLAT:
#   Turn on with: -DSUPERLAT
#   Requires: The SuperLAT SDK
# DECnet:
#   Turn on with: -DDECNET
#   Requires: The Pathworks32 SDK
# New URL Highlight:
#   Turn on with: -DNEW_URL_HIGHLIGHT
#   This does: no idea.
# Encryption (DES, CAST and some others)
#   Turn on with: -DCRYPT_DLL
#       and optionally: -DCK_ENCRYPTION
#   Turn off with: -DNO_ENCRYPTION

!if "$(CKF_NO_CRYPTO)" == "yes"
# A No-crypto build has been requested regardless of what libraries may have
# been found. Disable all crypto-related features
CKF_SSH=no
CKF_SSL=no
!endif

# Force SSL off - it doesn't build currently (the OS/2 and NT bits need
# upgrading to at least OpenSSL 1.1.1 if not 3.0)
CKF_SSL=no

# ZLIB:
#   Turn on with: -DZLIB
#   Requires: zlib
#             And also some stuff fixed
!if "$(CKF_ZLIB)" == "yes"
!message CKF_ZLIB set - turning ZLIB on.
ENABLED_FEATURES = $(ENABLED_FEATURES) ZLIB
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DZLIB
!else
DISABLED_FEATURES = $(DISABLED_FEATURES) ZLIB
!endif

# SSL:
#   Turn off with: -DNO_SSL
#   Requires: OpenSSL
#             And also some stuff fixed
!if "$(CKF_SSL)" == "yes"

#ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DOPENSSL_097

# No ZLIB? No OpenSSL Compression.
!if "$(CKF_ZLIB)" != "yes"
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DOPENSSL_NO_COMP
!endif

#SSL_LIBS=libssl.lib libcrypto.lib
SSL_LIBS=ssleay32.lib libeay32.lib

!else
DISABLED_FEATURES = $(DISABLED_FEATURES) SSL
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNO_SSL
!endif

# Built-in SSH support (libssh)
#   Turn on with: -DSSHBUILTIN
#   Turn off with: -DNOSSH
#   Requires: libssh
!if "$(CKF_SSH)" == "yes"
!message CKF_SSH set - turning built-in SSH on.
ENABLED_FEATURES = $(ENABLED_FEATURES) SSH
!else
DISABLED_FEATURES = $(DISABLED_FEATURES) SSH
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNOSSH
!endif

# Windows Pseudoterminal Support (ConPTY)
#   Turn on with: -DCK_CONPTY
#   Requires: Visual C++ 2019 or newer
!if "$(CKF_CONPTY)" == "yes"
ENABLED_FEATURES = $(ENABLED_FEATURES) ConPTY
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DCK_CONPTY
# Needed for STARTUPINFOEX
WIN32_VERSION=0x0600
!else
DISABLED_FEATURES = $(DISABLED_FEATURES) ConPTY
!endif

# Produce a build with no cryptography support at all
#  - probably doesn't work at the moment.
#!if "$(CKF_NO_CRYPTO)" == "yes"
#DISABLED_FEATURES = $(DISABLED_FEATURES) Encryption
#DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNO_ENCRYPTION
#!endif

# If beta-test mode hasn't been explicitly turned off then assume its on.
!if "$(CKF_BETATEST)" != "no"
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DBETATEST
!endif

!if "$(CKF_DEBUG)" == "no"
DISABLED_FEATURES = $(DISABLED_FEATURES) Debug
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNODEBUG
!endif

# XYZMODEM:
#   Turn on with: Maybe -DCK_XYZ
#   Turn off with: -DNOCKXYZ
#   Requires: A currently proprietary library ('P')
!if "$(CKF_XYZ)" == "yes"
ENABLED_FEATURES = $(ENABLED_FEATURES) -DCK_XYZ
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) XYZMODEM
!else
DISABLED_FEATURES = $(DISABLED_FEATURES) XYZMODEM
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNOCKXYZ
!endif

# KUI Toolbar:
#   Turn off with: -DNOTOOLBAR
# Removes the toolbar in K95G which requires features unavailable in the version
# of comctl32.dll shipped prior to NT 3.51.
!if "$(CKF_TOOLBAR)" == "no"
DISABLED_FEATURES = $(DISABLED_FEATURES) Toolbar
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNOTOOLBAR
!endif

# Login:
#   Turn off with: -DNOLOGIN
# Turns authentication for IKS which requires APIs unavailable before NT 3.51.
!if "$(CKF_LOGIN)" == "no"
DISABLED_FEATURES = $(DISABLED_FEATURES) Login
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNOLOGIN
!endif

# NTLM:
#   Turn off with: -DNONTLM
!if "$(CKF_NTLM)" == "no"
DISABLED_FEATURES = $(DISABLED_FEATURES) NTLM
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNONTLM
!endif

# TAPI
#   Turn off with -DNODIAL
# Turns off telephony support
!if "$(CKF_TAPI)" == "no"
DISABLED_FEATURES = $(DISABLED_FEATURES) TAPI
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNODIAL
!endif

# Rich Edit control
#   Turn off with -DNORICHEDIT
# Turns off features relying on the Rich Edit control
!if "$(CKF_RICHEDIT)" == "no"
DISABLED_FEATURES = $(DISABLED_FEATURES) RichEdit
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNORICHEDIT
!endif


# NetBIOS (for OS/2 only)
#   Turn on with -DCK_NETBIOS
!if ("$(PLATFORM)" == "OS2")
!if ("$(CKF_NETBIOS)" == "yes")
ENABLED_FEATURES = $(ENABLED_FEATURES) NetBIOS
ENABLED_FEATURE_DEFS = $(ENABLED_FEATURE_DEFS) -DCK_NETBIOS
!else
DISABLED_FEATURES = $(DISABLED_FEATURES) NetBIOS
!endif
!endif

# Mouse Wheel support
#   Turn off with -DNOSCROLLWHEEL
# Turns off "set mouse wheel" command along with the ability to scroll
# the terminal or do other things with the mouse wheel
!if "$(CKF_MOUSEWHEEL)" == "no"
DISABLED_FEATURES = $(DISABLED_FEATURES) Mouse-Wheel
DISABLED_FEATURE_DEFS = $(DISABLED_FEATURE_DEFS) -DNOSCROLLWHEEL
!endif
