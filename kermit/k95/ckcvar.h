typedef
struct _cktype {
    unsigned char function:1;
    unsigned char pointer :1;
    unsigned char type    :4;
} CKTYPE, * PCKTYPE ;

typedef
union _ckfld {
           char     schar ;
    unsigned char   uchar ;
           short    sshort ;
    unsigned short  ushort ;
           int      sint ;
    unsigned int    uint ;
           long     slong ;
    unsigned long   ulong ;
           char *   sstr  ;
    unsigned char * ustr ;

           char     *pschar ;
    unsigned char   *puchar ;
           short    *psshort ;
    unsigned short  *pushort ;
           int      *psint ;
    unsigned int    *puint ;
           long     *pslong ;
    unsigned long   *pulong ;
           char *   *psstr  ;
    unsigned char * *pustr ;

           char      (*scharf)() ;
    unsigned char    (*ucharf)() ;
           short     (*sshortf)() ;
    unsigned short   (*ushortf)() ;
           int       (*sintf)() ;
    unsigned int     (*uintf)() ;
           long      (*slongf)() ;
    unsigned long    (*ulongf)() ;
           char *    (*sstrf)()  ;
    unsigned char *  (*ustrf)() ;

           char     *(*pscharf)() ;
    unsigned char   *(*pucharf)() ;
           short    *(*psshortf)() ;
    unsigned short  *(*pushortf)() ;
           int      *(*psintf)() ;
    unsigned int    *(*puintf)() ;
           long     *(*pslongf)() ;
    unsigned long   *(*pulongf)() ;
           char *   *(*psstrf)()  ;
    unsigned char * *(*pustrf)() ;
} CKFLD, * PCKFLD ;

typedef struct _ckvar CKVAR, * PCKVAR ;
#ifdef ANSI
typedef unsigned char * (*SETSTRF)(PCKVAR) ;
#else
typedef unsigned char * (*SETSTRF)() ;
#endif

typedef
struct _ckvar {
    CKTYPE  def_t ;
    CKFLD   def;
    CKTYPE  cur_t;
    CKFLD   cur;
    SETSTRF setstrf;
} ;

typedef struct _ckvargrp CKVARGRP, * PCKVARGRP ;
struct _ckvargrp {
    int varcount ;
#ifdef ANSI
    int (*initf)(PCKVARGRP);
   #else
    int (*initf);
#endif
    CKVAR var[64];
} ;

typedef struct _ckvarall CKVARALL, * PCKVARALL ;
struct _ckvarall {
    CKVARGRP term ;
    CKVARGRP file ;
    CKVARGRP misc ;
    CKVARGRP proto ;
};
