#ifndef K_CONNECTOR_HPP
#define K_CONNECTOR_HPP

class KD_LIST_ITEM ;
class KD_CONFIG ;
class K_MODEM;
class K_LOCATION;

class ZIL_EXPORT_CLASS K_CONNECTOR : public UIW_WINDOW
{
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return _className ; } ;
    K_CONNECTOR(void) ; 
    ~K_CONNECTOR(void) ; 

   virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

   void AddEntry( KD_LIST_ITEM * entry ) ;
   ZIL_INT32 RemoveEntry( KD_LIST_ITEM * entry ) ;
   void UpdateStatusLine( ZIL_UINT32 ) ;
   void Connect( KD_LIST_ITEM * event ) ;
   void Shortcut( KD_LIST_ITEM * event ) ;
   void ScriptFile( KD_LIST_ITEM * event ) ;

   static ZIL_STORAGE * _userFile;
   static ZIL_STORAGE_READ_ONLY *_preloadFile ;
   static ZIL_STORAGE_READ_ONLY *_orgloadFile ;

   ZIL_UINT32 StartKermit( KD_LIST_ITEM * entry, KD_CONFIG * config, KD_LIST_ITEM * def_entry );
   ZIL_ICHAR * StartKermitFileName ; /* Output filename pointer */
   ZIL_UINT32  StartKermitErrorCode ;    /* This routine's error code */

   void FillListWithTemplateNames( UIW_VT_LIST * list ) ;
   ZIL_UINT8 IsATemplate( ZIL_ICHAR * name ) ;
   KD_LIST_ITEM * FindEntry( ZIL_ICHAR * name, ZIL_UINT8 = FALSE ) ;
   K_MODEM * FindModem( ZIL_ICHAR * name ) ;
   K_LOCATION * FindLocation( ZIL_ICHAR * name ) ;
   ZIL_UINT8 IsModemInUse( ZIL_ICHAR * name );

   void LoadEntries( void ) ;
   void LoadLocations( void ) ;
#if defined(WIN32)
   void LoadTapiModems( void ) ;
   void LoadTapiLocations( void ) ;
   void UpdateTapiLineConfigMenuItems( void );
#endif
   void LoadModems( void ) ;
   void LoadConfig( void ) ;
   void SaveConfig( void ) ;
   void GenerateDefaultLocationFromConfig();
   void UpdateLocationMenuItems( void );

   ZIL_ICHAR * GetEditorCommand(void);
   ZIL_ICHAR * GetFtpCommand(void);
   ZIL_ICHAR * GetBrowserCommand(void);
#if defined(WIN32)
   static void Real_Win32ShellExecute( void * object );
   ZIL_INT32 Win32ShellExecute( ZIL_ICHAR * object );
#endif
   void os2InitFromRegistry(void);
   ZIL_UINT32 Browse( ZIL_ICHAR * );
   static void ExecuteBrowser( void * );
   ZIL_UINT32 ReadManual(void);
   ZIL_UINT8  IsManualInstalled(void);

   ZIL_UINT32 GetPopupOffset(void);

   KD_CONFIG * _config ;
   EVENT_TYPE Connect( const UI_EVENT & event ) ;
   UI_LIST   * GetEntryList(void) { return &_entry_list; };
   void       OutofMemory(ZIL_ICHAR *);

   UI_LIST        _modem_list;
   UI_LIST        _location_list;
   ZIL_UINT8      _dns_srv_avail;
   ZIL_UINT8      _krb5_avail;
   ZIL_UINT8      _krb4_avail;
   ZIL_UINT8      _libeay_avail;
   ZIL_UINT8      _ssh_avail;

#ifdef WIN32
   HWND          _hwndNextConnect;
   DWORD         _dwNextConnectPid;
#endif /* WIN32 */

  private:
   ZIL_UINT32 CreateShortcut( KD_LIST_ITEM * entry, KD_CONFIG * config, 
			      KD_LIST_ITEM * def_entry );

   ZIL_UINT32 CreateScriptFile( KD_LIST_ITEM * entry, KD_CONFIG * config, 
			      KD_LIST_ITEM * def_entry );

   ZIL_UINT32 GenerateScript(KD_LIST_ITEM * entry, KD_CONFIG * config, 
			      KD_LIST_ITEM * def_entry, FILE * OutFile);

   ZIL_UINT32 GenerateLocation( K_LOCATION * location, FILE * OutFile );
   ZIL_UINT32 GenerateModem( K_MODEM * modem, KD_LIST_ITEM * entry, FILE * OutFile );

   ZIL_UINT32 ExportModems(void);
   ZIL_UINT32 ExportLocations(void);

   EVENT_TYPE Disconnect( const UI_EVENT & event ) ;
   EVENT_TYPE AddEntry( const UI_EVENT & event ) ;
   EVENT_TYPE EditEntry( const UI_EVENT & event ) ;
   EVENT_TYPE CloneEntry( const UI_EVENT & event ) ;
   EVENT_TYPE RemoveEntry( const UI_EVENT & event ) ;
   EVENT_TYPE QuickConnect( const UI_EVENT & event ) ;
   EVENT_TYPE SetApplications( const UI_EVENT & event ) ;
   EVENT_TYPE SetDirDial( const UI_EVENT & event ) ;
   EVENT_TYPE SetDirNetwork( const UI_EVENT & event ) ;
   EVENT_TYPE SetDialOptions( const UI_EVENT & event ) ;
   EVENT_TYPE SetModemOptions( const UI_EVENT & event ) ;
   EVENT_TYPE HelpAbout( const UI_EVENT & event ) ;
   EVENT_TYPE Shortcut( const UI_EVENT & event ) ;
   EVENT_TYPE ScriptFile( const UI_EVENT & event ) ;

   static struct _PROCESS_INFORMATION K95_PI; /* K95 Process Info struct */
   ZIL_UINT32 ReadKermitInfo( KD_CONFIG * config, KD_LIST_ITEM * defentry );

   UI_LIST        _entry_list;
   UIW_STRING *   StatusName;
   UIW_TIME *     StatusTime;
   UIW_STRING *   StatusString;
   UIW_VT_LIST *  VtList;
   UIW_TOOL_BAR * ToolBar;

   ZIL_ICHAR browser[256];
   ZIL_ICHAR browsopts[256];
   ZIL_ICHAR editor[256];
   ZIL_ICHAR editopts[256];
   ZIL_ICHAR ftpapp[256];
   ZIL_ICHAR ftpopts[256];
   ZIL_ICHAR manpath[256];
};

#define PASSWD_SZ 256

class ZIL_EXPORT_CLASS K_PASSWORD : public ZAF_DIALOG_WINDOW
{
  public:
     K_PASSWORD( K_CONNECTOR *, ZIL_ICHAR *, ZIL_UINT8 * ) ; 
    ~K_PASSWORD(void) ;

    virtual EVENT_TYPE Event( const UI_EVENT & event ) ;

  private:
    K_CONNECTOR *    _parent ;
    ZIL_ICHAR *      _password;
    ZIL_UINT8 *      _ok;
};



#endif