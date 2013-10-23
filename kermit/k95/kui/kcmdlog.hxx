#ifndef kcmdlog_hxx_included
#define kcmdlog_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kcmdlog.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: command processor for logfile property page.

========================================================================*/

#include "kcmdproc.hxx"

struct _logFileStruct;
class KCmdLogfiles : public KCmdProc
{
public:
    KCmdLogfiles( K_GLOBAL* );
    ~KCmdLogfiles();

    Bool doInitDialog( HWND );
    Bool doCommand( HWND hPar, WORD code, WORD idCtrl, HWND hCtrl );
    Bool killActive();

protected:
    void browseFile( int );

private:
    _logFileStruct** files;
};


#endif
