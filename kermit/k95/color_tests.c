/*
 * These are simple tests for the colour attribute macros defined in ckocon.h
 * It should be built and run four times, with the following defined:
 *     16-color debug:   CK_COLORS_DEBUG
 *    16-color normal:   (nothing)
 *          256-color:   CK_COLORS_256
 *         24-bit RGB:   CK_COLORS_24BIT
 * It will, I hope, catch any bugs/errors in the macros which might be difficult
 * to catch otherwise by testing the macros in-place via the terminal emulator.
 *
 * This isn't built using any kind of unit test framework, just a few macros to
 * try and reduce boiler-plate code. It was written to help the port to 24-bit
 * colour.
 */

#include <stdio.h>
#include <windows.h>

/* A few definitions that ckocon.h needs */
#define UCHAR unsigned char
#define USHORT unsigned short
#define CHAR char
#define BOOL int
#define _PROTOTYP( func, parms ) func parms

#define OS2
#define NT

/* This is required to get colour depths above 16 */
#define KUI

#include <ckocon.h>

#undef printf

#define TRUE 1
#define FALSE 0

/* RGB Tables for testing RGB Lookup */

/* The xterm 256-color palette
 *
 * NOTE! These values aren't RGB, they're COLORREF - 0x00bbggrr (ugh)
 */
ULONG RGBTable256[256] = {
    /* First 8 are the standard SGR colors, and the second 8
     * are the intense colors. These are in OS/2 VIO order, *not*
     * SGR/xterm indexed color order  (the number in the comments
     * is the xterm color number */
    0x000000,  /*   0 - Black                      0/  0/  0 */
    0x800000,  /*   4 - Navy (Blue)                0/  0/128 */
    0x008000,  /*   2 - Green                      0/128/  0 */
    0x808000,  /*   3 - Olive (Brown)            128/128/  0 */
    0x000080,  /*   1 - Maroon (Red)             128/  0/  0 */
    0x800080,  /*   5 - Purple (Magenta)         128/  0/128 */
    0x008080,  /*   6 - Teal (Cyan)                0/128/128 */
    0xc0c0c0,  /*   7 - Silver (Light Gray)      192/192/192 */
    0x808080,  /*   8 - Grey (Dark Gray)         128/128/128 */
    0xff0000,  /*  12 - Blue (Light Blue)          0/  0/255 */
    0x00ff00,  /*  10 - Lime (Light Green)         0/255/  0 */
    0xffff00,  /*  11 - Yellow                   255/255/  0 */
    0x0000ff,  /*   9 - Red (Light Red)          255/  0/  0 */
    0xff00ff,  /*  13 - Fuchsia (Light Magenta)  255/  0/255 */
    0x00ffff,  /*  14 - Aqua (Light Cyan)          0/255/255 */
    0xffffff,  /*  15 - White                    255/255/255 */
    /* And the rest is the xterm 256-color palette */
    //BBGGRR
    0x000000,    /*  16 -                            0/  0/  0 */
    0x5F0000,    /*  17 -                            0/  0/ 95 */
    0x870000,    /*  18 -                            0/  0/135 */
    0xAF0000,    /*  19 -                            0/  0/175 */
    0xD70000,    /*  20 -                            0/  0/215 */
    0xFF0000,    /*  21 -                            0/  0/255 */
    0x005F00,    /*  22 -                            0/ 95/  0 */
    0x5F5F00,    /*  23 -                            0/ 95/ 95 */
    0x875F00,    /*  24 -                            0/ 95/135 */
    0xAF5F00,    /*  25 -                            0/ 95/175 */
    0xD75F00,    /*  26 -                            0/ 95/215 */
    0xFF5F00,    /*  27 -                            0/ 95/255 */
    0x008700,    /*  28 -                            0/135/  0 */
    0x5F8700,    /*  29 -                            0/135/ 95 */
    0x878700,    /*  30 -                            0/135/135 */
    0xAF8700,    /*  31 -                            0/135/175 */
    0xD78700,    /*  32 -                            0/135/215 */
    0xFF8700,    /*  33 -                            0/135/255 */
    0x00AF00,    /*  34 -                            0/175/  0 */
    0x5FAF00,    /*  35 -                            0/175/ 95 */
    0x87AF00,    /*  36 -                            0/175/135 */
    0xAFAF00,    /*  37 -                            0/175/175 */
    0xD7AF00,    /*  38 -                            0/175/215 */
    0xFFAF00,    /*  39 -                            0/175/255 */
    0x00D700,    /*  40 -                            0/215/  0 */
    0x5FD700,    /*  41 -                            0/215/ 95 */
    0x87D700,    /*  42 -                            0/215/135 */
    0xAFD700,    /*  43 -                            0/215/175 */
    0xD7D700,    /*  44 -                            0/215/215 */
    0xFFD700,    /*  45 -                            0/215/255 */
    0x00FF00,    /*  46 -                            0/255/  0 */
    0x5FFF00,    /*  47 -                            0/255/ 95 */
    0x87FF00,    /*  48 -                            0/255/135 */
    0xAFFF00,    /*  49 -                            0/255/175 */
    0xD7FF00,    /*  50 -                            0/255/215 */
    0xFFFF00,    /*  51 -                            0/255/255 */
    0x00005F,    /*  52 -                           95/  0/  0 */
    0x5F005F,    /*  53 -                           95/  0/ 95 */
    0x87005F,    /*  54 -                           95/  0/135 */
    0xAF005F,    /*  55 -                           95/  0/175 */
    0xD7005F,    /*  56 -                           95/  0/215 */
    0xFF005F,    /*  57 -                           95/  0/255 */
    0x005F5F,    /*  58 -                           95/ 95/  0 */
    0x5F5F5F,    /*  59 -                           95/ 95/ 95 */
    0x875F5F,    /*  60 -                           95/ 95/135 */
    0xAF5F5F,    /*  61 -                           95/ 95/175 */
    0xD75F5F,    /*  62 -                           95/ 95/215 */
    0xFF5F5F,    /*  63 -                           95/ 95/255 */
    0x00875F,    /*  64 -                           95/135/  0 */
    0x5F875F,    /*  65 -                           95/135/ 95 */
    0x87875F,    /*  66 -                           95/135/135 */
    0xAF875F,    /*  67 -                           95/135/175 */
    0xD7875F,    /*  68 -                           95/135/215 */
    0xFF875F,    /*  69 -                           95/135/255 */
    0x00AF5F,    /*  70 -                           95/175/  0 */
    0x5FAF5F,    /*  71 -                           95/175/ 95 */
    0x87AF5F,    /*  72 -                           95/175/135 */
    0xAFAF5F,    /*  73 -                           95/175/175 */
    0xD7AF5F,    /*  74 -                           95/175/215 */
    0xFFAF5F,    /*  75 -                           95/175/255 */
    0x00D75F,    /*  76 -                           95/215/  0 */
    0x5FD75F,    /*  77 -                           95/215/ 95 */
    0x87D75F,    /*  78 -                           95/215/135 */
    0xAFD75F,    /*  79 -                           95/215/175 */
    0xD7D75F,    /*  80 -                           95/215/215 */
    0xFFD75F,    /*  81 -                           95/215/255 */
    0x00FF5F,    /*  82 -                           95/255/  0 */
    0x5FFF5F,    /*  83 -                           95/255/ 95 */
    0x87FF5F,    /*  84 -                           95/255/135 */
    0xAFFF5F,    /*  85 -                           95/255/175 */
    0xD7FF5F,    /*  86 -                           95/255/215 */
    0xFFFF5F,    /*  87 -                           95/255/255 */
    0x000087,    /*  88 -                          135/  0/  0 */
    0x5F0087,    /*  89 -                          135/  0/ 95 */
    0x870087,    /*  90 -                          135/  0/135 */
    0xAF0087,    /*  91 -                          135/  0/175 */
    0xD70087,    /*  92 -                          135/  0/215 */
    0xFF0087,    /*  93 -                          135/  0/255 */
    0x005F87,    /*  94 -                          135/ 95/  0 */
    0x5F5F87,    /*  95 -                          135/ 95/ 95 */
    0x875F87,    /*  96 -                          135/ 95/135 */
    0xAF5F87,    /*  97 -                          135/ 95/175 */
    0xD75F87,    /*  98 -                          135/ 95/215 */
    0xFF5F87,    /*  99 -                          135/ 95/255 */
    0x008787,    /* 100 -                          135/135/  0 */
    0x5F8787,    /* 101 -                          135/135/ 95 */
    0x878787,    /* 102 -                          135/135/135 */
    0xAF8787,    /* 103 -                          135/135/175 */
    0xD78787,    /* 104 -                          135/135/215 */
    0xFF8787,    /* 105 -                          135/135/255 */
    0x00AF87,    /* 106 -                          135/175/  0 */
    0x5FAF87,    /* 107 -                          135/175/ 95 */
    0x87AF87,    /* 108 -                          135/175/135 */
    0xAFAF87,    /* 109 -                          135/175/175 */
    0xD7AF87,    /* 110 -                          135/175/215 */
    0xFFAF87,    /* 111 -                          135/175/255 */
    0x00D787,    /* 112 -                          135/215/  0 */
    0x5FD787,    /* 113 -                          135/215/ 95 */
    0x87D787,    /* 114 -                          135/215/135 */
    0xAFD787,    /* 115 -                          135/215/175 */
    0xD7D787,    /* 116 -                          135/215/215 */
    0xFFD787,    /* 117 -                          135/215/255 */
    0x00FF87,    /* 118 -                          135/255/  0 */
    0x5FFF87,    /* 119 -                          135/255/ 95 */
    0x87FF87,    /* 120 -                          135/255/135 */
    0xAFFF87,    /* 121 -                          315/255/175 */
    0xD7FF87,    /* 122 -                          135/255/215 */
    0xFFFF87,    /* 123 -                          135/255/255 */
    0x0000AF,    /* 124 -                          175/  0/  0 */
    0x5F00AF,    /* 125 -                          175/  0/ 95 */
    0x8700AF,    /* 126 -                          175/  0/135 */
    0xAF00AF,    /* 127 -                          175/  0/175 */
    0xD700AF,    /* 128 -                          175/  0/215 */
    0xFF00AF,    /* 129 -                          175/  0/255 */
    0x005FAF,    /* 130 -                          175/ 95/  0 */
    0x5F5FAF,    /* 131 -                          175/ 95/ 95 */
    0x875FAF,    /* 132 -                          175/ 95/135 */
    0xAF5FAF,    /* 133 -                          175/ 95/175 */
    0xD75FAF,    /* 134 -                          175/ 95/215 */
    0xFF5FAF,    /* 135 -                          175/ 95/255 */
    0x0087AF,    /* 136 -                          175/135/  0 */
    0x5F87AF,    /* 137 -                          175/135/ 95 */
    0x8787AF,    /* 138 -                          175/135/135 */
    0xAF87AF,    /* 139 -                          175/135/175 */
    0xD787AF,    /* 140 -                          175/135/215 */
    0xFF87AF,    /* 141 -                          175/135/255 */
    0x00AFAF,    /* 142 -                          175/175/  0 */
    0x5FAFAF,    /* 143 -                          175/175/ 95 */
    0x87AFAF,    /* 144 -                          175/175/135 */
    0xAFAFAF,    /* 145 -                          175/175/175 */
    0xD7AFAF,    /* 146 -                          175/175/215 */
    0xFFAFAF,    /* 147 -                          175/175/255 */
    0x00D7AF,    /* 148 -                          175/215/  0 */
    0x5FD7AF,    /* 149 -                          175/215/ 95 */
    0x87D7AF,    /* 150 -                          175/215/135 */
    0xAFD7AF,    /* 151 -                          175/215/175 */
    0xD7D7AF,    /* 152 -                          175/215/215 */
    0xFFD7AF,    /* 153 -                          175/215/255 */
    0x00FFAF,    /* 154 -                          175/255/  0 */
    0x5FFFAF,    /* 155 -                          175/255/ 95 */
    0x87FFAF,    /* 156 -                          175/255/135 */
    0xAFFFAF,    /* 157 -                          175/255/175 */
    0xD7FFAF,    /* 158 -                          175/255/215 */
    0xFFFFAF,    /* 159 -                          175/255/255 */
    0x0000A7,    /* 160 -                          215/  0/  0 */
    0x5F00D7,    /* 161 -                          215/  0/ 95 */
    0x8700D7,    /* 162 -                          215/  0/135 */
    0xAF00D7,    /* 163 -                          215/  0/175 */
    0xD700D7,    /* 164 -                          215/  0/215 */
    0xFF00D7,    /* 165 -                          215/  0/255 */
    0x005FD7,    /* 166 -                          215/ 95/  0 */
    0x5F5FD7,    /* 167 -                          215/ 95/ 95 */
    0x875FD7,    /* 168 -                          215/ 95/135 */
    0xAF5FD7,    /* 169 -                          215/ 95/175 */
    0xD75FD7,    /* 170 -                          215/ 95/215 */
    0xFF5FD7,    /* 171 -                          215/ 95/255 */
    0x0087D7,    /* 172 -                          215/135/  0 */
    0x5F87D7,    /* 173 -                          215/135/ 95 */
    0x8787D7,    /* 174 -                          215/135/135 */
    0xAF87D7,    /* 175 -                          215/135/175 */
    0xD787D7,    /* 176 -                          215/135/215 */
    0xFF87D7,    /* 177 -                          215/135/255 */
    0x00AFD7,    /* 178 -                          215/175/  0 */
    0x5FAFD7,    /* 179 -                          215/175/ 95 */
    0x87AFD7,    /* 180 -                          215/175/135 */
    0xAFAFD7,    /* 181 -                          215/175/175 */
    0xD7AFD7,    /* 182 -                          215/175/215 */
    0xFFAFD7,    /* 183 -                          215/175/255 */
    0x00D7D7,    /* 184 -                          215/215/  0 */
    0x5FD7D7,    /* 185 -                          215/215/ 95 */
    0x87D7D7,    /* 186 -                          215/215/135 */
    0xAFD7D7,    /* 187 -                          215/215/175 */
    0xD7D7D7,    /* 188 -                          215/215/215 */
    0xFFD7D7,    /* 189 -                          215/215/255 */
    0x00FFD7,    /* 190 -                          215/255/  0 */
    0x5FFFD7,    /* 191 -                          215/255/ 95 */
    0x87FFD7,    /* 192 -                          215/255/135 */
    0xAFFFD7,    /* 193 -                          215/255/175 */
    0xD7FFD7,    /* 194 -                          215/255/215 */
    0xFFFFD7,    /* 195 -                          215/255/255 */
    0x0000FF,    /* 196 -                          255/  0/  0 */
    0x5F00FF,    /* 197 -                          255/  0/ 95 */
    0x8700FF,    /* 198 -                          255/  0/135 */
    0xAF00FF,    /* 199 -                          255/  0/175 */
    0xD700FF,    /* 200 -                          255/  0/215 */
    0xFF00FF,    /* 201 -                          255/  0/255 */
    0x005FFF,    /* 202 -                          255/ 95/  0 */
    0x5F5FFF,    /* 203 -                          255/ 95/ 95 */
    0x875FFF,    /* 204 -                          255/ 95/135 */
    0xAF5FFF,    /* 205 -                          255/ 95/175 */
    0xD75FFF,    /* 206 -                          255/ 95/215 */
    0xFF5FFF,    /* 207 -                          255/ 95/255 */
    0x0087FF,    /* 208 -                          255/135/  0 */
    0x5F87FF,    /* 209 -                          255/135/ 95 */
    0x8787FF,    /* 210 -                          255/135/135 */
    0xAF87FF,    /* 211 -                          255/135/175 */
    0xD787FF,    /* 212 -                          255/135/215 */
    0xFF87FF,    /* 213 -                          255/135/255 */
    0x00AFFF,    /* 214 -                          255/175/  0 */
    0x5FAFFF,    /* 215 -                          255/175/ 95 */
    0x87AFFF,    /* 216 -                          255/175/135 */
    0xAFAFFF,    /* 217 -                          255/175/175 */
    0xD7AFFF,    /* 218 -                          255/175/215 */
    0xFFAFFF,    /* 219 -                          255/175/255 */
    0x00D7FF,    /* 220 -                          255/215/  0 */
    0x5FD7FF,    /* 221 -                          255/215/ 95 */
    0x87D7FF,    /* 222 -                          255/215/135 */
    0xAFD7FF,    /* 223 -                          255/215/175 */
    0xD7D7FF,    /* 224 -                          255/215/215 */
    0xFFD7FF,    /* 225 -                          255/215/255 */
    0x00FFFF,    /* 226 -                          255/255/  0 */
    0x5FFFFF,    /* 227 -                          255/255/ 95 */
    0x87FFFF,    /* 228 -                          255/255/135 */
    0xAFFFFF,    /* 229 -                          255/255/175 */
    0xD7FFFF,    /* 230 -                          255/255/215 */
    0xFFFFFF,    /* 231 -                          255/255/255 */
    0x080808,    /* 232 -                            8/  8/  8 */
    0x121212,    /* 233 -                           18/ 18/ 18 */
    0x1C1C1C,    /* 234 -                           28/ 28/ 28 */
    0x262626,    /* 235 -                           38/ 38/ 38 */
    0x303030,    /* 236 -                           48/ 48/ 48 */
    0x3A3A3A,    /* 237 -                           58/ 58/ 58 */
    0x444444,    /* 238 -                           68/ 68/ 68 */
    0x4E4E4E,    /* 239 -                           78/ 78/ 78 */
    0x585858,    /* 240 -                           88/ 88/ 88 */
    0x606060,    /* 241 -                           98/ 98/ 98 */
    0x666666,    /* 242 -                          108/108/108 */
    0x767676,    /* 243 -                          118/118/118 */
    0x808080,    /* 244 -                          128/128/128 */
    0x8A8A8A,    /* 245 -                          138/138/138 */
    0x949494,    /* 246 -                          148/148/148 */
    0x9E9E9E,    /* 247 -                          158/158/158 */
    0xA8A8A8,    /* 248 -                          168/168/168 */
    0xB2B2B2,    /* 249 -                          178/178/178 */
    0xBCBCBC,    /* 250 -                          188/188/188 */
    0xC6C6C6,    /* 251 -                          198/198/198 */
    0xD0D0D0,    /* 252 -                          208/208/208 */
    0xDADADA,    /* 253 -                          218/218/218 */
    0xE4E4E4,    /* 254 -                          228/228/228 */
    0xEEEEEE     /* 255 -                          238/238/238 */
};


