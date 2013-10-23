#include "khwndset.hxx"
#include "karray.hxx"

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KHwndSet::KHwndSet()
    : array( new KArray() )
    , arraycount( 0 )
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KHwndSet::~KHwndSet()
{
    KWIN_HWND* kwinh;
    for( long i = 0; i < arraycount; i++ )
    {
        kwinh = (KWIN_HWND*) array->get( i );
        delete kwinh;
    }
    delete array;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KWin* KHwndSet::find( HWND h )
{
    KWIN_HWND* kwinh = locate( h );
    if( kwinh )
        return kwinh->win;
    return 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KHwndSet::add( KWin* k, HWND h )
{
    KWIN_HWND* kwinh = locate( h );
    if( !kwinh )
    {
        kwinh = new KWIN_HWND;
        kwinh->win = k;
        kwinh->hwnd = h;
        array->put( (void*)kwinh, arraycount++ );
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KHwndSet::remove( KWin* k )
{
    KWIN_HWND* kwinh;
    for( long i = 0; i < arraycount; i++ )
    {
        kwinh = (KWIN_HWND*)array->get( i );
        if( kwinh->win == k )
        {
            delete kwinh;
            array->remove( i );
            arraycount--;
            return;
        }
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void KHwndSet::remove( HWND h )
{
    KWIN_HWND* kwinh;
    for( long i = 0; i < arraycount; i++ )
    {
        kwinh = (KWIN_HWND*)array->get( i );
        if( kwinh->hwnd == h )
        {
            delete kwinh;
            array->remove( i );
            arraycount--;
            return;
        }
    }
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KWIN_HWND* KHwndSet::locate( KWin* k )
{
    KWIN_HWND* kwinh;
    for( long i = 0; i < arraycount; i++ )
    {
        kwinh = (KWIN_HWND*)array->get( i );
        if( kwinh->win == k )
            return kwinh;
    }
    return 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KWIN_HWND* KHwndSet::locate( HWND h )
{
    KWIN_HWND* kwinh;
    for( long i = 0; i < arraycount; i++ )
    {
        kwinh = (KWIN_HWND*)array->get( i );
        if( kwinh->hwnd == h )
            return kwinh;
    }
    return 0;
}
