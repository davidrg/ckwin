#include "kmenu.hxx"
#include "kwin.hxx"
#include "ksysmets.hxx"
#include "afxres.h"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KMenu::KMenu( int mid )
    : hMenu( 0 )
    , menuid( mid )
    , parent( 0 )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KMenu::~KMenu()
{
    SetMenu( parent->hwnd(), NULL );
    DestroyMenu( hMenu );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KMenu::createMenu( HINSTANCE hinst, KWin* p )
{
    parent = p;
    hMenu = LoadMenu( hinst, MAKEINTRESOURCE( menuid ) );
    SetMenu( parent->hwnd(), hMenu );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KMenu::getHeight( int& h )
{
    int defheight = kglob->sysMets->menuHeight();
    int captionheight = kglob->sysMets->captionHeight();
    int parh, parw;
    parent->getSize( parh, parw );

    RECT rect;
    GetClientRect( parent->hwnd(), &rect );
    int clih = rect.bottom - rect.top;

    h = parh - clih - captionheight;
}

void KMenu::setCheck( UINT id )
{
    UINT stat = GetMenuState( hMenu, id, MF_BYCOMMAND );
    UINT newstat;    
    if ( !(stat & MF_CHECKED) ) {
        newstat = MF_CHECKED | MF_BYCOMMAND;
        CheckMenuItem( hMenu, id, newstat );
    }
}


void KMenu::unsetCheck( UINT id )
{
    UINT stat = GetMenuState( hMenu, id, MF_BYCOMMAND );
    UINT newstat;    
    if( stat & MF_CHECKED ) {
        newstat = MF_UNCHECKED | MF_BYCOMMAND;
        CheckMenuItem( hMenu, id, newstat );
    }
}

/*------------------------------------------------------------------------
    return if the item was checked already
------------------------------------------------------------------------*/
bool KMenu::toggleCheck( UINT id )
{
    UINT stat = GetMenuState( hMenu, id, MF_BYCOMMAND );
    bool checked;
    UINT newstat;    
    if( stat & MF_CHECKED ) {
        checked = false;
        newstat = MF_UNCHECKED | MF_BYCOMMAND;
    }
    else {
        checked = true;
        newstat = MF_CHECKED | MF_BYCOMMAND;
    }

    CheckMenuItem( hMenu, id, newstat );

    return checked;
}

void KMenu::enable( UINT id, bool on )
{
    EnableMenuItem( hMenu, id, on ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED );
}

void KMenu::remove( UINT id )
{
    RemoveMenu( hMenu, id, MF_BYCOMMAND );
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KMenu::setCopyPaste( bool textselected, bool clipboarddata )
{
    UINT flags = MF_BYCOMMAND;
    flags |= textselected ? MF_ENABLED : MF_GRAYED;
    EnableMenuItem( hMenu, ID_EDIT_COPY, flags );

    flags = MF_BYCOMMAND;
    flags |= clipboarddata ? MF_ENABLED : MF_GRAYED;
    EnableMenuItem( hMenu, ID_EDIT_PASTE, flags );
}