/* The xterm 88-color palette
 *
 * NOTE! These values aren't RGB, they're COLORREF - 0x00bbggrr (ugh)
 */
ULONG RGBTable88[88] = {
    /* First 8 are the standard SGR colors, and the second 8
     * are the intense colors. These are in OS/2 VIO order, *not*
     * SGR/xterm indexed color order  (the number in the comments
     * is the xterm color number */
    0x000000,  /*   0 - Black                      0/  0/  0 */
    0x800000,  /*   4 - Navy (Blue)                0/  0/128 */
    0x008000,  /*   2 - Green                      0/128/  0 */
    0x808000,  /*   3 - Olive (Brown)            128/128/  0 */
    0x000080,  /*   1 - Maroon (Red)             128/  0/  0 */
    0x800080,  /*   5 - Purple (Magenta)         128/  0/128 */
    0x008080,  /*   6 - Teal (Cyan)                0/128/128 */
    0xc0c0c0,  /*   7 - Silver (Light Gray)      192/192/192 */
    0x808080,  /*   8 - Grey (Dark Gray)         128/128/128 */
    0xff0000,  /*  12 - Blue (Light Blue)          0/  0/255 */
    0x00ff00,  /*  10 - Lime (Light Green)         0/255/  0 */
    0xffff00,  /*  11 - Yellow                   255/255/  0 */
    0x0000ff,  /*   9 - Red (Light Red)          255/  0/  0 */
    0xff00ff,  /*  13 - Fuchsia (Light Magenta)  255/  0/255 */
    0x00ffff,  /*  14 - Aqua (Light Cyan)          0/255/255 */
    0xffffff,  /*  15 - White                    255/255/255 */
    /* And the rest is the xterm 256-color palette */
    //BBGGRR
    0x000000,  /* 16 -                            00/ 00/ 00 */
    0x00008B,  /* 17 -                            00/ 00/ 8b */
    0x0000CD,  /* 18 -                            00/ 00/ cd */
    0x0000FF,  /* 19 -                            00/ 00/ ff */
    0x008B00,  /* 20 -                            00/ 8b/ 00 */
    0x008B8B,  /* 21 -                            00/ 8b/ 8b */
    0x008BCD,  /* 22 -                            00/ 8b/ cd */
    0x008BFF,  /* 23 -                            00/ 8b/ ff */
    0x00CD00,  /* 24 -                            00/ cd/ 00 */
    0x00CD8B,  /* 25 -                            00/ cd/ 8b */
    0x00CDCD,  /* 26 -                            00/ cd/ cd */
    0x00CDFF,  /* 27 -                            00/ cd/ ff */
    0x00FF00,  /* 28 -                            00/ ff/ 00 */
    0x00FF8B,  /* 29 -                            00/ ff/ 8b */
    0x00FFCD,  /* 30 -                            00/ ff/ cd */
    0x00FFFF,  /* 31 -                            00/ ff/ ff */
    0x8B0000,  /* 32 -                            8b/ 00/ 00 */
    0x8B008B,  /* 33 -                            8b/ 00/ 8b */
    0x8B00CD,  /* 34 -                            8b/ 00/ cd */
    0x8B00FF,  /* 35 -                            8b/ 00/ ff */
    0x8B8B00,  /* 36 -                            8b/ 8b/ 00 */
    0x8B8B8B,  /* 37 -                            8b/ 8b/ 8b */
    0x8B8BCD,  /* 38 -                            8b/ 8b/ cd */
    0x8B8BFF,  /* 39 -                            8b/ 8b/ ff */
    0x8BCD00,  /* 40 -                            8b/ cd/ 00 */
    0x8BCD8B,  /* 41 -                            8b/ cd/ 8b */
    0x8BCDCD,  /* 42 -                            8b/ cd/ cd */
    0x8BCDFF,  /* 43 -                            8b/ cd/ ff */
    0x8BFF00,  /* 44 -                            8b/ ff/ 00 */
    0x8BFF8B,  /* 45 -                            8b/ ff/ 8b */
    0x8BFFCD,  /* 46 -                            8b/ ff/ cd */
    0x8BFFFF,  /* 47 -                            8b/ ff/ ff */
    0xCD0000,  /* 48 -                            cd/ 00/ 00 */
    0xCD008B,  /* 49 -                            cd/ 00/ 8b */
    0xCD00CD,  /* 50 -                            cd/ 00/ cd */
    0xCD00FF,  /* 51 -                            cd/ 00/ ff */
    0xCD8B00,  /* 52 -                            cd/ 8b/ 00 */
    0xCD8B8B,  /* 53 -                            cd/ 8b/ 8b */
    0xCD8BCD,  /* 54 -                            cd/ 8b/ cd */
    0xCD8BFF,  /* 55 -                            cd/ 8b/ ff */
    0xCDCD00,  /* 56 -                            cd/ cd/ 00 */
    0xCDCD8B,  /* 57 -                            cd/ cd/ 8b */
    0xCDCDCD,  /* 58 -                            cd/ cd/ cd */
    0xCDCDFF,  /* 59 -                            cd/ cd/ ff */
    0xCDFF00,  /* 60 -                            cd/ ff/ 00 */
    0xCDFF8B,  /* 61 -                            cd/ ff/ 8b */
    0xCDFFCD,  /* 62 -                            cd/ ff/ cd */
    0xCDFFFF,  /* 63 -                            cd/ ff/ ff */
    0xFF0000,  /* 64 -                            ff/ 00/ 00 */
    0xFF008B,  /* 65 -                            ff/ 00/ 8b */
    0xFF00CD,  /* 66 -                            ff/ 00/ cd */
    0xFF00FF,  /* 67 -                            ff/ 00/ ff */
    0xFF8B00,  /* 68 -                            ff/ 8b/ 00 */
    0xFF8B8B,  /* 69 -                            ff/ 8b/ 8b */
    0xFF8BCD,  /* 70 -                            ff/ 8b/ cd */
    0xFF8BFF,  /* 71 -                            ff/ 8b/ ff */
    0xFFCD00,  /* 72 -                            ff/ cd/ 00 */
    0xFFCD8B,  /* 73 -                            ff/ cd/ 8b */
    0xFFCDCD,  /* 74 -                            ff/ cd/ cd */
    0xFFCDFF,  /* 75 -                            ff/ cd/ ff */
    0xFFFF00,  /* 76 -                            ff/ ff/ 00 */
    0xFFFF8B,  /* 77 -                            ff/ ff/ 8b */
    0xFFFFCD,  /* 78 -                            ff/ ff/ cd */
    0xFFFFFF,  /* 79 -                            ff/ ff/ ff */
    0x2E2E2E,  /* 80 -                            2e/ 2e/ 2e */
    0x5C5C5C,  /* 81 -                            5c/ 5c/ 5c */
    0x737373,  /* 82 -                            73/ 73/ 73 */
    0x8B8B8B,  /* 83 -                            8b/ 8b/ 8b */
    0xA2A2A2,  /* 84 -                            a2/ a2/ a2 */
    0xB9B9B9,  /* 85 -                            b9/ b9/ b9 */
    0xD0D0D0,  /* 86 -                            d0/ d0/ d0 */
    0xE7E7E7,  /* 87 -                            e7/ e7/ e7 */
};

