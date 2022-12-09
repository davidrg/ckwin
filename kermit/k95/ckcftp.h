/* Prototypes for static functions defined in ckcftp.c */
#ifdef CK_ANSIC

static VOID bytswap( int *, int * );
static VOID cancel_remote( int );
static VOID changetype( int, int );
static VOID dbtime( char *, struct tm * );
static VOID ftscreen( int, char, CK_OFF_T, char * );
static char * ftp_hookup( char *, int, int );
static char * radix_error( int );
static char * shopl( int );
static char * strval( char *, char * );
static int check_data_connection( int, int );
static int chkmodtime( char *, char *, int );
static int dataconn( char * );
static int doftpcwd( char *, int );
static int doftpdir( int );
static int doftpxmkd( char *, int );
static int ftp_login( char * );
static int ftp_rename( char *, char * );
static int ftp_umask( char * );
static int ftp_user( char *, char *, char * );
static int ftpcmd( char *, char *, int, int, int );
static int fts_cpl( int );
static int fts_dpl( int );
static int getfile( char *, char *, int, int, char *, int, int, int );
static int getreply( int, int, int, int, int );
static int ispathsep( int );
static int looping_read(int, register char *, register int );
static int looping_write( int, register CONST char *, int );
static int openftp( char *, int );
static int putfile( int, char *, char *, int, int, char *, char *, char *, int, int, int, int, int, int, int );
static int recvrequest(char *, char *, char *, char *, int, int, char *, int, int, int );
static int secure_flush( int );
static int secure_getbyte( int, int );
static int secure_read( int fd, char *, int );
static int sendrequest( char *, char *, char *, int, int, int, int );
static int syncdir( char *, int );

static int tmcompare( struct tm *, struct tm * );
static int xlatec( int, int, int, int );
static sigtype cancelrecv( int );
static sigtype cancelsend( int );
static sigtype cmdcancel( int );

#endif  /* CK_ANSIC */
