#ifndef CKNVER_H
#define CKNVER_H

#include "ckover.h"

/*
 * Version numbers used in Windows Resource Files
 */
#define K95_VERSION_PRODUCT "Kermit 95"
#define K95_VERSION_PRODUCT_SHORT "K95"

/* Version number with just the major and minor numbers */
#define APP_VERSION_MAJ_MIN STR(K95_VERSION_MAJOR) "." STR(K95_VERSION_MINOR)

/* Version number with major, minor and revision */
#define APP_VERSION_STR APP_VERSION_MAJ_MIN "." STR(K95_VERSION_REVISION)


/* File Flags */
#ifdef _DEBUG
#define K95_FF_DEBUG VS_FF_DEBUG
#else
#define K95_FF_DEBUG 0
#endif
#ifdef DEVBUILD
#define K95_FF_DEVBUILD VS_FF_PRERELEASE
#else
#define K95_FF_DEVBUILD 0
#endif
#ifndef K95_COMMIT_SHA
#define K95_FF_PRIVATEBUILD VS_FF_PRIVATEBUILD
#define K95_PRIVATEBUILD_INFO "Developer test build (not from release build system)"
#else
#define K95_FF_PRIVATEBUILD 0
#endif
#ifdef CKB_BUILD_DESC
#define K95_FF_SPECIALBUILD VS_FF_SPECIALBUILD
#define K95_SPECIALBUILD_INFO STR(CKB_BUILD_DESC)
#else
#define K95_FF_SPECIALBUILD 0
#endif


/*
 * Version numbers used in the Windows Resource File
 */

/* File version (comma separated), all components */
#define RC_FILE_VERSION K95_VERSION_MAJOR,K95_VERSION_MINOR,K95_VERSION_REVISION,\
    K95_VERSION_BUILD

/* File version (comma separated) string, all components */
#define RC_FILE_VERSION_STR STR(K95_VERSION_MINOR) ", " STR(K95_VERSION_MINOR) ", " \
    STR(K95_VERSION_REVISION) ", " STR(K95_VERSION_BUILD) "\0"

/* Product version (comma separated), all components */
#define RC_PROD_VERSION RC_FILE_VERSION

/* Product version (comma separated) string, major & minor only */
#define RC_PROD_VERSION_STR K95_VERSION_MAJ_MIN "\0"

#endif // CKNVER_H

