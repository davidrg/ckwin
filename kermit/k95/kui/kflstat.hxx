#ifndef kflstat_hxx_included
#define kflstat_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kflstat.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: file transfer status dialog box.

========================================================================*/

#include "kwin.hxx"

class KProgress;
class KArray;
class KFileStatus : public KWin
{
public:
    KFileStatus( K_GLOBAL* );
    ~KFileStatus();

    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );
    void show( Bool bVisible = TRUE );
    void close();

    void setProperty( UINT propid, LONG value );

private:
    KProgress* progress;

    HWND hcurrentdir;
    HWND hsending;
    HWND hfiletype;
    HWND hfilesize;
    HWND htimeleft;
    HWND htransferrate;
    HWND hwindowslots;
    HWND hgroupmsg;
    HWND heditmsg;
    HWND hspinmsg;
    HWND hgrouperr;
    HWND hediterr;
    HWND hspinerr;
    HWND hpackettype;
    HWND hpacketcount;
    HWND hpacketlength;
    HWND hpacketretry;
    HWND hbytestrans;

    long bytesTrans;
    long prevPacketLength;

    KArray* messages;
    long numMessages;
    KArray* errors;
    long numErrors;

};

extern KFileStatus* filestatus;

#endif
