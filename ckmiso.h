/* ckmiso.h -- mapping between 8859/1 and Apple format */
/* March 15, 1989  Paul Placeway */

/*
 * Note that the mapping is incomplete both ways.  This poses quite a
 * problem: do we just leave the blank ones out, or do we cross refrence
 * unrelated characters?  If we do the latter, how do we tell the difference
 * between like and unlike glyphs?
 */

#define ISO1toApple(c)	(((((c) & 0xff) < 0x80) ? ((c) & 0xff) : \
			  (_l1toa[((c) & 0xff) - 0x80])) & 0xff)
#define AppletoISO1(c)	(((((c) & 0xff) < 0x80) ? ((c) & 0xff) : \
			  (_atol1[((c) & 0xff) - 0x80])) & 0xff)

char _l1toa[128] = {
/* these are more control characters in all 8859 char sets */
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,

	0xCA,	/* non-printing space */
	0xC1,	/* inverted ! */
	0xA2,	/* cent */
	0xA3,	/* sterling */
	0xDB,	/* generic curency */
	0xB4,	/* yen */
	   0,	/* broken vert. bar */
	0xA4,	/* section */
	0xAC,	/* dieresis (AKA umlaut) */
	0xA9,	/* copyright   ( (C) ) */
	0xBB,	/* feminine ordinal (underlined superscript a) */
	0xC7,	/* left guillemot (like << ) */
	0xC2,	/* logical not */
	0xD0 /* maybe */,	/* en dash */
	0xA8,	/* registered  ( (R) ) */
	0xF8,	/* macron */

	0xA1,	/* superscript ring */
	0xB1,	/* plus minus */
	   0,	/* superscript 2 */
	   0,	/* superscript 3 */
	0xAB,	/* acute accent */
	0xB5,	/* greek lowercase mu */
	0xA6,	/* paragraph */
	0xA5,	/* bullet */
	0xFC,	/* cedilla */
	   0,	/* superscript 1 */
	0xBC,	/* masculine ordinal (underlined superscript o) */
	0xC8,	/* right guillemot (like >> ) */
	   0,	/* 1/4 */
	   0,	/* 1/2 */
	   0,	/* 3/4 */
	0xC0,	/* inverted ? */

	0xCB,	/* A grave */
	0xE7,	/* A accute */
	0xE5,	/* A circumflex */
	0xCC,	/* A tilde */
	0x80,	/* A dieresis */
	0x81,	/* A ring */
	0xAE,	/* AE */
	0x82,	/* C cedilla */
	0xE9,	/* E grave */
	0x83,	/* E accute */
	0xE6,	/* E circumflex */
	0xE8,	/* E dieresis */
	0xED,	/* I grave */
	0xEA,	/* I accute */
	0xEB,	/* I circumflex */
	0xEC,	/* I dieresis */

	   0,	/* Uppercase Eth */
	0x84,	/* N tilde */
	0xF1,	/* O grave */
	0xEE,	/* O accute */
	0xEF,	/* O circumflex */
	0xCD,	/* O tilde */
	0x85,	/* O dieresis */
	   0,	/* X mark */
	0xAF,	/* O slash */
	0xF4,	/* U grave */
	0xF2,	/* U accute */
	0xF3,	/* U circumflex */
	0x86,	/* U dieresis */
	   0,	/* Y accute */
	   0,	/* Uppercase Thorn */
	0xA7,	/* Es-sed (German double s) */

	0x88,	/* a grave */
	0x87,	/* a accute */
	0x89,	/* a circumflex */
	0x8B,	/* a tilde */
	0x8A,	/* a dieresis */
	0x8C,	/* a ring */
	0xBE,	/* ae */
	0x8D,	/* c cedilla */
	0x8F,	/* e grave */
	0x8E,	/* e accute */
	0x90,	/* e circumflex */
	0x91,	/* e dieresis */
	0x93,	/* i grave */
	0x92,	/* i accute */
	0x94,	/* i circumflex */
	0x95,	/* i dieresis */

	   0,	/* lowercase eth */
	0x96,	/* n tilde */
	0x98,	/* o grave */
	0x97,	/* o accute */
	0x99,	/* o circumflex */
	0x9B,	/* o tilde */
	0x9A,	/* o dieresis */
	0xD6,	/* divide */
	0xBF,	/* o slash */
	0x9D,	/* u grave */
	0x9C,	/* u accute */
	0x9E,	/* u circumflex */
	0x9F,	/* u dieresis */
	   0,	/* y accute */
	   0,	/* lowercase thorn */
	0xD8	/* y dieresis */
};

