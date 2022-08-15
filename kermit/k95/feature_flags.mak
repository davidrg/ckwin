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
#
# The following flags are set automatically:
#   CKF_SSH     Turned off when targeting OS/2 or when building with OpenWatcom
#   CKF_CONPTY  Turned on when building with MSC >= 192
#   CKF_SSL     Turned off always (SSL support doesn't currently build)
#
# All other flags should be set prior to starting the build, for example:
#   set CKF_DEBUG=no
#   mkg.bat
#------------------------------------------------------------------------------

!if "$(PLATFORM)" != "NT"
# No built-in SSH support for OS/2 (yet)
CKF_SSH=no
!endif

!if "$(CMP)" == "OWCL"
# No built-in SSH support for OpenWatcom (yet)
CKF_SSH=no
!endif

!if "$(PLATFORM)" == "NT"
!if ($(MSC_VER) >= 192)
# ConPTY on Windows 10+ requires a Platform SDK from late 2018 or newer.
# So we'll only turn this on automatically when building with Visual C++ 2019 or
# later.
CKF_CONPTY=yes
!endif
!endif


# Network Connections are always supported. We only put it here because
# the Watcom nmake clone can't handle empty macros so we need *something* here.
ENABLED_FEATURES = Network-Connections
ENABLED_FEATURE_DEFS = -DNETCONN

DISABLED_FEATURES = SuperLAT DECnet Kerberos SRP Telnet-Encryption CryptDLL
DISABLED_FEATURE_DEFS = -DNO_KERBEROS -DNO_SRP -DNO_ENCRYPTION


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