/* The standard 16 color palette.
 *
 * NOTE! These values aren't RGB, they're COLORREF - 0x00bbggrr (ugh)
 */
ULONG RGBTable[16] = {
    /* First 8 are the standard SGR colors, and the second 8
     * are the intense colors. These are in OS/2 VIO order, *not*
     * SGR/xterm indexed color order  (the number in the comments
     * is the xterm color number */
    0x000000,  /*   0 - Black                      0/  0/  0 */
    0x800000,  /*   4 - Navy (Blue)                0/  0/128 */
    0x008000,  /*   2 - Green                      0/128/  0 */
    0x808000,  /*   3 - Olive (Brown)            128/128/  0 */
    0x000080,  /*   1 - Maroon (Red)             128/  0/  0 */
    0x800080,  /*   5 - Purple (Magenta)         128/  0/128 */
    0x008080,  /*   6 - Teal (Cyan)                0/128/128 */
    0xc0c0c0,  /*   7 - Silver (Light Gray)      192/192/192 */
    0x808080,  /*   8 - Grey (Dark Gray)         128/128/128 */
    0xff0000,  /*  12 - Blue (Light Blue)          0/  0/255 */
    0x00ff00,  /*  10 - Lime (Light Green)         0/255/  0 */
    0xffff00,  /*  11 - Yellow                   255/255/  0 */
    0x0000ff,  /*   9 - Red (Light Red)          255/  0/  0 */
    0xff00ff,  /*  13 - Fuchsia (Light Magenta)  255/  0/255 */
    0x00ffff,  /*  14 - Aqua (Light Cyan)          0/255/255 */
    0xffffff,  /*  15 - White                    255/255/255 */
};


/* Both CK_COLORS_256 and CK_COLORS_24BIT support a indexed colors > 15 */
#ifdef CK_COLORS_256
#define EXTENDED_COLORS
#endif
#ifdef CK_COLORS_24BIT
#define EXTENDED_COLORS
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define RUN_TEST(test) output_test_line(STR(test)); if (! ( test() ) ) result = FALSE;

#ifdef EXTENDED_COLORS
#define RUN_EXT_TEST(test) RUN_TEST(test)
#else
#define RUN_EXT_TEST(test) /* test */
#endif

#ifdef CK_COLORS_24BIT
#define RUN_24_TEST(test) RUN_TEST(test)
#else
#define RUN_24_TEST(test) /* test */
#endif

#define FAIL_TEST if (result) {result=FALSE; printf("fail\n"); }
#define END_TEST if(result) printf("pass\n"); return result;

#define ASSERT_BG_EQUALS(attr, expected) if (cell_video_attr_background(attr) != (expected)) { \
    FAIL_TEST; \
    printf("\tbackground should be 0x%2x, got 0x%2x\n", (expected), cell_video_attr_background(attr) ); \
}

#define ASSERT_FG_EQUALS(attr, expected) if (cell_video_attr_foreground(attr) != (expected)) { \
    FAIL_TEST; \
    printf("\tforeground should be 0x%2x, got 0x%2x\n", (expected), cell_video_attr_foreground(attr)); \
}

#define ASSERT_TRUE(a) if (!(a)) { \
FAIL_TEST; \
printf("\tExpected TRUE, got FALSE\n"); \
}


#define ASSERT_TRUE(a) if (!(a)) { \
    FAIL_TEST; \
    printf("\tExpected TRUE, got FALSE\n"); \
}

#define ASSERT_FALSE(a) if ((a)) { \
    FAIL_TEST; \
    printf("\tExpected FALSE, got TRUE\n"); \
}

#define ASSERT_INT_EQUALS(a, b, m) if ((a) != (b)) { \
    FAIL_TEST; \
    printf("\t%s - expected %d, got %d\n", (m), (a), (b)); \
}

#define TEST_LINE_LEN   75

void output_test_line(char* name) {
    int i;
    int len = strlen(name);

    printf(name);
    for (i = len + 1; i < TEST_LINE_LEN; i++) {
        printf(".");
    }
}

/******************************************************************************/
/*** TESTS ********************************************************************/
/******************************************************************************/

/* This is mostly a compile test - if cell_video_attr_t is not a scalar value
 * then the way it needs to be initialised is a bit different. It should
 * initialise the attribute with a pair of 4-bit colors packed into an
 * unsigned char. */
BOOL test_cell_video_attr_init_vio_attribute() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x42);

    ASSERT_BG_EQUALS(attr, 0x4);
    ASSERT_FG_EQUALS(attr, 0x2);

    END_TEST;
}

/* The 24-bit build has various utility macros to try and simplify things */
#ifdef CK_COLORS_24BIT
BOOL test_cell_video_attr_fg_is_indexed_is_set() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr.flags = CK_RGB_FLAG_FG_INDEXED;

    ASSERT_TRUE(cell_video_attr_fg_is_indexed(attr));

    if (result) printf("pass\n");
    return result;
}

BOOL test_cell_video_attr_fg_is_indexed_is_unset() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr.flags = 0xF0;

    ASSERT_FALSE(cell_video_attr_fg_is_indexed(attr));

    END_TEST;
}

BOOL test_cell_video_attr_bg_is_indexed_is_set() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr.flags = CK_RGB_FLAG_BG_INDEXED;

    ASSERT_TRUE(cell_video_attr_bg_is_indexed(attr));

    if (result) printf("pass\n");
    return result;
}

BOOL test_cell_video_attr_bg_is_indexed_is_unset() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr.flags = 0xF0;

    ASSERT_FALSE(cell_video_attr_bg_is_indexed(attr));

    END_TEST;
}

BOOL test_cell_video_attr_bg_is_16_colors() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr.flags = CK_RGB_FLAG_BG_INDEXED;
    attr.bg_b = 15;

    ASSERT_TRUE(cell_video_attr_bg_is_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_fg_is_16_colors() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr.flags = CK_RGB_FLAG_FG_INDEXED;
    attr.fg_b = 15;

    ASSERT_FALSE(cell_video_attr_bg_is_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_bg_is_not_16_colors() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr.flags = CK_RGB_FLAG_BG_INDEXED;
    attr.bg_b = 16;

    ASSERT_FALSE(cell_video_attr_bg_is_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_fg_is_not_16_colors() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr.flags = CK_RGB_FLAG_FG_INDEXED;
    attr.fg_b = 16;

    ASSERT_FALSE(cell_video_attr_bg_is_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_bg_is_not_16_colors_because_not_indexed() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr.flags = 0;
    attr.bg_b = 15;

    ASSERT_FALSE(cell_video_attr_bg_is_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_fg_is_not_16_colors_because_not_indexed() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr.flags = 0;
    attr.fg_b = 15;

    ASSERT_FALSE(cell_video_attr_bg_is_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_set() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(123, 1, 2, 3, 4, 5, 6);

    ASSERT_INT_EQUALS(attr.flags, 123, "flags");
    ASSERT_INT_EQUALS(1, attr.fg_r, "fg_r");
    ASSERT_INT_EQUALS(2, attr.fg_g, "fg_g");
    ASSERT_INT_EQUALS(3, attr.fg_b, "fg_b");
    ASSERT_INT_EQUALS(4, attr.bg_r, "bg_r");
    ASSERT_INT_EQUALS(5, attr.bg_g, "bg_g");
    ASSERT_INT_EQUALS(6, attr.bg_b, "bg_b");

    END_TEST;
}

BOOL test_cell_video_attr_fg_to_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr;
    int color;

    /*                                 R     G     B */
    attr = cell_video_attr_set(123, 0xD7, 0x5F, 0xAF, 4, 5, 6);
    color = cell_video_attr_fg_to_rgb(attr);

    ASSERT_INT_EQUALS(0xAF5FD7, color, "color");

    END_TEST;
}

BOOL test_cell_video_attr_bg_to_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr;
    int color;

    /*                                          R     G     B */
    attr = cell_video_attr_set(123, 1, 2, 3, 0xD7, 0x5F, 0xAF);
    color = cell_video_attr_bg_to_rgb(attr);

    ASSERT_INT_EQUALS(0xAF5FD7, color, "color");

    END_TEST;
}


#endif /* CK_COLORS_24BIT */


BOOL test_cell_video_attr_foreground() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x42);

    ASSERT_FG_EQUALS(attr, 0x2);

    END_TEST;
}

BOOL test_cell_video_attr_background() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x42);

    ASSERT_BG_EQUALS(attr, 0x4);

    END_TEST;
}

