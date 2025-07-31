#ifndef CKOVER_H
#define CKOVER_H

/******************************************
 * Kermit 95 Version Number               *
 ******************************************/
#define K95_VERSION_MAJOR 3
#define K95_VERSION_MINOR 0
#define K95_VERSION_REVISION 0
#ifndef K95_VERSION_BUILD
#define K95_VERSION_BUILD 0
#endif
#define K95_VERSION_L 3000L
#define K95_TEST "Pre-Beta"
#define K95_TEST_VER 8
/* Remember to update the news text (newstxt)
 * in ckuus2.c with a summary of what's new! */
/*******************************************/

#ifndef K95_VERSION_BUILD
#define K95_VERSION_BUILD 0
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* Version number with just the major and minor numbers */
#define K95_VERSION_MAJ_MIN STR(K95_VERSION_MAJOR) "." STR(K95_VERSION_MINOR)
#define K95_VERSION_MAJ_MIN_REV K95_VERSION_MAJ_MIN "." STR(K95_VERSION_REVISION)

/* Version number with major, minor and revision */
#define K95_VERSION_STR K95_VERSION_MAJ_MIN "." STR(K95_VERSION_REVISION)

#define K95_TEST_VER_S STR(K95_TEST_VER)

#endif /* CKOVER_H */
