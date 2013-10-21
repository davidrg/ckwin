#define CHAR unsigned char

static char rxdigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char *
#ifdef CK_ANSIC
ckctox(CHAR c, int flag)
#else
ckctox(c, flag) CHAR c; int flag;
#endif
/* ckctox */ {
    static char buf[48];
    static int current = 0;
    int x;
    char h;
    if (current > 45)
      current = 0;
    x = (c >> 4) & 0x0f;
    h = rxdigits[x];
    if (!flag && isupper(rxdigits[x]))
      h = tolower(rxdigits[x]);
    buf[current++] = h;
    x = c & 0x0f;
    h = rxdigits[x];
    if (!flag && isupper(rxdigits[x]))
      h = tolower(rxdigits[x]);
    buf[current++] = h;
    buf[current++] = '\0';
    return((char *)(buf + current - 3));
}


main() {
    int i;
    CHAR c;
    for (i = 0; i < 256; i++) {
        c = i & 0xff;
        printf("%s",ckctox(c,0));
        printf("%s",ckctox(c,1));
    }
}