#ifdef CK_COLORS_24BIT

BOOL test_cell_video_attr_foreground_ignores_rg() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 0x01, 0x02, 0x02, 0xD7, 0x5F, 0xAF);

    ASSERT_FG_EQUALS(attr, 0x2);

    END_TEST;
}

BOOL test_cell_video_attr_background_ignores_rg() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 0xD7, 0x5F, 0xAF, 0x01, 0x02, 0x04);

    ASSERT_BG_EQUALS(attr, 0x4);

    END_TEST;
}

BOOL test_cell_video_attr_foreground_gives_0_for_rgb_value() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(CK_RGB_FLAG_RGB, 0x01, 0x02, 0x02, 0xD7, 0x5F, 0xAF);

    ASSERT_FG_EQUALS(attr, 0);

    END_TEST;
}

BOOL test_cell_video_attr_background_gives_0_for_rgb_value() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(CK_RGB_FLAG_RGB, 0xD7, 0x5F, 0xAF, 0x01, 0x02, 0x04);

    ASSERT_BG_EQUALS(attr, 0);

    END_TEST;
}

#endif


BOOL test_cell_video_attr_set_fg_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_fg_color(attr, 5);

    ASSERT_FG_EQUALS(attr, 5);

    END_TEST;
}

BOOL test_cell_video_attr_set_bg_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_bg_color(attr, 5);

    ASSERT_BG_EQUALS(attr, 5);

    END_TEST;
}

#ifdef EXTENDED_COLORS
BOOL test_cell_video_attr_set_fg_color_extended() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_fg_color(attr, 42);

    ASSERT_FG_EQUALS(attr, 42);

    END_TEST;
}

BOOL test_cell_video_attr_set_bg_color_extended() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_bg_color(attr, 42);

    ASSERT_BG_EQUALS(attr, 42);

    END_TEST;
}
#endif /* EXTENDED_COLORS */


BOOL test_cell_video_attr_set_3bit_fg_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_3bit_fg_color(attr, 4);
    ASSERT_FG_EQUALS(attr, 4);

    END_TEST;
}

BOOL test_cell_video_attr_set_3bit_bg_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_3bit_bg_color(attr, 4);
    ASSERT_BG_EQUALS(attr, 4);

    END_TEST;
}

/* When a 4-bit value is passed to the 3-bit set macros, it should mask out
 * the 4th bit rather than letting it be overwritten. */
BOOL test_cell_video_attr_set_3bit_fg_color_with_4bit_value() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_3bit_fg_color(attr, 9);
    ASSERT_FG_EQUALS(attr, 1);

    END_TEST;
}

BOOL test_cell_video_attr_set_3bit_bg_color_with_4bit_value() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_3bit_bg_color(attr, 9);
    ASSERT_BG_EQUALS(attr, 1);

    END_TEST;
}

/* This should set the attribute to a pair of 4-bit colors packed into an
 * unsigned char. */
BOOL test_cell_video_attr_from_vio_attribute() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_from_vio_attribute(0x42);

    ASSERT_BG_EQUALS(attr, 0x4);
    ASSERT_FG_EQUALS(attr, 0x2);

    END_TEST;
}

/* cell_video_attr_set_vio_3bit_colors stores a pair of 3-bit colours. The
 * 4th (intensity) bit should be preserved. */
BOOL test_cell_video_attr_set_vio_3bit_colors_preserves_set_4th_bit() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    attr = cell_video_attr_set_vio_3bit_colors(attr, 0x75);
    /* result should be 0xFD if the 4th bit is preserved */

    ASSERT_BG_EQUALS(attr, 0xF);
    ASSERT_FG_EQUALS(attr, 0xD);

    END_TEST;
}

/* cell_video_attr_set_vio_3bit_colors stores a pair of 3-bit colours. The
 * 4th (intensity) bit should be preserved. */
BOOL test_cell_video_attr_set_vio_3bit_colors_preserves_cleared_4th_bit() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_vio_3bit_colors(attr, 0x75);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 0x5);

    END_TEST;
}

#ifdef EXTENDED_COLORS
BOOL test_cell_video_attr_set_vio_3bit_colors_preserves_set_FG_bit4_only() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    /* Both FG and BG are now set to 0xF, a 4-bit color with the intensity bit set */

    /* Now set BG to a value outside the 4-bit color range that happens to have
     * bit 4 set */
    attr = cell_video_attr_set_bg_color(attr, 16);

    attr = cell_video_attr_set_vio_3bit_colors(attr, 0x57);

    ASSERT_BG_EQUALS(attr, 0x5);
    ASSERT_FG_EQUALS(attr, 0xF);

    END_TEST;
}

BOOL test_cell_video_attr_set_vio_3bit_colors_preserves_set_BG_bit4_only() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    /* Both FG and BG are now set to 0xF, a 4-bit color with the intensity bit set */

    /* Now set FG to a value outside the 4-bit color range that happens to have
     * bit 4 set */
    attr = cell_video_attr_set_fg_color(attr, 24);

    /* Set both FG and BG to new 3-bit colors. As BG is currently a 4-color, its
     * intensity bit should be preserved. FG is not a 4-bit colour, so its
     * intensity bit should *not* be preserved
     */
    attr = cell_video_attr_set_vio_3bit_colors(attr, 0x75);

    ASSERT_BG_EQUALS(attr, 0x0F);
    ASSERT_FG_EQUALS(attr, 0x05);

    END_TEST;
}

BOOL test_cell_video_attr_set_vio_3bit_colors_does_notpreserve_bit4() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    /* Both FG and BG are now set to 0xF, a 4-bit color with the intensity bit set */

    /* Now set FG to a value outside the 4-bit color range that happens to have
     * bit 4 set */
    attr = cell_video_attr_set_fg_color(attr, 24);
    attr = cell_video_attr_set_bg_color(attr, 24);

    /* Set both FG and BG to new 3-bit colors. As both FG and BG are currently
     * not 4-bit values, the intesnity bit shouldn't be preserved
     */
    attr = cell_video_attr_set_vio_3bit_colors(attr, 0x55);

    ASSERT_BG_EQUALS(attr, 0x05);
    ASSERT_FG_EQUALS(attr, 0x05);

    END_TEST;
}
#endif /* EXTENDED_COLORS */

#ifdef CK_COLORS_24BIT
BOOL test_cell_video_attr_set_vio_3bit_colors_preserves_set_FG_bit4_bg_rgb() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    /* Both FG and BG are now set to 0xF, a 4-bit color with the intensity bit set */

    /* Now set flags such that only FG holds an indexed value */
    attr.flags = CK_RGB_FLAG_FG_INDEXED;

    attr = cell_video_attr_set_vio_3bit_colors(attr, 0x57);

    ASSERT_BG_EQUALS(attr, 0x5);
    ASSERT_FG_EQUALS(attr, 0xF);

    END_TEST;
}

BOOL test_cell_video_attr_set_vio_3bit_colors_preserves_set_BG_bit4_fg_rgb() {
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    /* Both FG and BG are now set to 0xF, a 4-bit color with the intensity bit set */

    /* Now set flags such that only FG holds an indexed value */
    attr.flags = CK_RGB_FLAG_BG_INDEXED;

    /* Set both FG and BG to new 3-bit colors. As BG is currently a 4-color, its
     * intensity bit should be preserved. FG is not a 4-bit colour, so its
     * intensity bit should *not* be preserved
     */
    attr = cell_video_attr_set_vio_3bit_colors(attr, 0x75);

    ASSERT_BG_EQUALS(attr, 0x0F);
    ASSERT_FG_EQUALS(attr, 0x05);

    END_TEST;
}
#endif

/* This should return two 4-bit colours packed into an unsigned char, arranged
 *   +----+----+
 *   | BG | FG |
 *   +----+----+
 * This can only be done in 16-colour builds. For builds supporting higher
 * colour depths, this macro should not be defined at all.
 */
BOOL test_cell_video_attr_to_win32_console() {
#ifdef CK_COLORS_16
    BOOL result = TRUE;
    int fg, bg;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x42);
    unsigned char attr_c;

    attr_c = cell_video_attr_to_win32_console(attr);

    if ((((attr_c) & 0xF0) >> 4) != (0x04)) {
        FAIL_TEST;
        printf("\tbackground should be 0x%2x, got 0x%2x\n", (((attr_c) & 0xF0) >> 4), 0x04 );
    }

    if (((attr_c) & 0x0F) != 0x02) {
        FAIL_TEST;
        printf("\tbackground should be 0x%2x, got 0x%2x\n", ((attr_c) & 0x0F) != 0x02, 0x02 );
    }

    END_TEST;
#else
#ifdef cell_video_attr_to_win32_console
    printf("fail\n");
    printf("\tcell_video_attr_to_win32_console should not be defined for this build type\n");
#else
    printf("pass\n");
    return TRUE;
#endif
#endif
    return FALSE;
}

#ifdef EXTENDED_COLORS
BOOL test_cell_video_attr_foreground_rgb_xt256() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_XT256;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    ULONG color;

    attr = cell_video_attr_set_fg_color(attr, 120);

    color = cell_video_attr_foreground_rgb(attr);

    ASSERT_INT_EQUALS(0x87FF87, color, "FG RGB Color");

    END_TEST;
}

BOOL test_cell_video_attr_foreground_rgb_xt88() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_XT88;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_fg_color(attr, 68);

    color = cell_video_attr_foreground_rgb(attr);

    ASSERT_INT_EQUALS(0xFF8B00, color, "FG RGB Color");

    END_TEST;
}

