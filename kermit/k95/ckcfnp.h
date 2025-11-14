/*
  ckcfnp.h, new to C-Kermit 10.0 as of 23 March 2023.

    Frank da Cruz
    Most recent update: 5 February 2024 (remove locate_srv_dns())

  Prototypes for functions that previously were not prototyped.
  Used only for ANSI-C builds in which __STDC__ is defined.
  ckcfnp.h must be included in each C source file as the LAST #include.
  Prototype format: return-type function-name ( arguments );
  If no arguments, "void" appears in the parentheses, e.g. "( void )".
  If one argument, its type appears, e.g. "( int )".
  If more than one argument, the type of each, separated by commas.
  The argument names are omitted; K&R second edition says:
  "Parameter names need not agree (and) are optional".

  Note: Functions that *were* already prototyped are scattered
  throughout the ck*.c and ck*.h files using a macro _PROTOTYP; I'm
  not changing those, they've been working just fine on both K&R and
  ANSI builds for decades, they coexist with this file, and we don't
  "fix what ain't broke".
*/
#ifndef CKCFNP_H
#define CKCFNP_H

#ifndef NOANSI
#ifdef __STDC__
#ifdef CK_ANSIC
/*
  #include ckcker.h was added 27 April 2023 because certain builds (like
  "linux+ssl") were failing.  ckcker.h defines data types and other symbols
  referenced in this file.  It should be included by every module before
  including this one.  But just in case there's an omission, including it here
  too does no harm because the ck*.h files protect themselves against multiple
  inclusion.  ckcdeb.h added 3 may 2023, because the mainname definition was
  moved from here (where non-ansi builds would never see it) to ckcdeb.h.
  ckuusr.h added 12 May 2023 for MINIX / ckucon.c / struct m[x,xx]tab.
*/
#include "ckcdeb.h"
#include "ckcker.h"
#include "ckucmd.h"
#include "ckuusr.h"

/* Prototype for main()/Main() */
MAINTYPE MAINNAME( int argc, char ** argv ); 

/* PROTOTYPES ADDED 6 May 2023... */

#ifdef CK_AUTHENTICATION
int show_auth( int );
#endif /* CK_AUTHENTICATION */

/* PROTOTYPES ADDED 11-14 April 2023... */

int hasnopath ( char * );
VOID puschcmd ( char * );
VOID newerrmsg ( char * );
char * getdm ( int );

/* PROTOTYPES ADDED 26 March 2023... */

void docmdline( void * );
void failcmdline( void * );
void failtakeini( void * );
void dotakeini( void * );
void failtakeini( void * );
void doicp( void * );
void failicp( void * );
void docmdfile( void * );
void failcmdfile( void * );
char * getsysid( char * );
int getsysix( char * );
int isabsolute( char * path );
int is_a_tty( int );
void initxlist ( void );
void initflow ( void );
void makever ( void );
void dourl ( void );
int getiobs ( void );

#ifndef NOSPL
/* struct mtab [] exists only if NOSPL isn't defined */
int mlook( struct mtab [], char *, int );
int mxlook( struct mtab [], char *, int );
int mxxlook ( struct mtab [], char *, int );
#endif /* NOSPL */

int savhistory (char *, int);
char * ckltoa ( long );
char * ckultoa ( unsigned long );
char * ckltox ( long );
char * ckitoa ( int );
char * ckfstoa ( CK_OFF_T );
char * ckitox( int );
CK_OFF_T ckatofs(char * s);
char * ckctoa(char c);
char * ckctox(CHAR c, int flag);
int dquote( char * fn, int len, int flag );
void makestr(char **p, const char *s);
void xmakestr(char **p, const char *s);
CKFLOAT ckround( CKFLOAT, int, char *, int );
int chknum( char * );
int rdigits( char * s );
char * parnam( char c );
char * hhmmss ( long x );
char * ulongtohex( unsigned long z, int n );
int ckhexbytetoint( char * s );
struct stringarray * cksplit(int fc,int n1,char * s1,char * s2,char * s3,
                             int n2,int n3,int n4,int n5);
 
/* PROTOTYPES ADDED 24 March 2023... */

