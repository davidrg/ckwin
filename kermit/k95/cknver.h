#ifndef VERSION_H
#define VERSION_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/*
 * These are the internal version numbers used by C-Kermit for Windows to make
 * it a bit easier to tell different builds sharing the same C-Kermit base
 * apart (assuming this file is kept up-to-date).
 */
#define VERSION_MAJOR 3
#define VERSION_MINOR 0
#define VERSION_REVISION 5
#ifndef VERSION_BUILD
#define VERSION_BUILD 0
#endif
#define VERSION_YEAR "2024"
#define VERSION_PRODUCT "C-Kermit for Windows"
#define VERSION_PRODUCT_SHORT "CKW"

/* Version number with just the major and minor numbers */
#define APP_VERSION_MAJ_MIN STR(VERSION_MAJOR) "." STR(VERSION_MINOR)

/* Version number with major, minor and revision */
#define APP_VERSION_STR APP_VERSION_MAJ_MIN "." STR(VERSION_REVISION)


/*
 * Version numbers used in the Windows Resource File
 */

/* File version (comma separated), all components */
#define RC_FILE_VERSION VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION,\
    VERSION_BUILD

/* File version (comma separated) string, all components */
#define RC_FILE_VERSION_STR STR(VERSION_MAJOR) ", " STR(VERSION_MINOR) ", " \
    STR(VERSION_REVISION) ", " STR(VERSION_BUILD) "\0"

/* Product version (comma separated), all components */
#define RC_PROD_VERSION RC_FILE_VERSION

/* Product version (comma separated) string, major & minor only */
#define RC_PROD_VERSION_STR APP_VERSION_MAJ_MIN "\0"

#endif // VERSION_H