BOOL test_cell_video_attr_foreground_rgb_xt88_wraps() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_XT88;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_fg_color(attr, 120);

    color = cell_video_attr_foreground_rgb(attr);

    ASSERT_INT_EQUALS(0x8B0000, color, "FG RGB Color");

    END_TEST;
}
#endif /* EXTENDED_COLORS */

BOOL test_cell_video_attr_foreground_rgb_aixterm16() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_16;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_fg_color(attr, 13);

    color = cell_video_attr_foreground_rgb(attr);

    ASSERT_INT_EQUALS(0xff00ff, color, "FG RGB Color");

    END_TEST;
}

BOOL test_cell_video_attr_foreground_rgb_aixterm16_wraps() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_16;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_fg_color(attr, 20);

    color = cell_video_attr_foreground_rgb(attr);

    ASSERT_INT_EQUALS(0x000080, color, "FG RGB Color");

    END_TEST;
}

#ifdef EXTENDED_COLORS
BOOL test_cell_video_attr_background_rgb_xt256() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_XT256;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    ULONG color;

    attr = cell_video_attr_set_bg_color(attr, 120);

    color = cell_video_attr_background_rgb(attr);

    ASSERT_INT_EQUALS(0x87FF87, color, "BG RGB Color");

    END_TEST;
}

BOOL test_cell_video_attr_background_rgb_xt88() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_XT88;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_bg_color(attr, 68);

    color = cell_video_attr_background_rgb(attr);

    ASSERT_INT_EQUALS(0xFF8B00, color, "GG RGB Color");

    END_TEST;
}

BOOL test_cell_video_attr_background_rgb_xt88_wraps() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_XT88;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_bg_color(attr, 120);

    color = cell_video_attr_background_rgb(attr);

    ASSERT_INT_EQUALS(0x8B0000, color, "BG RGB Color");

    END_TEST;
}
#endif /* EXTENDED_COLORS */

BOOL test_cell_video_attr_background_rgb_aixterm16() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_16;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_bg_color(attr, 13);

    color = cell_video_attr_background_rgb(attr);

    ASSERT_INT_EQUALS(0xff00ff, color, "BG RGB Color");

    END_TEST;
}

BOOL test_cell_video_attr_background_rgb_aixterm16_wraps() {
    BOOL result = TRUE;
    int colorpalette = CK_PALETTE_16;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);
    ULONG color;

    attr = cell_video_attr_set_bg_color(attr, 20);

    color = cell_video_attr_background_rgb(attr);

    ASSERT_INT_EQUALS(0x000080, color, "BG RGB Color");

    END_TEST;
}

#ifdef CK_COLORS_24BIT
BOOL test_cell_video_attr_foreground_rgb_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr;
    int color;
    int colorpalette = CK_PALETTE_XT256;

    /*                                                    R     G     B  BACKGND*/
    attr = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 0xD7, 0x5F, 0xAF, 0, 0, 6);
    color = cell_video_attr_foreground_rgb(attr);

    ASSERT_INT_EQUALS(0xAF5FD7, color, "color");

    END_TEST;
}

BOOL test_cell_video_attr_background_rgb_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr;
    int color;
    int colorpalette = CK_PALETTE_XT256;

    /*                                                 FOREGND     R     G     B */
    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 0, 0, 3, 0xD7, 0x5F, 0xAF);
    color = cell_video_attr_background_rgb(attr);

    ASSERT_INT_EQUALS(0xAF5FD7, color, "color");

    END_TEST;
}
#endif

BOOL test_cell_video_attr_is_null() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    ASSERT_TRUE(cell_video_attr_is_null(attr));

    END_TEST;
}

BOOL test_cell_video_attr_is_not_null_has_fg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x01);

    ASSERT_FALSE(cell_video_attr_is_null(attr));

    END_TEST;
}

BOOL test_cell_video_attr_is_not_null_has_bg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x10);

    ASSERT_FALSE(cell_video_attr_is_null(attr));

    END_TEST;
}

#ifdef CK_COLORS_24BIT
BOOL test_cell_video_attr_is_null_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(CK_RGB_FLAG_RGB, 0, 0, 0, 0, 0, 0);

    ASSERT_TRUE(cell_video_attr_is_null(attr));

    END_TEST;
}

BOOL test_cell_video_attr_is_null_rgb_with_garbage() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    /* For indexed values, the red and green channels are ignored - only the
     * blue channel holds a value. Below we have an indexed FG of 0, and an
     * indexed BG of 0. The Red and Green channges have garbage in them but it
     * doesn't matter as they're not part of the current color.
     */
    attr = cell_video_attr_set(CK_RGB_FLAG_INDEXED, 1, 2, 0, 1, 2, 0);

    ASSERT_TRUE(cell_video_attr_is_null(attr));

    END_TEST;
}


#endif

BOOL test_cell_video_attr_is_equal() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x12);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x12);

    ASSERT_TRUE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

BOOL test_cell_video_attr_is_not_equal() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x12);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x21);

    ASSERT_FALSE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

#ifdef CK_COLORS_24BIT
BOOL test_cell_video_attr_is_equal_fg_rgb_bg_indexed() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 1, 2, 3, 0, 0, 42);
    attr2 = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 1, 2, 3, 3, 4, 42);

    ASSERT_TRUE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

BOOL test_cell_video_attr_is_equal_bg_rgb_fg_indexed() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 42, 1, 2, 3);
    attr2 = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 4, 3, 42, 1, 2, 3);

    ASSERT_TRUE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

BOOL test_cell_video_attr_is_equal_bg_rgb_fg_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 42, 1, 2, 3);
    attr2 = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 42, 1, 2, 3);

    ASSERT_TRUE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

BOOL test_cell_video_attr_is_not_equal_fg_types_differ() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set(CK_RGB_FLAG_INDEXED, 1, 2, 3, 3, 4, 42);
    attr2 = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 1, 2, 3, 3, 4, 42);

    ASSERT_FALSE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

BOOL test_cell_video_attr_is_not_equal_bg_types_differ() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set(CK_RGB_FLAG_INDEXED, 1, 2, 3, 3, 4, 42);
    attr2 = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 3, 3, 4, 42);

    ASSERT_FALSE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

BOOL test_cell_video_attr_is_not_equal_bg_rgb_fg_indexed_fg_differs() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 42, 1, 2, 3);
    attr2 = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 43, 1, 2, 3);

    ASSERT_FALSE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

BOOL test_cell_video_attr_is_not_equal_bg_rgb_fg_indexed_bg_differs() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);
    cell_video_attr_t attr2 = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 42, 1, 2, 3);
    attr2 = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 42, 1, 3, 3);

    ASSERT_FALSE(cell_video_attr_equal(attr, attr2));

    END_TEST;
}

#endif /* CK_COLORS_24BIT */

/* This inverts the intensity bit. Its used only in ckctel.c to temporarily
 * invert the foreground intensity for telnet debug messages so that they stand
 * out.
 * TODO: For higher colour depths: ??? Maybe use a VT attribute ???
 */
/* (attr) (cell_video_attr_from_vio_attribute((attr).a ^ 0x8))
* */
BOOL test_cell_video_attr_with_fg_intensity_toggled_clears_set_intensity() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    attr = cell_video_attr_with_fg_intensity_toggled(attr);

    ASSERT_BG_EQUALS(attr, 0xF);
    ASSERT_FG_EQUALS(attr, 0x7);

    END_TEST;
}

BOOL test_cell_video_attr_with_fg_intensity_toggled_sets_cleared_intensity() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x77);

    attr = cell_video_attr_with_fg_intensity_toggled(attr);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 0xF);

    END_TEST;
}

#ifdef EXTENDED_COLORS
/* Check colors outside the 4-bit range don't have their intensity toggled */
BOOL test_cell_video_attr_with_fg_intensity_toggled_leaves_non_4bit_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x77);

    attr = cell_video_attr_set_fg_color(attr, 42);

    attr = cell_video_attr_with_fg_intensity_toggled(attr);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 42);

    END_TEST;
}
#endif /* EXTENDED_COLORS */

#ifdef CK_COLORS_24BIT
/* Check RGB colors don't have their intensity toggled */
BOOL test_cell_video_attr_with_fg_intensity_toggled_leaves_rgb_color() {
    BOOL result = TRUE;
    int color;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x77);

    /* Set foreground to RGB, background to indexed    FOREGND   BACKGND*/
    attr = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 1, 2, 42, 43, 44, 45);

    /* Grab a copy of the foreground RGB color */
    color = cell_video_attr_fg_to_rgb(attr);

    /* Try to toggle the foreground intensity bit - this should do nothing as
     * the foreground isn't an indexed colour */
    attr = cell_video_attr_with_fg_intensity_toggled(attr);

    /* BG should be unchanged as we were never doing anything with that anyway*/
    ASSERT_BG_EQUALS(attr, 45);
    ASSERT_INT_EQUALS(color, cell_video_attr_fg_to_rgb(attr), "color");

    END_TEST;
}
#endif /*CK_COLORS_24BIT */

BOOL test_cell_video_attr_with_bg_intensity_set_sets_intensity() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x77);

    attr = cell_video_attr_with_bg_intensity_set(attr);

    ASSERT_BG_EQUALS(attr, 0xF);
    ASSERT_FG_EQUALS(attr, 0x7);

    END_TEST;
}

#ifdef EXTENDED_COLORS
/* Check colors outside the 4-bit range don't have their intensity toggled */
BOOL test_cell_video_attr_with_bg_intensity_set_leaves_non_4bit_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x77);

    attr = cell_video_attr_set_bg_color(attr, 42);

    attr = cell_video_attr_with_bg_intensity_set(attr);

    ASSERT_BG_EQUALS(attr, 42);
    ASSERT_FG_EQUALS(attr, 0x7);

    END_TEST;
}
#endif /* EXTENDED_COLORS */

#ifdef CK_COLORS_24BIT
/* Check RGB colors don't have their intensity toggled */
BOOL test_cell_video_attr_with_bg_intensity_set_leaves_rgb_color() {
    BOOL result = TRUE;
    int color;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    /* Set background to RGB, background to indexed    FOREGND   BACKGND*/
    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 42, 43, 44, 45);

    /* Grab a copy of the foreground RGB color */
    color = cell_video_attr_bg_to_rgb(attr);

    /* Try to set the background intensity bit - this should do nothing as
     * the foreground isn't an indexed colour */
    attr = cell_video_attr_with_bg_intensity_set(attr);

    /* BG should be unchanged as we were never doing anything with that anyway*/
    ASSERT_FG_EQUALS(attr, 42);
    ASSERT_INT_EQUALS(color, cell_video_attr_bg_to_rgb(attr), "color");

    END_TEST;
}
#endif /*CK_COLORS_24BIT */


BOOL test_cell_video_attr_with_fg_intensity_set_sets_intensity() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x77);

    attr = cell_video_attr_with_fg_intensity_set(attr);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 0xF);

    END_TEST;
}

#ifdef EXTENDED_COLORS
/* Check colors outside the 4-bit range don't have their set toggled */
BOOL test_cell_video_attr_with_fg_intensity_set_leaves_non_4bit_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x77);

    attr = cell_video_attr_set_fg_color(attr, 42);

    attr = cell_video_attr_with_fg_intensity_set(attr);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 42);

    END_TEST;
}
#endif /* EXTENDED_COLORS */

