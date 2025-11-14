#ifndef CKOVER_H
#define CKOVER_H

/******************************************
 * Kermit 95 Version Number               *
 ******************************************/
/* Only one digit each */
#define K95_VERSION_MAJOR 3
#define K95_VERSION_MINOR 0
/* Up to two digits */
#define K95_VERSION_REVISION 0
/* Numeric version - all three together (MajorMinorRevision) */
#define K95_VERSION_L 3000L
/* Release date */
#define K95_REL_DATE "2025/03/22"
#define K95_REL_DATE_N "20250322"
/* The test build type and release number, if any */
#define K95_TEST_TYPE "Beta"
#define K95_TEST_VER 8
/* This primarily goes in the windows resource scripts */
#define K95_VERSION_YEAR "2024"

/*******************************************
 * -> Remember to update the news text (newstxt)
 *      in ckuus2.c with a summary of what's new!
 * -> For release builds, remember to update the EDITDATE
 *      in ckcmai.c with the release date.
 * -> The edit number should probably be bumped over in ckcmai.c.
 *      This affects C-Kermit too, so it should be done sparingly.
 * -> Version numbers and dates should be bumped in various cko*.c
 *      modules where necessary
 ********************************************/

/* Nothing below here should need updating for a release
 ----------------------------------------------------------------------------*/

#ifndef K95_VERSION_BUILD
#define K95_VERSION_BUILD 0
#endif

#ifndef BETATEST
/* If its not a test build, force the test build type and release number to
 * nothing */
#undef K95_TEST
#undef K95_TEST_VER
#define K95_TEST ""
#define K95_TEST_VER 0
#else /* BETATEST */

/* For development builds between releases, add on the "Pre-" prefix */
#ifdef DEVBUILD
#define K95_TEST "Pre-" K95_TEST_TYPE
#else /* DEVBUILD */
#define K95_TEST K95_TEST_TYPE
#endif /* DEVBUILD */

/* If its a beta test, use the build date as the release date */
#ifdef __DATE__
#undef K95_REL_DATE
#define K95_REL_DATE __DATE__
#endif /* __DATE__ */
#endif /* BETATEST */

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#ifdef COMMIT_SHA
#define K95_COMMIT_SHA STR(COMMIT_SHA)
#endif /* COMMIT_SHA */

/* Version number with just the major and minor numbers */
#define K95_VERSION_MAJ_MIN STR(K95_VERSION_MAJOR) "." STR(K95_VERSION_MINOR)
#define K95_VERSION_MAJ_MIN_REV K95_VERSION_MAJ_MIN "." STR(K95_VERSION_REVISION)

/* Version number with major, minor and revision */
#define K95_VERSION_STR K95_VERSION_MAJ_MIN "." STR(K95_VERSION_REVISION)

#define K95_TEST_VER_S STR(K95_TEST_VER)

#endif /* CKOVER_H */