char _atol1[128] = {
	0xC4,	/* A dieresis */
	0xC5,	/* A ring */
	0xC7,	/* C cedilla */
	0xC9,	/* E accute */
	0xD1,	/* N tilde */
	0xD6,	/* O dieresis */
	0xDC,	/* U dieresis */
	0xE1,	/* a accute */
	0xE0,	/* a grave */
	0xE2,	/* a circumflex */
	0xE4,	/* a dieresis */
	0xE3,	/* a tilde */
	0xE5,	/* a ring */
	0xE7,	/* c cedilla */
	0xE9,	/* e accute */
	0xE8,	/* e grave */

	0xEA,	/* e circumflex */
	0xEB,	/* e dieresis */
	0xED,	/* i accute */
	0xEC,	/* i grave */
	0xEE,	/* i circumflex */
	0xEF,	/* i dieresis */
	0xF1,	/* n tilde */
	0xF3,	/* o accute */
	0xF2,	/* o grave */
	0xF4,	/* o circumflex */
	0xF6,	/* o dieresis */
	0xF5,	/* o tilde */
	0xFA,	/* u accute */
	0xF9,	/* u grave */
	0xFB,	/* u circumflex */
	0xFC,	/* u dieresis */

	   0,	/* dagger */
	0xB0,	/* superscript ring */
	0xA2,	/* cent */
	0xA3,	/* sterling */
	0xA7,	/* section */
	0xB7,	/* bullet */
	0xB6,	/* paragraph */
	0xDF,	/* Es-sed (German double s) */
	0xAE,	/* registered  ( (R) ) */
	0xA9,	/* copyright   ( (C) ) */
	   0,	/* trade mark */
	0xB4,	/* acute accent */
	0xA8,	/* dieresis (AKA umlaut) */
	   0,	/* not equal */
	0xC6,	/* AE */
	0xD8,	/* O slash */

	   0,	/* infinity */
	0xB1,	/* plus minus */
	   0,	/* less than or equal to */
	   0,	/* greater than or equal to */
	0xA5,	/* yen */
	0xB5,	/* greek lowercase mu */
	   0,	/* partial */
	   0,	/* Uppercase Sigma (Summation) */
	   0,	/* Uppercase Pi (Power) */
	   0,	/* lowercase pi */
	   0,	/* integral */
	0xAA,	/* feminine ordinal (underlined superscript a) */
	0xBA,	/* masculine ordinal (underlined superscript o) */
	   0,	/* Uppercase Omega */
	0xE6,	/* ae */
	0xF8,	/* o slash */

	0xBF,	/* inverted ? */
	0xA1,	/* inverted ! */
	0xAC,	/* logical not */
	   0,	/* radical (square root) */
	   0,	/* florin */
	   0,	/* approx equal */
	   0,	/* Uppercase Delta */
	0xAB,	/* left guillemot (like << ) */
	0xBB,	/* right guillemot (like >> ) */
	   0,	/* elipsis (...) */
	0xA0,	/* non-printing space */
	0xC0,	/* A grave */
	0xC3,	/* A tilde */
	0xD5,	/* O tilde */
	   0,	/* OE */
	   0,	/* oe */

	0xAD /* maybe */,	/* en dash */
	   0,	/* em dash */
	   0,	/* left doublequote ( `` ) */
	   0,	/* right doublequote ( '' ) */
	   0,	/* left singlequote ( ` ) */
	   0,	/* right singlequote ( ' ) */
	0xF7,	/* divide */
	   0,	/* lozenge (open diamond) */
	0xFF,	/* y dieresis */
	   0,	/* Y dieresis */
	   0,	/* divide (a / with less slope) */
	0xA4,	/* generic curency */
	   0,	/* single left guil (like < ) */
	   0,	/* single left guil (like > ) */
	   0,	/* fi */
	   0,	/* fl */

	   0,	/* double dagger */
	   0,	/* centered (small) dot */
	   0,	/* baseline single close quote */
	   0,	/* baseline double close quote */
	   0,	/* per thousand */
	0xC2,	/* A circumflex */
	0xCA,	/* E circumflex */
	0xC1,	/* A accute */
	0xCB,	/* E dieresis */
	0xC8,	/* E grave */
	0xCD,	/* I accute */
	0xCE,	/* I circumflex */
	0xCF,	/* I dieresis */
	0xCC,	/* I grave */
	0xD3,	/* O accute */
	0xD4,	/* O circumflex */

	   0,	/* (closed) Apple */
	0xD2,	/* O grave */
	0xDA,	/* U accute */
	0xDB,	/* U circumflex */
	0xD9,	/* U grave */
	   0,	/* dotless i */
	   0,	/* circumflex */
	   0,	/* tilde */
	0xAF,	/* macron */
	   0,	/* breve */
	   0,	/* dot accent */
	   0,	/* ring accent */
	0xB8,	/* cedilla */
	   0,	/* Hungarian umlaut */
	   0,	/* ogonek */
	   0	/* caron */
};