#ifdef CK_COLORS_24BIT
/* Check RGB colors don't have their intensity toggled */
BOOL test_cell_video_attr_with_fg_intensity_set_leaves_rgb_color() {
    BOOL result = TRUE;
    int color;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    /* Set background to RGB, background to indexed    FOREGND   BACKGND*/
    attr = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 1, 2, 42, 43, 44, 45);

    /* Grab a copy of the foreground RGB color */
    color = cell_video_attr_fg_to_rgb(attr);

    /* Try to set the background intensity bit - this should do nothing as
     * the foreground isn't an indexed colour */
    attr = cell_video_attr_with_fg_intensity_set(attr);

    /* BG should be unchanged as we were never doing anything with that anyway*/
    ASSERT_BG_EQUALS(attr, 45);
    ASSERT_INT_EQUALS(color, cell_video_attr_fg_to_rgb(attr), "color");

    END_TEST;
}
#endif /*CK_COLORS_24BIT */

BOOL test_cell_video_attr_with_fg_intensity_unset_unsets_intensity() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    attr = cell_video_attr_with_fg_intensity_unset(attr);

    ASSERT_BG_EQUALS(attr, 0xF);
    ASSERT_FG_EQUALS(attr, 0x7);

    END_TEST;
}

#ifdef EXTENDED_COLORS
/* Check colors outside the 4-bit range don't have their set toggled */
BOOL test_cell_video_attr_with_fg_intensity_unset_leaves_non_4bit_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    attr = cell_video_attr_set_fg_color(attr, 0xFF);

    attr = cell_video_attr_with_fg_intensity_unset(attr);

    ASSERT_BG_EQUALS(attr, 0xF);
    ASSERT_FG_EQUALS(attr, 0xFF);

    END_TEST;
}
#endif /* EXTENDED_COLORS */

#ifdef CK_COLORS_24BIT
/* Check RGB colors don't have their intensity toggled */
BOOL test_cell_video_attr_with_fg_intensity_unset_leaves_rgb_color() {
    BOOL result = TRUE;
    int color;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    /* Set background to RGB, background to indexed    FOREGND   BACKGND*/
    attr = cell_video_attr_set(CK_RGB_FLAG_BG_INDEXED, 1, 2, 0xFF, 43, 44, 0xF);

    /* Grab a copy of the foreground RGB color */
    color = cell_video_attr_fg_to_rgb(attr);

    /* Try to set the background intensity bit - this should do nothing as
     * the foreground isn't an indexed colour */
    attr = cell_video_attr_with_fg_intensity_unset(attr);

    /* BG should be unchanged as we were never doing anything with that anyway*/
    ASSERT_BG_EQUALS(attr, 0xF);
    ASSERT_INT_EQUALS(color, cell_video_attr_fg_to_rgb(attr), "color");

    END_TEST;
}
#endif /*CK_COLORS_24BIT */

BOOL test_cell_video_attr_with_bg_intensity_unset_unsets_intensity() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    attr = cell_video_attr_with_bg_intensity_unset(attr);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 0xF);

    END_TEST;
}

#ifdef EXTENDED_COLORS
/* Check colors outside the 4-bit range don't have their set toggled */
BOOL test_cell_video_attr_with_bg_intensity_unset_leaves_non_4bit_color() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    attr = cell_video_attr_set_bg_color(attr, 0xFF);

    attr = cell_video_attr_with_bg_intensity_unset(attr);

    ASSERT_FG_EQUALS(attr, 0xF);
    ASSERT_BG_EQUALS(attr, 0xFF);

    END_TEST;
}
#endif /* EXTENDED_COLORS */

#ifdef CK_COLORS_24BIT
/* Check RGB colors don't have their intensity toggled */
BOOL test_cell_video_attr_with_bg_intensity_unset_leaves_rgb_color() {
    BOOL result = TRUE;
    int color;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xFF);

    /* Set background to RGB, background to indexed    FOREGND   BACKGND*/
    attr = cell_video_attr_set(CK_RGB_FLAG_FG_INDEXED, 1, 2, 0xF, 43, 44, 0xFF);

    /* Grab a copy of the foreground RGB color */
    color = cell_video_attr_bg_to_rgb(attr);

    /* Try to set the background intensity bit - this should do nothing as
     * the foreground isn't an indexed colour */
    attr = cell_video_attr_with_bg_intensity_unset(attr);

    /* FG should be unchanged as we were never doing anything with that anyway*/
    ASSERT_FG_EQUALS(attr, 0xF);
    ASSERT_INT_EQUALS(color, cell_video_attr_bg_to_rgb(attr), "color");

    END_TEST;
}
#endif /*CK_COLORS_24BIT */

BOOL test_cell_video_attr_set_colors() {
    BOOL result = TRUE;
    int color;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_colors(9, 14);

    ASSERT_FG_EQUALS(attr, 9);
    ASSERT_BG_EQUALS(attr, 14);

    END_TEST;
}

#ifdef EXTENDED_COLORS
BOOL test_cell_video_attr_set_colors_extended() {
    BOOL result = TRUE;
    int color;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_colors(109, 214);

    ASSERT_FG_EQUALS(attr, 109);
    ASSERT_BG_EQUALS(attr, 214);

    END_TEST;
}
#endif

#ifdef CK_COLORS_24BIT
BOOL test_cell_video_attr_set_fg_rgb() {
    BOOL result = TRUE;
    int color;
    int colorpalette = CK_PALETTE_XT256;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_bg_color(attr, 214);
    attr = cell_video_attr_set_fg_rgb(attr, 0xD7, 0x5F, 0xAF);

    color = cell_video_attr_foreground_rgb(attr);

    ASSERT_BG_EQUALS(attr, 214);
    ASSERT_INT_EQUALS(0xAF5FD7, color, "color");

    END_TEST;
}

BOOL test_cell_video_attr_set_bg_rgb() {
    BOOL result = TRUE;
    int color = 0;
    int colorpalette = CK_PALETTE_XT256;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x00);

    attr = cell_video_attr_set_fg_color(attr, 214);

    ASSERT_INT_EQUALS(CK_RGB_FLAG_INDEXED, attr.flags, "flags pre rgb-set");

    attr = cell_video_attr_set_bg_rgb(attr, 0xD7, 0x5F, 0xAF);

    color = cell_video_attr_background_rgb(attr);

    ASSERT_FG_EQUALS(attr, 214);
    ASSERT_INT_EQUALS(0xAF5FD7, color, "color");
    ASSERT_INT_EQUALS(CK_RGB_FLAG_FG_INDEXED, attr.flags, "flags");

    END_TEST;
}
#endif /* CK_COLORS_24BIT */

BOOL test_swapcolors_3bit() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x12);

    attr = swapcolors(attr);

    ASSERT_BG_EQUALS(attr, 2);
    ASSERT_FG_EQUALS(attr, 1);

    END_TEST;
}

BOOL test_swapcolors_preserves_bg_high() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xF2);

    attr = swapcolors(attr);

    ASSERT_BG_EQUALS(attr, 0xA);
    ASSERT_FG_EQUALS(attr, 0x7);

    END_TEST;
}

BOOL test_swapcolors_preserves_fg_high() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = swapcolors(attr);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 0xA);

    END_TEST;
}

#ifdef EXTENDED_COLORS
BOOL test_swapcolors_does_not_preserve_4th_bit_for_fg_color_above_15() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xF2);

    /* 103 does not have bit 4 set. If bit 4 gets preserved, it should become
     * 111 */
    attr = cell_video_attr_set_fg_color(attr,103);

    attr = swapcolors(attr);

    ASSERT_BG_EQUALS(attr, 103);
    ASSERT_FG_EQUALS(attr, 0x7);

#ifdef CK_COLORS_24BIT
    ASSERT_INT_EQUALS(0, attr.bg_r, "bg_r");
    ASSERT_INT_EQUALS(0, attr.bg_g, "bg_g");
#endif

    END_TEST;
}

BOOL test_swapcolors_does_not_preserve_4th_bit_for_bg_color_above_15() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    /* 103 does not have bit 4 set. If bit 4 gets preserved, it should become
     * 111 */
    attr = cell_video_attr_set_bg_color(attr,103);

    attr = swapcolors(attr);

    ASSERT_FG_EQUALS(attr, 103);
    ASSERT_BG_EQUALS(attr, 0x7);

#ifdef CK_COLORS_24BIT
    ASSERT_INT_EQUALS(0, attr.fg_r, "bg_r");
    ASSERT_INT_EQUALS(0, attr.fg_g, "bg_g");
#endif

    END_TEST;
}

BOOL test_swapcolors_does_not_preserve_4th_bit_for_both_colors_above_15() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    /* 103 does not have bit 4 set. If bit 4 gets preserved, it should become
     * 111 */
    attr = cell_video_attr_set_bg_color(attr,103);
    attr = cell_video_attr_set_fg_color(attr,203);

    attr = swapcolors(attr);

    ASSERT_FG_EQUALS(attr, 103);
    ASSERT_BG_EQUALS(attr, 203);

    END_TEST;
}
#endif

#ifdef CK_COLORS_24BIT

/* These are helper macros to make the main swapcolors macro a bit more manageable */
BOOL test_rgbsc_swap_4b_3bit() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x12);

    attr = rgbsc_swap_4b(attr);

    ASSERT_BG_EQUALS(attr, 2);
    ASSERT_FG_EQUALS(attr, 1);

    END_TEST;
}

BOOL test_rgbsc_swap_4b_preserves_bg_high() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xF2);

    attr = rgbsc_swap_4b(attr);

    ASSERT_BG_EQUALS(attr, 0xA);
    ASSERT_FG_EQUALS(attr, 0x7);

    END_TEST;
}

BOOL test_rgbsc_swap_4b_preserves_fg_high() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = rgbsc_swap_4b(attr);

    ASSERT_BG_EQUALS(attr, 0x7);
    ASSERT_FG_EQUALS(attr, 0xA);

    END_TEST;
}

BOOL test_rgbsc_swap_4b_bg_3b() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(123, 1, 2, 3, 4, 5, 6);

    attr = rgbsc_swap_4b_bg(attr);

    /* Flags should be unchaged. All the FG values (1,2,3) should go into BG,
     * while the BG value should go unchanged into fg_b while fg_r and fg_g get
     * zeroed */
    ASSERT_INT_EQUALS(123, attr.flags, "flags");
    ASSERT_INT_EQUALS(0, attr.fg_r, "fg_r");
    ASSERT_INT_EQUALS(0, attr.fg_g, "fg_g");
    ASSERT_INT_EQUALS(6, attr.fg_b, "fg_b");
    ASSERT_INT_EQUALS(1, attr.bg_r, "bg_r");
    ASSERT_INT_EQUALS(2, attr.bg_g, "bg_g");
    ASSERT_INT_EQUALS(3, attr.bg_b, "bg_b");

    END_TEST;
}

