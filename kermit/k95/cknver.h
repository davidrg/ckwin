#ifndef CKNVER_H
#define CKNVER_H

#include "ckover.h"

/*
 * Version numbers used in Windows Resource Files
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