CK_OFF_T z_count( int, int );
CK_OFF_T z_getline( int );
CK_OFF_T z_getpos( int );
CK_OFF_T zchki( char * );
CK_OFF_T zgetfs( char * );
SIGTYP timerh( int );
SIGTYP xtimerh( int );
char * bldlen( char *, char *);
char * brstrip( char * );
char * chk_ac( int, char [] );
char * ckaddr2name( char * );
char * ckcvtdate( char *, int );
char * ckferror( int );
char * ckname2addr( char * );
char * ckradix( char *, int, int );
char * ckstrpbrk( char *, char * );
char * ckstrstr( char *, char * );
char * ckuitoa( unsigned int );
char * cmcvtdate( char *, int );
char * cmdelta( int,int,int,int,int,int,int,int,int,int,int,int,int );
char * cmdiffdate( char *, char * );
char * cmgetcmd( char * );
char * dbchr( int );
char * dosexp( char * );
char * evala( char * );
char * evalx( char * );
char * fpformat( CKFLOAT, int, int );
char * getbasename( char * );
char * getdnum( int );
char * getsysid( char * );
char * gfmode( int, int );
char * gskreason( int );
char * jzdate( char * );
char * locale_dayname( int, int );
char * locale_monthname( int, int );
char * nvlook( char * );
char * pncvt( char * );
char * rlookup( struct keytab [], int, int );
char * sgetsave( char * );
char * showoff( int );
char * showooa( int );
char * showstring( char * );
char * shuffledate( char *, int );
char * tilde_expand( char * );
char * z_getname( int );
char * zfcdat( char * );
char * zgperm( char * );
char * ziperm( char * );
char * zjdate( char * );
char * zlocaltime( char * );
int rlog_ctrl( unsigned char *, int );
int addlocal( char * );
int addmac( char *, char * );
int addmmac( char *, char * [] );
int adebu( char *, struct zattr * );
int adjpkl( int, int, int );
int arraybounds( char *, int *, int * );
int arraynam( char *, int *, int * );
int b64tob8( char *, int, char *, int );
int b8tob64( char *, int, char *, int );
int bkupnum( char *, int * );
int boolexp( int );
int canned( CHAR * );
int chk1( register CHAR *, register int );
int chkarray( int, int );
int chkfn( int );
int chkspkt( char * );
int chktimo( int, int );
int chktok( char * );
int chkvar( char * );
int chkwin( int, int, int );
int cisalphanum( CHAR );
int ck_curpos( int, int );
int ck_linger( int, int, int );
int ckcgetc( int );
int ckcputc( int );
int ckdial( char *, int, int, int, int );
int ckindex( char *, char *, int, int, int );
int cklower( char * );
int ckmatch( char *, char *, int, int );
int ckmkdir( int, char *, char **, int, int );
int ckopenx( struct zattr * );
int ckrchar( char * );
int ckstrcmp( char *, char *, register int, register int );
int ckstrpre( char *, char * );
int ckupper( char * );
int clsconnx( int );
int clskconnx( int );
int clsof( int );
int cmdate( char *, char *, char **, int, xx_strp );
int cmdir( char *, char *, char **, xx_strp );
int cmdirp( char *, char *, char **xp, char *, xx_strp );
int cmdlinput( int );
int cmdsquo( int );
int cmfdb( struct FDB * );
int cmfld( char *, char *, char **, xx_strp );
int cmifi( char *, char *, char **, int *, xx_strp f);
int cmifi2( char *, char *, char **, int *, int, char *, xx_strp, int );
int cmifip( char *, char *, char **, int *, int, char *, xx_strp );
int cmiofi( char *, char *, char **, int *, xx_strp );
int cmkey(  struct keytab [], int, char *, char *, xx_strp );
int cmkey2( struct keytab [], int, char *, char *, char *, xx_strp, int );
int cmkeyx( struct keytab [], int, char *, char *, xx_strp );
int cmnum( char *, char *, int, int *, xx_strp );
int cmnumw( char *, char *, int, CK_OFF_T *, xx_strp );
int cmofi( char *, char *, char **, xx_strp );
int cmrini( int );
int cmswi( struct keytab [], int, char *, char *, xx_strp );
int cmtxt( char *, char *, char **, xx_strp );
int coninc( int  );
int conol( char * );
int conola( char * [] );
int conoll( char * );
int conxo( int, char * );
int cs_is_nrc( int );
int cs_is_std( int );
int cs_size( int );
int cwd( char * );
int cx_fail( int, char * );
int debopn( char *, int );
int delmac( char *, int );
int delta2sec( char *, long * );
int diaopn( char *, int, int fc );
int do_open( char * );
int do_pty( int *, char *, int );
int doask( int );
int docd( int );
int doclslog( int );
int docmd( int cx );
int doconect( int, int );
int dodef( int );
int dodial( int );
int dodir( int );
int dodo( int, char *, int );
int doenable( int, int );
int dofile( int );
int doftpget( int, int );
int doftprmt( int, int );
int doftptyp( int );
int dogoto( char *, int );
int dogta( int );
int dohfunc( int );
int dohlp( int );
int dohrmt( int );
int dohset( int );
int doif( int );
int doincr( int );
int doinput( int, char * [], int [], int, int );
int dolog( int );
int dologin( char * );
int domac( char *, char *, int );
int dontroute( int, int );
int dooutput( char *, int );
int dopaus( int );
int doprm( int, int );
int doputenv( char *, char * );
int doreinp( int, char *, int );
int doreturn( char * );
int dormt( int );
int dosave( int );
int doshift( int );
int doshow( int );
int dostat( int );
int dotake( char * );
int dotype( char *, int,int,int, char *, int, char *, int,int, char *, int );
int doundef( int );
int doxarg( char **, int );
int doxconn( int );
int doxget( int );
int doxsend( int );
int dquote( char *, int, int );
int filhelp( int, char *, char *, int, int );
int fnparse( char * );
int freeslot( int );
int ftpopen( char *, char *, int );
int gattr( CHAR *, struct zattr * );
int getlocalipaddrs( char *, int, int );
int getncm( char *, int );
int getnct( char *, int, FILE *, int );
int getok( int, int );
int getpkt( int, int );
int getptyslave( int *, int );
int getrtt( int, int );
int getsysix( char * );
int gettcs( int, int );
int gettoken( FILE * );
int getyesno( char *, int );
int gnirts( char *, char *, int );
#ifndef OS2
int hash( char * );     /* Conflicts with SRP support on Windows and OS/2 */
#endif /* OS2 */
int hmsg( char * );
int hmsga( char * [] );
int http_inc( int );
int http_tol( CHAR *, int );
int hupok( int );
int incvar( char *, CK_OFF_T, int );
int inibufs( int, int );
int initattr( struct zattr * );
int initslot( int );
int is_a_tty( int );
int isabsolute( char * );
int isalink( char * );
int isalphanum( char * );
int isdir( char *s );
int isfloat( char *, int );
int isin( char *, int c );
int isinternalmacro( int );
int ispattern( char * );
int iswild( char * );
int isword( int );
int keepalive( int, int );
int litcmd( char **, char **dest, int );
int lkup( char * );
int locate_txt_rr( char *, char *, char ** );
int lookup( struct keytab [], char *, int, int * );
int makebuf( int, int, CHAR [], struct pktinfo * );
int matchname( char *, int, int );
int mkrbuf( int );
int mksbuf( int );
int msleep( int );
int nack( int );
int netinc( int );
int netopen( char *, int *, int );
int nettol( CHAR *, int );
int netxin( int, CHAR * );
int nlookup( struct keytab [], char *, int, int * );
int no_delay( int, int );
int nonalphanum( char * );
int opena( char *f, struct zattr * );
int openc( int, char * );
int openi( char * );
int openo( char *, struct zattr *, struct filinfo * );
int opent( struct zattr * );
int parser( int );
int parsevar( char *, int *, int * );
int pktopn( char *, int );
int plogin( int );
int priv_opn( char *, int );
int prtopt( int *, char * );
int psuspend( int );
int pusharray( int, int );
int rcvfil( char * );
int rdigits( char * );
int readpass( char *, char *, int );
int readtext( char *, char *, int );
int recvbuf( int, int );
int remset( char * );
int remtxt( char ** );
int reof( char *, struct zattr * );
int savhistory( char *, int );
int savkeys( char *, int );
int scanfile( char *, int *, int );
int scanstring( char * );
int sendbuf( int, int );
int seof( int );
int sesopn( char *, int );
int setat( int );
int setcc( char *, int * );
int setfil( int );
int setlin( int, int, int );
int setnum( int *, int, int, int );
int seton( int * );
int setonaut( int * );
int setsr( int, int );
int sfile( int );
int shoesc( int );
int shoftp( int );
int shomac( char *, char * );
int shotcp( int );
int shotel( int );
int shotopt( int );
int shxpand( char *, char * [], int );
int slotdir( char *, char *);
int slotstate( int, char *, char *, char * );
int snddel( char * );
int snddir( char *  );
int sndstring( char * );
int sndtype( char * );
int srinit( int, int, int );
int sxeof( int );
int syscmd( char *, char * );
int szeof( CHAR * );
int tcpsrv_open( char *, int *, int, int );
int tinit( int );
int tn_siks( int );
int tn_sopt( int, int );
int tn_ssbopt( int, int sub, CHAR *, int );
int traopn( char *, int );
int ttinc( int );
int ttol( CHAR *, int );
int ttopen( char *, int *, int, int );
int ttptycmd( char * );
int ttpushback( CHAR *, int );
int ttruncmd( char * );
int ttscarr( int );
int ttsetflow( int );
int ttsspd( int );
int ttwait( int, int );
int ttxin( int, CHAR * );
int untabify( char *, char *, int );
int updslot( int );
#ifndef NOURL
int urlparse( char *, struct urldata * );
#endif /* NOURL */
int varval( char *, CK_OFF_T * );
int window( int );
int xarray( char * );
int xdelmac( int );
int xlate( char *, char *, int, int );
int xlookup( struct keytab [], char *, int, int * );
int xxdormt( int );
int yystring( char *, char ** );
int z_close( int );
int z_flush( int );
int z_getfnum( int );
int z_getmode( int );
int z_in( int, char *, int, int, int );
int z_open( char *, int );
int z_out( int, char *, int, int);
int zchdir( char * );
int zchin( int, int * );
int zchko( char * );
int zclose( int );
int zclosf( int );
int zcmpfn( char *, char * );
int zcopy( char *, char * );
int zdelet( char * );
int zmkdir( char * );
int znext( char * );
int zopeni( int, char * );
int zopeno( int, char *, struct zattr *, struct filinfo * );
int zprint( char *, char * );
int zrename( char *, char * );
int zrmdir( char * );
int zsattr( struct zattr * );
int zsetfil( int, int );
int zsetperm( char *, int );
int zshcmd( char * );
int zsinl( int, char *, int );
int zsout( int, char * );
int zsoutl( int, char * );
int zsoutx( int, char *, int );
int zstime( char *, struct zattr *, int );
int zsyscmd( char * );
int zvpass( char * );
int zvuser( char * );
int zxcmd( int, char * );
int zxin( int, char *, int );
int zzstring( char *, char **, int * );
#ifndef NOLOCAL
long dologshow( int );
#endif  /* NOLOCAL */
long hextoulong( char *, int );
long mjd( char * );
long pty_cleanup( char *, int, int );
long pty_initialize_slave ( int );
long pty_open_ctty( char *, int *, int );
long pty_open_slave( char *, int *, int );
long tod2sec( char * );
struct passwd * sgetpwnam( char * );
struct stringarray * cksplit( int,int,char *, char *,char *, int,int,int,int );
struct zfnfp * zfnqfp( char *, int, char * );
time_t zstrdt( char *, int );
unsigned int chk2( register CHAR *, register int );
unsigned int chk3( register CHAR *, register int );
void addaction( int, int, int );
void addcmd( char * );
void ckhost( char *, int );
void cmini( int );
void cmsetp( char * );
void conbgt( int );
void copy_termbuf( char *, int );
void copyact( FILE *, FILE *, int );
void copyrest( FILE *, FILE * );
void doclean( int );
void doexit( int, int );
void doftpglobaltype( int );
void enter( char *, int );
void epilogue( FILE * );
void ermsg( char * );
void evalmacroarg( char ** );
void exec_cmd( char * );
void fatal( char * );
void fatal2( char *, char * );
void freelocal( int );
void freerpkt( int );
#ifndef NOLOCAL
void fxdinit( int );
#endif /* NOLOCAL */
void init_termbuf( int );
void initial( FILE *, FILE * );
void initmdm( int );
void initproto( int, char *, char *, char *, char *, char *, char *, char * );
void initxlate( int, int );
void kwdhelp( struct keytab [], int, char *, char *, char *, int, int );
void logstr( char *, int );
void lset( char *, char *, int, int );
void makelist( char *, char * [], int );
void nettout( int );
void nzltor( char *, char *, int, int, int );
void nzrtol( char *, char *, int, int, int );
void prescan( int );
void prolog( FILE * );
void prompt( xx_strp );
void pty_make_raw( int );
void rdcmnt( FILE * );
void rdebu( CHAR *, int );
void rdstates( FILE *, FILE * );
void rdword( FILE *, char * );
void rinit( CHAR * );
void rset( char *, char *, int, int );
void sdebu( int );
void setautodl( int, int );
void setautolocus( int );
void setcmask( int );
void setdebses( int );
void setexitwarn( int );
void setlclcharset( int );
void setlocus( int, int );
void setprefix( int );
void setremcharset( int, int );
void setseslog( int );
void setxlatype( int, int );
void sh_sort( char **, char **, int, int, int, int );
void shods( char * );
#ifdef OS2
void shokeycode( int, int );
#else
void shokeycode( int );
#endif /* OS2 */
void shostrdef( CHAR * );
void shotcs( int, int );
void tn_debug( char * );
void untab( char * );
void warray( FILE *, char *, int [], int, char * );
void writetbl( FILE * );
void xwords( char *, int, char * [], int );
void zltor( char *, char * );
void znewn( char *, char **s );
void zrtol( char *, char * );
void zstrip( char *, char ** );
void ztime( char ** );

#endif /* CK_ANSIC */
#endif /* __STDC__ */
#endif /* NOANSI   */
#endif /* CKCFNP_H */