BOOL test_rgbsc_swap_4b_bg_4b() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(123, 1, 2, 3, 4, 5, 0xF);

    attr = rgbsc_swap_4b_bg(attr);

    /* Flags should be unchaged. All the FG values (1,2,3) should go into BG,
     * while the BG value should go have the 4th bit chopped off and go into
     * fg_b while fg_r and fg_g get zeroed */
    ASSERT_INT_EQUALS(123, attr.flags, "flags");
    ASSERT_INT_EQUALS(0, attr.fg_r, "fg_r");
    ASSERT_INT_EQUALS(0, attr.fg_g, "fg_g");
    ASSERT_INT_EQUALS(0x7, attr.fg_b, "fg_b");
    ASSERT_INT_EQUALS(1, attr.bg_r, "bg_r");
    ASSERT_INT_EQUALS(2, attr.bg_g, "bg_g");
    ASSERT_INT_EQUALS(3, attr.bg_b, "bg_b");

    END_TEST;
}

BOOL test_rgbsc_swap_4b_fg_3b() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(123, 4, 5, 6, 1, 2, 3 );

    attr = rgbsc_swap_4b_fg(attr);

    /* Flags should be unchaged. All the BG values (1,2,3) should go into FG,
     * while the FG value should go unchanged into bg_b while bg_r and bg_g get
     * zeroed */
    ASSERT_INT_EQUALS(123, attr.flags, "flags");
    ASSERT_INT_EQUALS(0, attr.bg_r, "fg_r");
    ASSERT_INT_EQUALS(0, attr.bg_g, "fg_g");
    ASSERT_INT_EQUALS(6, attr.bg_b, "fg_b");
    ASSERT_INT_EQUALS(1, attr.fg_r, "bg_r");
    ASSERT_INT_EQUALS(2, attr.fg_g, "bg_g");
    ASSERT_INT_EQUALS(3, attr.fg_b, "bg_b");

    END_TEST;
}

BOOL test_rgbsc_swap_4b_fg_4b() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    attr = cell_video_attr_set(123, 4, 5, 0xF, 1, 2, 3);

    attr = rgbsc_swap_4b_fg(attr);

    /* Flags should be unchaged. All the BG values (1,2,3) should go into FG,
     * while the FG value should go have the 4th bit chopped off and go into
     * bg_b while bg_r and bg_g get zeroed */
    ASSERT_INT_EQUALS(123, attr.flags, "flags");
    ASSERT_INT_EQUALS(0, attr.bg_r, "fg_r");
    ASSERT_INT_EQUALS(0, attr.bg_g, "fg_g");
    ASSERT_INT_EQUALS(0x7, attr.bg_b, "fg_b");
    ASSERT_INT_EQUALS(1, attr.fg_r, "bg_r");
    ASSERT_INT_EQUALS(2, attr.fg_g, "bg_g");
    ASSERT_INT_EQUALS(3, attr.fg_b, "bg_b");

    END_TEST;
}

BOOL test_rgbsc_swap_all_swaps_all() {
    BOOL result = TRUE;
    cell_video_attr_t attr;

    /*                        flags FGR  FGG  FGB  BGR  BGB  BGG */
    attr = cell_video_attr_set(123,   4,   5, 0xF,   1,   2,   3);

    attr = rgbsc_swap_all(attr);

    /* Flags should be unchaged. All the BG values (1,2,3) should go into FG,
     * while the FG value should go have the 4th bit chopped off and go into
     * bg_b while bg_r and bg_g get zeroed */
    ASSERT_INT_EQUALS(123, attr.flags, "flags");
    ASSERT_INT_EQUALS(1, attr.fg_r, "fg_r");
    ASSERT_INT_EQUALS(2, attr.fg_g, "fg_g");
    ASSERT_INT_EQUALS(3, attr.fg_b, "fg_b");
    ASSERT_INT_EQUALS(4, attr.bg_r, "bg_r");
    ASSERT_INT_EQUALS(5, attr.bg_g, "bg_g");
    ASSERT_INT_EQUALS(0xF, attr.bg_b, "bg_b");

    END_TEST;
}

BOOL test_cell_video_attr_is_rgb_is_false_for_indexed_pair() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    ASSERT_FALSE(cell_video_attr_is_rgb(attr));

    END_TEST;
}

BOOL test_cell_video_attr_is_rgb_is_false_for_indexed_fg_rgb_bg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = cell_video_attr_set_bg_rgb(attr, 4, 5, 6);

    ASSERT_FALSE(cell_video_attr_is_rgb(attr));

    END_TEST;
}

BOOL test_cell_video_attr_is_rgb_is_false_for_indexed_bg_rgb_fg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = cell_video_attr_set_fg_rgb(attr, 4, 5, 6);

    ASSERT_FALSE(cell_video_attr_is_rgb(attr));

    END_TEST;
}

BOOL test_cell_video_attr_is_rgb_is_true_for_rgb_fg_rgb_bg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = cell_video_attr_set_fg_rgb(attr, 4, 5, 6);
    attr = cell_video_attr_set_bg_rgb(attr, 4, 5, 6);

    ASSERT_TRUE(cell_video_attr_is_rgb(attr));

    END_TEST;
}


BOOL test_cell_video_attr_fgbg_are_16colors_is_true_for_3bit_fgbg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);


    ASSERT_TRUE(cell_video_attr_fgbg_are_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_fgbg_are_16colors_is_false_for_ext_fg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = cell_video_attr_set_fg_color(attr, 23);

    ASSERT_FALSE(cell_video_attr_fgbg_are_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_fgbg_are_16colors_is_false_for_ext_bg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = cell_video_attr_set_bg_color(attr, 23);

    ASSERT_FALSE(cell_video_attr_fgbg_are_16colors(attr));

    END_TEST;
}

BOOL test_cell_video_attr_fgbg_are_16colors_is_false_for_ext_fgbg() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);

    attr = cell_video_attr_set_fg_color(attr, 23);
    attr = cell_video_attr_set_bg_color(attr, 25);

    ASSERT_FALSE(cell_video_attr_fgbg_are_16colors(attr));

    END_TEST;
}

BOOL test_rgbsc_swap_flags_does_nothing_for_rgb_fgbg() {
    BOOL result = TRUE;
    /* 0x64 does not have bits 0 or 1 set. All other bits should be
     * preserved */
    ASSERT_INT_EQUALS(0x64, rgbsc_swap_flags(0x64), "flags");
    END_TEST;
}

BOOL test_rgbsc_swap_flags_does_nothing_for_indexed_fgbg() {
    BOOL result = TRUE;
    /* 0x64 has bits 0 or 1 set. All other bits should be preserved */
    ASSERT_INT_EQUALS(0x67, rgbsc_swap_flags(0x67), "flags");
    END_TEST;
}

BOOL test_rgbsc_swap_flags_swaps_indexed_fg() {
    BOOL result = TRUE;
    /* 0x65 has bits 0 set. Result should have bit 1 set and all other bits preserved. */
    ASSERT_INT_EQUALS(0x66, rgbsc_swap_flags(0x65), "flags");
    END_TEST;
}

BOOL test_rgbsc_swap_flags_swaps_indexed_bg() {
    BOOL result = TRUE;
    /* 0x66 has bits 1 set. Result should have bit 0 set and all other bits preserved. */
    ASSERT_INT_EQUALS(0x65, rgbsc_swap_flags(0x66), "flags");
    END_TEST;
}


/*** End swapcolors helper macros ***/


BOOL test_swapcolors_does_not_preserve_4th_bit_for_fg_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0xF2);
    int color;
    int colorpalette = CK_PALETTE_XT256;

    /* Currently, BG is 0x0F, FG is 0x02 */

    /* Set FG to an RGB value with 0x67 in the blue channel. 0x67 does
     * not have the 4th bit set so we should be able to tell if that bit
     * is incorrectly preserved */
    attr = cell_video_attr_set_fg_rgb(attr, 0xD7, 0x5F, 0x67);

    /* Grab a copy of the current FG as an RGB value so we can compare it
     * later. We should find this value in the background. */
    color = cell_video_attr_foreground_rgb(attr);

    /* Swap colors. This should put 0x07 in the FG, and (0xD7, 0x5F, 0x67) in
     * the BG */
    attr = swapcolors(attr);

    ASSERT_FG_EQUALS(attr, 0x07);
    ASSERT_INT_EQUALS(color, cell_video_attr_background_rgb(attr), "color");

    END_TEST;
}

BOOL test_swapcolors_does_not_preserve_4th_bit_for_bg_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);
    int color;
    int colorpalette = CK_PALETTE_XT256;

    /* Currently, BG is 0x02 and FG is 0x0F */

    /* Set BG to an RGB value with 0x67 in the blue channel. 0x67 does
     * not ahve the 4th bit set so we should be able to tell if that bit
     * is incorrectly preserved */
    attr = cell_video_attr_set_bg_rgb(attr, 0xD7, 0x5F, 0x67);

    /* Grab a copy of the current background color for later comparison. We
     * should find this value in the foreground. */
    color = cell_video_attr_background_rgb(attr);

    /* Swap the FG and BG. We should find (0xD7, 0x5F, 0x67) in the foreground
     * and 0x07 in the background after this. */
    attr = swapcolors(attr);

    ASSERT_BG_EQUALS(attr, 0x07);
    ASSERT_INT_EQUALS(color, cell_video_attr_foreground_rgb(attr), "color");

    END_TEST;
}

BOOL test_swapcolors_does_not_preserve_4th_bit_for_both_colors_rgb() {
    BOOL result = TRUE;
    cell_video_attr_t attr = cell_video_attr_init_vio_attribute(0x2F);
    int color1, color2;
    int colorpalette = CK_PALETTE_XT256;

    /* 127 has bit 4 set, 103 does not */
    attr = cell_video_attr_set_fg_rgb(attr, 0xD7, 0x5F, 103);
    attr = cell_video_attr_set_bg_rgb(attr, 0xD7, 0x5F, 127);

    color1 = cell_video_attr_foreground_rgb(attr);
    color2 = cell_video_attr_background_rgb(attr);

    attr = swapcolors(attr);

    ASSERT_INT_EQUALS(color1, cell_video_attr_background_rgb(attr), "color1");
    ASSERT_INT_EQUALS(color2, cell_video_attr_foreground_rgb(attr), "color2");

    END_TEST;
}
#endif

/*   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx */

