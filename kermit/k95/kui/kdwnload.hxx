#ifndef kdwnload_hxx_included
#define kdwnload_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kdwnload.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: download dialog box.

========================================================================*/

#include "kwin.hxx"

class KDownLoad : public KWin
{
public:
    KDownLoad( K_GLOBAL* );
    ~KDownLoad();

    void createWin( KWin* par );
    void show( Bool bVisible = TRUE );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );
    Bool saveAsMsg( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    WNDPROC getOldProc() { return oldSaveAsProc; }

    Bool  wasCancelled() { return (success == FALSE); }

    DWORD getErrorCode() { return errorCode; }

    void   setInitialDirectory(char * dir);
    void   setInitialFileName(char * file);
    void   setTitle(char * title);
    char * getSelectedFileName(void) {
        return ( success == TRUE ? szFile : NULL );

    }

protected:
    void initDialog( HWND );

private:
    HWND hwndpar;
    HWND hdownload;
    WNDPROC oldSaveAsProc;

    int downloadID;
    int optionID;

    OPENFILENAME OpenFileName;
    DWORD errorCode;
    DWORD success;
    char szFilter[MAX_PATH];
    char szDir[MAX_PATH];
    char szFile[MAX_PATH];
    char szTitle[MAX_PATH];
};

#endif
