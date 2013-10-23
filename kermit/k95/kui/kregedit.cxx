#include "kregedit.hxx"

char* settingLabels[ numSettings ] = {
    "Terminal Left", "Terminal Top",
    "Command Left" , "Command Top",
    "CServer Left" , "CServer Top",
    "Font Facename", "Font Height",
    "Font Width"
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KRegistryEdit::KRegistryEdit()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
KRegistryEdit::~KRegistryEdit()
{
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KRegistryEdit::getCoordinates( K_GLOBAL* kglob )
{
    Bool done = TRUE;
    HKEY hkey = openSettings();

    DWORD type = REG_DWORD;
    DWORD size = sizeof(DWORD);
    DWORD data;

    LONG ret = -1L;
    if( hkey ) {
        ret = ERROR_SUCCESS;
        for( int i = Terminal_Left; i <= Terminal_Top && ret == ERROR_SUCCESS; i++ )
        {
            ret = RegQueryValueEx( hkey, settingLabels[i]
                        , 0, &type, (LPBYTE) &data, &size );

            if( ret == ERROR_SUCCESS )
                kglob->settings[i] = data;
        }
    }
    
    if( ret != ERROR_SUCCESS )  // an error occurred or first time through
    {
        // set the terminal coordinates for top left corner
        //
        kglob->settings[Terminal_Left] = 0L;
        kglob->settings[Terminal_Top]  = 0L;

        kglob->settings[Command_Left] = (DWORD)CW_USEDEFAULT;
        kglob->settings[Command_Top]  = 0L;
        kglob->settings[CServer_Left] = (DWORD)CW_USEDEFAULT;
        kglob->settings[CServer_Top]  = 0L;
    }

    if( hkey )
        RegCloseKey( hkey );
    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KRegistryEdit::setCoordinates( K_GLOBAL* kglob )
{
    HKEY hkey = openSettings();
    if( !hkey )
        return FALSE;

    DWORD data;
    LONG ret = ERROR_SUCCESS;
    for( int i = Terminal_Left; i <= Terminal_Top && ret == ERROR_SUCCESS; i++ )
    {
        data = kglob->settings[i];
        ret = RegSetValueEx( hkey, settingLabels[i]
                    , 0, REG_DWORD, (BYTE*) &data, sizeof(DWORD) );
    }

    RegCloseKey( hkey );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KRegistryEdit::getFontinfo( K_GLOBAL* kglob )
{
    Bool done = TRUE;
    HKEY hkey = openSettings();

    DWORD type = REG_SZ;
    DWORD size = sizeof(kglob->faceName);
    CHAR szData[256];
    DWORD dwData;

    LONG ret = -1L;
    if( hkey ) {
        ret = RegQueryValueEx( hkey, settingLabels[Font_Facename]
                               , 0, &type, (LPBYTE) &szData, &size );
        if ( ret == ERROR_SUCCESS )
            strcpy(kglob->faceName,szData);

        size = sizeof(DWORD);
        type = REG_DWORD;
        ret = RegQueryValueEx( hkey, settingLabels[Font_Height]
                        , 0, &type, (LPBYTE) &dwData, &size );

        if( ret == ERROR_SUCCESS )
            kglob->fontHeight = dwData;

        ret = RegQueryValueEx( hkey, settingLabels[Font_Width]
                        , 0, &type, (LPBYTE) &dwData, &size );

        if( ret == ERROR_SUCCESS )
            kglob->fontWidth = dwData;

        RegCloseKey( hkey );
    }
    return done;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
Bool KRegistryEdit::setFontinfo( K_GLOBAL* kglob )
{
    HKEY hkey = openSettings();
    if( !hkey )
        return FALSE;

    LONG ret = ERROR_SUCCESS;

    ret = RegSetValueEx( hkey, settingLabels[Font_Facename]
                         , 0, REG_SZ, (BYTE*) &kglob->faceName
                         , strlen(kglob->faceName) );

    ret = RegSetValueEx( hkey, settingLabels[Font_Height]
                         , 0, REG_DWORD, (BYTE*) &kglob->fontHeight, sizeof(DWORD) );

    ret = RegSetValueEx( hkey, settingLabels[Font_Width]
                         , 0, REG_DWORD, (BYTE*) &kglob->fontWidth, sizeof(DWORD) );

    RegCloseKey( hkey );
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
HKEY KRegistryEdit::openSettings()
{
    // if the key exists, it will open (not create) the key.
    //
    HKEY hkey = 0;
    DWORD disp = 0;

    LONG ret = RegCreateKeyEx( HKEY_CURRENT_USER
                , "Software\\Kermit"        // parent key
                , 0                         // reserved
                , "Kermit Settings"         // class
                , REG_OPTION_NON_VOLATILE   // open non-volitile key
                , KEY_ALL_ACCESS            // options flag
                , NULL                      // opened key's sec attr
                , &hkey                     // opened key
                , &disp );                  // disposition

    if( ret != ERROR_SUCCESS )
        return 0;

    return hkey;
}