int main() {
    BOOL result = TRUE;
#ifdef CK_COLORS_24BIT
    printf("24-bit color test\n");
#else /* CK_COLORS_24BIT */
# ifdef CK_COLORS_256
    printf("256-color test\n");
# else /* CK_COLORS_256 */
#  ifdef CK_COLORS_DEBUG
    printf("16-color debug test\n");
#  else /* CK_COLORS_DEBUG */
    printf("16-color test\n");
#  endif /* CK_COLORS_DEBUG */
# endif /* CK_COLORS_256 */
#endif /* CK_COLORS_24BIT */

    /* RUN_TEST     - Test for all color build types
     * RUN_EXT_TEST - Test for 256-color and RGB builds only
     * RUN_24_TEST  - Test for 24-bit RGB builds only
     */

    RUN_TEST(test_cell_video_attr_init_vio_attribute);
    RUN_24_TEST(test_cell_video_attr_fg_is_indexed_is_set);
    RUN_24_TEST(test_cell_video_attr_fg_is_indexed_is_unset);
    RUN_24_TEST(test_cell_video_attr_bg_is_indexed_is_set);
    RUN_24_TEST(test_cell_video_attr_bg_is_indexed_is_unset);
    RUN_24_TEST(test_cell_video_attr_bg_is_16_colors);
    RUN_24_TEST(test_cell_video_attr_fg_is_16_colors);
    RUN_24_TEST(test_cell_video_attr_bg_is_not_16_colors);
    RUN_24_TEST(test_cell_video_attr_fg_is_not_16_colors);
    RUN_24_TEST(test_cell_video_attr_bg_is_not_16_colors_because_not_indexed);
    RUN_24_TEST(test_cell_video_attr_fg_is_not_16_colors_because_not_indexed);
    RUN_24_TEST(test_cell_video_attr_set);
    RUN_24_TEST(test_cell_video_attr_fg_to_rgb);
    RUN_24_TEST(test_cell_video_attr_bg_to_rgb);
    RUN_TEST(test_cell_video_attr_foreground);
    RUN_24_TEST(test_cell_video_attr_foreground_ignores_rg);
    RUN_24_TEST(test_cell_video_attr_background_ignores_rg);
    RUN_24_TEST(test_cell_video_attr_foreground_gives_0_for_rgb_value);
    RUN_24_TEST(test_cell_video_attr_background_gives_0_for_rgb_value);
    RUN_TEST(test_cell_video_attr_background);
    RUN_TEST(test_cell_video_attr_set_fg_color);
    RUN_TEST(test_cell_video_attr_set_bg_color);
    RUN_EXT_TEST(test_cell_video_attr_set_fg_color_extended);
    RUN_EXT_TEST(test_cell_video_attr_set_bg_color_extended);
    RUN_TEST(test_cell_video_attr_set_3bit_fg_color);
    RUN_TEST(test_cell_video_attr_set_3bit_bg_color);
    RUN_TEST(test_cell_video_attr_set_3bit_fg_color_with_4bit_value);
    RUN_TEST(test_cell_video_attr_set_3bit_bg_color_with_4bit_value);
    RUN_TEST(test_cell_video_attr_from_vio_attribute);
    RUN_TEST(test_cell_video_attr_set_vio_3bit_colors_preserves_set_4th_bit);
    RUN_TEST(test_cell_video_attr_set_vio_3bit_colors_preserves_cleared_4th_bit);
    RUN_EXT_TEST(test_cell_video_attr_set_vio_3bit_colors_preserves_set_BG_bit4_only);
    RUN_EXT_TEST(test_cell_video_attr_set_vio_3bit_colors_preserves_set_FG_bit4_only);
    RUN_EXT_TEST(test_cell_video_attr_set_vio_3bit_colors_does_notpreserve_bit4);
    RUN_24_TEST(test_cell_video_attr_set_vio_3bit_colors_preserves_set_FG_bit4_bg_rgb);
    RUN_24_TEST(test_cell_video_attr_set_vio_3bit_colors_preserves_set_BG_bit4_fg_rgb);
    RUN_TEST(test_cell_video_attr_to_win32_console);
    RUN_EXT_TEST(test_cell_video_attr_foreground_rgb_xt256);
    RUN_EXT_TEST(test_cell_video_attr_foreground_rgb_xt88);
    RUN_EXT_TEST(test_cell_video_attr_foreground_rgb_xt88_wraps);
    RUN_TEST(test_cell_video_attr_foreground_rgb_aixterm16);
    RUN_TEST(test_cell_video_attr_foreground_rgb_aixterm16_wraps);
    RUN_EXT_TEST(test_cell_video_attr_background_rgb_xt256);
    RUN_EXT_TEST(test_cell_video_attr_background_rgb_xt88);
    RUN_EXT_TEST(test_cell_video_attr_background_rgb_xt88_wraps);
    RUN_TEST(test_cell_video_attr_background_rgb_aixterm16);
    RUN_TEST(test_cell_video_attr_background_rgb_aixterm16_wraps);
    RUN_24_TEST(test_cell_video_attr_foreground_rgb_rgb);
    RUN_24_TEST(test_cell_video_attr_background_rgb_rgb);
    RUN_TEST(test_cell_video_attr_is_null);
    RUN_TEST(test_cell_video_attr_is_not_null_has_fg);
    RUN_TEST(test_cell_video_attr_is_not_null_has_bg);
    RUN_24_TEST(test_cell_video_attr_is_null_rgb);
    RUN_24_TEST(test_cell_video_attr_is_null_rgb_with_garbage);
    RUN_TEST(test_cell_video_attr_is_equal);
    RUN_TEST(test_cell_video_attr_is_not_equal);
    RUN_24_TEST(test_cell_video_attr_is_equal_fg_rgb_bg_indexed);
    RUN_24_TEST(test_cell_video_attr_is_equal_bg_rgb_fg_indexed);
    RUN_24_TEST(test_cell_video_attr_is_equal_bg_rgb_fg_rgb);
    RUN_24_TEST(test_cell_video_attr_is_not_equal_fg_types_differ);
    RUN_24_TEST(test_cell_video_attr_is_not_equal_bg_types_differ);
    RUN_24_TEST(test_cell_video_attr_is_not_equal_bg_rgb_fg_indexed_fg_differs);
    RUN_24_TEST(test_cell_video_attr_is_not_equal_bg_rgb_fg_indexed_bg_differs);
    RUN_TEST(test_cell_video_attr_with_fg_intensity_toggled_clears_set_intensity);
    RUN_TEST(test_cell_video_attr_with_fg_intensity_toggled_sets_cleared_intensity);
    RUN_EXT_TEST(test_cell_video_attr_with_fg_intensity_toggled_leaves_non_4bit_color);
    RUN_24_TEST(test_cell_video_attr_with_fg_intensity_toggled_leaves_rgb_color)
    RUN_TEST(test_cell_video_attr_with_bg_intensity_set_sets_intensity);
    RUN_EXT_TEST(test_cell_video_attr_with_bg_intensity_set_leaves_non_4bit_color);
    RUN_24_TEST(test_cell_video_attr_with_bg_intensity_set_leaves_rgb_color);
    RUN_TEST(test_cell_video_attr_with_fg_intensity_set_sets_intensity);
    RUN_EXT_TEST(test_cell_video_attr_with_fg_intensity_set_leaves_non_4bit_color);
    RUN_24_TEST(test_cell_video_attr_with_fg_intensity_set_leaves_rgb_color);
    RUN_TEST(test_cell_video_attr_with_fg_intensity_unset_unsets_intensity);
    RUN_EXT_TEST(test_cell_video_attr_with_fg_intensity_unset_leaves_non_4bit_color);
    RUN_24_TEST(test_cell_video_attr_with_fg_intensity_unset_leaves_rgb_color);
    RUN_TEST(test_cell_video_attr_with_bg_intensity_unset_unsets_intensity);
    RUN_EXT_TEST(test_cell_video_attr_with_bg_intensity_unset_leaves_non_4bit_color);
    RUN_24_TEST(test_cell_video_attr_with_bg_intensity_unset_leaves_rgb_color);
    RUN_TEST(test_cell_video_attr_set_colors);
    RUN_EXT_TEST(test_cell_video_attr_set_colors_extended);
    RUN_24_TEST(test_cell_video_attr_set_fg_rgb);
    RUN_24_TEST(test_cell_video_attr_set_bg_rgb);
    RUN_TEST(test_swapcolors_3bit);
    RUN_TEST(test_swapcolors_preserves_bg_high);
    RUN_TEST(test_swapcolors_preserves_fg_high);
    RUN_EXT_TEST(test_swapcolors_does_not_preserve_4th_bit_for_fg_color_above_15);
    RUN_EXT_TEST(test_swapcolors_does_not_preserve_4th_bit_for_bg_color_above_15);
    RUN_EXT_TEST(test_swapcolors_does_not_preserve_4th_bit_for_both_colors_above_15);
    RUN_24_TEST(test_rgbsc_swap_4b_3bit);
    RUN_24_TEST(test_rgbsc_swap_4b_preserves_bg_high);
    RUN_24_TEST(test_rgbsc_swap_4b_preserves_fg_high);
    RUN_24_TEST(test_rgbsc_swap_4b_bg_3b);
    RUN_24_TEST(test_rgbsc_swap_4b_bg_4b);
    RUN_24_TEST(test_rgbsc_swap_4b_fg_3b);
    RUN_24_TEST(test_rgbsc_swap_4b_fg_4b);
    RUN_24_TEST(test_rgbsc_swap_all_swaps_all);
    RUN_24_TEST(test_cell_video_attr_is_rgb_is_false_for_indexed_pair);
    RUN_24_TEST(test_cell_video_attr_is_rgb_is_false_for_indexed_fg_rgb_bg);
    RUN_24_TEST(test_cell_video_attr_is_rgb_is_false_for_indexed_bg_rgb_fg);
    RUN_24_TEST(test_cell_video_attr_is_rgb_is_true_for_rgb_fg_rgb_bg);
    RUN_24_TEST(test_cell_video_attr_fgbg_are_16colors_is_true_for_3bit_fgbg);
    RUN_24_TEST(test_cell_video_attr_fgbg_are_16colors_is_false_for_ext_fg);
    RUN_24_TEST(test_cell_video_attr_fgbg_are_16colors_is_false_for_ext_bg);
    RUN_24_TEST(test_cell_video_attr_fgbg_are_16colors_is_false_for_ext_fgbg);
    RUN_24_TEST(test_rgbsc_swap_flags_does_nothing_for_rgb_fgbg);
    RUN_24_TEST(test_rgbsc_swap_flags_does_nothing_for_indexed_fgbg);
    RUN_24_TEST(test_rgbsc_swap_flags_swaps_indexed_fg);
    RUN_24_TEST(test_rgbsc_swap_flags_swaps_indexed_bg);
    RUN_24_TEST(test_swapcolors_does_not_preserve_4th_bit_for_fg_rgb);
    RUN_24_TEST(test_swapcolors_does_not_preserve_4th_bit_for_bg_rgb);
    RUN_24_TEST(test_swapcolors_does_not_preserve_4th_bit_for_both_colors_rgb);

/*  These are the only colour macros not currently tested. Not sure its worth
    testing them though - they just look a string up in an array.
     cell_video_attr_background_color_name(value) (colors[cell_video_attr_background(value)])
     cell_video_attr_foreground_color_name(value) (colors[cell_video_attr_foreground(value)])
*/
    if (result) return 0;

    printf("\nOne or more tests failed!\n\n");

    return 1;
}