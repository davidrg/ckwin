#include "kcolor.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KColor::KColor( K_GLOBAL* kg, KWin* par )
    : KWin( kg )
{
    parent = par;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KColor::~KColor()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KColor::show( Bool bVisible )
{
    CHOOSECOLOR chc;
    memset( &chc, '\0', sizeof(CHOOSECOLOR) );

    DWORD dwColor = RGB( 0,0,0 );
    DWORD dwCustColors[16];
    for( int i = 0; i < 16; i++ )
        dwCustColors[i] = RGB( 255,255,255 );

    chc.lStructSize = sizeof(CHOOSECOLOR);
    chc.hwndOwner = parent->hwnd();
    chc.rgbResult = dwColor;
    chc.lpCustColors = (LPDWORD) dwCustColors;
    chc.Flags = CC_RGBINIT;

//    chc.hInstance;
//   LPARAM       lCustData;
//   LPCCHOOKPROC lpfnHook;
//   LPCWSTR      lpTemplateName;

    ChooseColor( &chc );
}
    
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KColor::message( HWND hwnd, UINT msg, UINT wParam, LONG lParam )
{
    return FALSE;
}

