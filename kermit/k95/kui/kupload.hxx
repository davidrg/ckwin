#ifndef kupload_hxx_included
#define kupload_hxx_included
/*========================================================================
    Copyright (c) 1995, Trustees of Columbia University 
    in the City of New York.  All rights reserved.
-----------------------------------------------------------------------

    File.......: kupload.hxx
    Author.....: Thomas Wu (thomaswu@columbia.edu)
    Created....: December 27, 1995

    Description: upload dialog box.

========================================================================*/

#include "kwin.hxx"

class KFileName;
class KUpload : public KWin
{
public:
    KUpload( K_GLOBAL*, KFileName* );
    ~KUpload();

    Bool showUpload();
    void createWin( KWin* par );
    Bool message( HWND hwnd, UINT msg, UINT wParam, LONG lParam );

    void dblClickListView( int x, int y );
    WNDPROC getListViewProc() { return listViewProc; }

protected:
    void initDialog( HWND );
    void initDone();

    void initListView();
    void notifyListView( LONG lParam );

    void fileSelect();
    void removeFile( Bool bAll = FALSE );
    void addFileToList( char* );
    void fileUpload();
    
    void setFileStats( char*, int filestat );
    void showPropertyPage( int );

	void setProtocol();

private:
    HWND hwndpar;
    HWND hlist;
    HWND hcombo;
    HWND hfilecount;
    HWND hbytecount;
    HWND hupload;
    WNDPROC listViewProc;

    int numFiles;
    double totByteCount;

    KFileName* kfilename;
};

#endif
