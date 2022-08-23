#define INCL_DOSPROCESS
#include <ui_win.hpp>
#ifdef COMMENT
#undef COMMENT
#endif
#define USE_DIALOG_GUI_SETTINGS
#define USE_DERIVED_OBJECTS
#define USE_HELP_CONTEXTS
#include <stdlib.h>
#include <direct.h>
#include <direct.hpp>
#include <stdio.h>
#include "dialer.hpp"
#include "lstitm.hpp"
#include "ksetgui.hpp"
#include "kconnect.hpp"
#include "usermsg.hpp"

ZIL_ICHAR K_DIALOG_GUI_SETTINGS::_className[] = "K_DIALOG_GUI_SETTINGS" ;

extern K_CONNECTOR * connector ;

static char *
os2getinidir( void ) 
{
    static char buffer[512] ;
    char * kermrc = "k95.ini", *lp ;
/*
  The -y init file must be fully specified or in the current directory.
  KERMRC is looked for via INIT, DPATH and PATH in that order.  Finally, our
  own executable file path is taken and the .EXE suffix is replaced by .INI
  and this is tried as the initialization file.
*/
    char * env = 0 ;
#ifdef WIN32
    env = getenv("K95.INI") ;
#else
    env = getenv("K2.INI");
#endif
  
    buffer[0] = '\0' ;

    if (env)
	strcpy(buffer,env);
    if (buffer[0] == 0)
	_searchenv(kermrc,"INIT",buffer);
    if (buffer[0] == 0)
	_searchenv(kermrc,"DPATH",buffer);
    if (buffer[0] == 0)
	_searchenv(kermrc,"PATH",buffer);
    if (buffer[0] == 0) {
#ifdef WIN32
	GetModuleFileName( NULL, buffer, 512 ) ;
#else
	PTIB pptib;
	PPIB pppib;
	char *szPath;

	DosGetInfoBlocks(&pptib, &pppib);

	szPath = pppib -> pib_pchcmd;

	while (*szPath)
	    szPath = strchr(szPath, 0) + 1;
	strcpy(buffer,szPath);
#endif
	if (buffer) {
	    lp = strrchr(buffer, '\\');
	    if ( !lp )
		lp = buffer ;
#ifdef WIN32
	    strcpy(lp+1, "k95.ini");
#else
	    strcpy(lp+1, "k2.ini");
#endif
	}
   }
   return buffer;
}       


static ZIL_INT32
LoadFileIntoText( UIW_TEXT * text, ZIL_ICHAR * filename )
{
   ULONG i ;
   FILE * InFile = NULL;
   ZIL_ICHAR buffer[30001] ;
   ZIL_ICHAR startupdir[512], inidir[512], exedir[512], * p ;
   ZIL_ICHAR searchpath[512] ;
   int n2 ;

   memset( buffer, 0, 30001 ) ;

   for ( i=0; InFile == NULL && (i<=15) ; i++ )
   {
      switch( i )
      {
      case 0:
         strcpy( searchpath, filename ) ;
         break;
      case 1:
         /* Identify the startupdir */
         getcwd(startupdir,512) ;
         n2 = strlen( startupdir ) ;
         if ( startupdir[n2-1] != '\\' )
         {
            startupdir[n2++] = '\\' ;
            startupdir[n2] = '\0' ;
         }
         if ( n2 > -1 && n2 < 256 )
            if ( startupdir[n2-1] != '\\' )
            {
               startupdir[n2] = '\\' ;
               startupdir[n2+1] = '\0' ;
            }
           
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, filename ) ;
         break;
      case 2:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "scripts\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 3:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "keymaps\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 4:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "phones\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 5:
         strcpy( searchpath, startupdir ) ;
         strcat( searchpath, "network\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 6:
         /* Identify the inidir */
         strcpy( inidir, os2getinidir() );
         if ( (p = strrchr( inidir, '\\')) != NULL )
            *p = '\0' ;
         n2 = strlen( inidir ) ;
         if ( n2 > -1 && n2 < 256 )
            if ( inidir[n2-1] != '\\' )
            {
               inidir[n2] = '\\' ;
               inidir[n2+1] = '\0' ;
            }
           
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, filename ) ;
         break;
      case 7:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "scripts\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 8:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "keymaps\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 9:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "phones\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 10:
         strcpy( searchpath, inidir ) ;
         strcat( searchpath, "network\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 11:
         /* Identify the exedir */
#ifdef WIN32
          GetModuleFileName( NULL, exedir, 512 ) ;
#else 
	  extern ZIL_ICHAR * pathname ;
	  strcpy( exedir, pathname ) ;
#endif
         if ( (p = strrchr( exedir, '\\')) != NULL )
            *p = '\0' ;
         n2 = strlen( exedir ) ;
         if ( n2 > -1 && n2 < 256 )
            if ( exedir[n2-1] != '\\' )
            {
               exedir[n2] = '\\' ;
               exedir[n2+1] = '\0' ;
            }
           
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, filename ) ;
         break;
      case 12:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "scripts\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 13:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "keymaps\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 14:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "phones\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      case 15:
         strcpy( searchpath, exedir ) ;
         strcat( searchpath, "network\\" ) ;
         strcat( searchpath, filename ) ;
         break;
      }

      InFile = fopen ( searchpath, "rb" ) ;
   }

   /* Finally read the file */
   if ( InFile )
   {
       for ( int i=0;i<30000;i++ ) {
	   if ( feof(InFile) )
	       break;
	   buffer[i] = fgetc(InFile) ;
       }
       if ( buffer[0] ) {
           text->Information( I_SET_VALUE, buffer ) ;
	   text->DataSet( buffer, 30000 ) ;
       }
       fclose( InFile ) ; 
   }    
   return 0 ;
}


// Null Constructor for K_DIALOG_GUI_SETTINGS
//
//

K_DIALOG_GUI_SETTINGS::
K_DIALOG_GUI_SETTINGS(void)
   : K95_SETTINGS_DIALOG("DIALOG_GUI_SETTINGS",ADD_ENTRY),
   _entry(NULL),
   _initialized(0)
{
    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    // This constructor will never be called because this object
    // is only used to edit existing objects.

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);
}


const ZIL_USER_EVENT S_ADD_RECORD	= 20000;
const ZIL_USER_EVENT S_DELETE_RECORD	= 20001;
const ZIL_USER_EVENT S_CALCULATE_TOTALS	= 20002;

struct RGB_RECORD
{
    ZIL_ICHAR color[32];
    int red, green, blue;
};

static RGB_RECORD _rgbData[16] =
{
    {   " 0 - black", 0, 0, 0 },
    {   " 1 - blue", 0, 0, 127 },
    {   " 2 - green", 0, 127, 0 },
    {   " 3 - cyan", 0, 127, 127 },
    {   " 4 - red", 127, 0, 0 },
    {   " 5 - magenta", 127, 0, 127 },
    {   " 6 - brown", 127, 127, 0 },
    {   " 7 - lightgray", 192, 192, 192 },
    {   " 8 - darkgray", 127, 127, 127 },
    {   " 9 - lightblue", 0, 0, 255 },
    {   "10 - lightgreen", 0, 255, 0 },
    {   "11 - lightcyan", 0, 255, 255 },
    {   "12 - lightred", 255, 0, 0},
    {   "13 - lightmagenta", 255, 0, 255 },
    {   "14 - yellow", 255, 255, 0 },
    {   "15 - white", 255, 255, 255 }
};

class ZIL_EXPORT_CLASS SAMPLE_COLOR : public UIW_BUTTON {
  public:
    static ZIL_ICHAR _className[] ;
    virtual ZIL_ICHAR *ClassName(void) { return (_className); }

    SAMPLE_COLOR(void) : UIW_BUTTON(0,0,0, ZIL_NULLP(ZIL_ICHAR),
                                     BTF_NO_TOGGLE | BTF_AUTO_SIZE, 
                                     WOF_JUSTIFY_CENTER | WOF_NO_ALLOCATE_DATA | WOF_NON_SELECTABLE)
    { 
        _palette.fillCharacter = ' ';
        _palette.colorAttribute = attrib(BLACK, WHITE);
        _palette.monoAttribute = attrib(MONO_HIGH, MONO_BLACK);
        _palette.fillPattern = PTN_SOLID_FILL;
        _palette.colorForeground = BLACK;
        _palette.colorBackground = WHITE;
        _palette.bwForeground = BW_WHITE;
        _palette.bwBackground = BW_BLACK;
        _palette.grayScaleForeground = GS_BLACK;
        _palette.grayScaleBackground = GS_GRAY;

        woStatus |= WOS_OWNERDRAW;
        SetRGB(0,0,0);
    };

    SAMPLE_COLOR(int left, int top, int width, int red, int green, int blue) :
        UIW_BUTTON(left, top, width, ZIL_NULLP(ZIL_ICHAR),
                    BTF_NO_TOGGLE | BTF_AUTO_SIZE, 
                    WOF_JUSTIFY_CENTER | WOF_NO_ALLOCATE_DATA | WOF_NON_SELECTABLE)
        {
            _palette.fillCharacter = ' ';
            _palette.colorAttribute = attrib(BLACK, WHITE);
            _palette.monoAttribute = attrib(MONO_HIGH, MONO_BLACK);
            _palette.fillPattern = PTN_SOLID_FILL;
            _palette.colorForeground = BLACK;
            _palette.colorBackground = WHITE;
            _palette.bwForeground = BW_WHITE;
            _palette.bwBackground = BW_BLACK;
            _palette.grayScaleForeground = GS_BLACK;
            _palette.grayScaleBackground = GS_GRAY;

            woStatus |= WOS_OWNERDRAW;
            SetRGB(red, green, blue);
        }
    
    virtual ~SAMPLE_COLOR(void) { };

    virtual EVENT_TYPE DrawItem(const UI_EVENT &event, EVENT_TYPE ccode) {
        display->VirtualGet(screenID, trueRegion);

        _palette.colorForeground = RGB_BLACK;
        _palette.fillPattern = PTN_RGB_COLOR;
        _palette.colorBackground = (_blue << 16) | (_green << 8) | (_red);

	display->Rectangle(screenID, trueRegion, &_palette, 0, TRUE, FALSE, &clip);
	display->VirtualPut(screenID);
	return (TRUE);
    }

    void SetRGB(int red, int green, int blue) {
        _red = red;
        _green = green;
        _blue = blue;
    }

#if defined(ZIL_LOAD)
    void DataLoss(const ZIL_ICHAR * name)
    {
        ZAF_MESSAGE_WINDOW * message =
            new ZAF_MESSAGE_WINDOW( "Data Corruption Error", 
                                "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                "Data corruption detected while loading \"%s\"",
                                name);
        if ( message ) {
            message->Control() ;	
            delete message ;
        }
    }

    virtual ZIL_NEW_FUNCTION NewFunction(void) { return(SAMPLE_COLOR::New); }

    static UI_WINDOW_OBJECT *New(const ZIL_ICHAR *name,
                                  ZIL_STORAGE_READ_ONLY *file = ZIL_NULLP(ZIL_STORAGE_READ_ONLY),
                                 ZIL_STORAGE_OBJECT_READ_ONLY *object = ZIL_NULLP(ZIL_STORAGE_OBJECT_READ_ONLY),
                                 UI_ITEM *_objectTable = ZIL_NULLP(UI_ITEM),
                                 UI_ITEM *_userTable = ZIL_NULLP(UI_ITEM))
   { return (new SAMPLE_COLOR(name, file, object, _objectTable, _userTable)); }

    SAMPLE_COLOR( const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
                  ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, 
                  UI_ITEM *userTable) :
                      UIW_BUTTON( 0,  // Left
                                  0,  // Top
                                  0,  // Width
                                  ZIL_NULLP(ZIL_ICHAR),
                                  BTF_NO_TOGGLE | BTF_AUTO_SIZE, 
                                  WOF_JUSTIFY_CENTER | WOF_NO_ALLOCATE_DATA | WOF_NON_SELECTABLE)
                      {
                          _palette.fillCharacter = ' ';
                          _palette.colorAttribute = attrib(BLACK, WHITE);
                          _palette.monoAttribute = attrib(MONO_HIGH, MONO_BLACK);
                          _palette.fillPattern = PTN_SOLID_FILL;
                          _palette.colorForeground = BLACK;
                          _palette.colorBackground = WHITE;
                          _palette.bwForeground = BW_WHITE;
                          _palette.bwBackground = BW_BLACK;
                          _palette.grayScaleForeground = GS_BLACK;
                          _palette.grayScaleBackground = GS_GRAY;

                          woStatus |= WOS_OWNERDRAW;
                          SetRGB(0, 0, 0);
                      };

    void Load(const ZIL_ICHAR *name, ZIL_STORAGE_READ_ONLY *directory,
               ZIL_STORAGE_OBJECT_READ_ONLY *file, UI_ITEM *objectTable, UI_ITEM *userTable)
    {
        // Load the button information.
        UIW_BUTTON::Load(name, directory, file, objectTable, userTable);
        _palette.fillCharacter = ' ';
        _palette.colorAttribute = attrib(BLACK, WHITE);
        _palette.monoAttribute = attrib(MONO_HIGH, MONO_BLACK);
        _palette.fillPattern = PTN_SOLID_FILL;
        _palette.colorForeground = BLACK;
        _palette.colorBackground = WHITE;
        _palette.bwForeground = BW_WHITE;
        _palette.bwBackground = BW_BLACK;
        _palette.grayScaleForeground = GS_BLACK;
        _palette.grayScaleBackground = GS_GRAY;

        woStatus |= WOS_OWNERDRAW;
        SetRGB(0,0,0);
    }
#endif

#if defined(ZIL_STORE)
    void Store(const ZIL_ICHAR *name, ZIL_STORAGE *directory,
                ZIL_STORAGE_OBJECT *file, UI_ITEM *objectTable, UI_ITEM *userTable)
    {
	// Store the button information.
        UIW_BUTTON::Store(name, directory, file, objectTable, userTable);
    }
#endif

    SAMPLE_COLOR * Next(void) { 
        return( SAMPLE_COLOR *)next; 
    }

  private:
    int _red, _green, _blue;
    UI_PALETTE _palette;
};
ZIL_ICHAR SAMPLE_COLOR::_className[] = "SAMPLE_COLOR" ;

EVENT_TYPE RecordFunction(UI_WINDOW_OBJECT *object, UI_EVENT &event, EVENT_TYPE ccode)
{
    RGB_RECORD *rgbRecord = (RGB_RECORD *)event.data;
    UIW_STRING       *colorField;
    UIW_SPIN_CONTROL *redField;
    UIW_SPIN_CONTROL *greenField;
    UIW_SPIN_CONTROL *blueField;
    SAMPLE_COLOR *    sample;

    if (ccode == S_SET_DATA)
    {
        colorField = (UIW_STRING *)object->Get("COLOR_FIELD");
        if ( colorField )
            colorField->DataSet( rgbRecord->color, 32 ) ;
        redField = (UIW_SPIN_CONTROL *)object->Get("RED_FIELD");
        if ( redField )
            redField->Information( I_SET_VALUE, &rgbRecord->red ) ;
        blueField = (UIW_SPIN_CONTROL *)object->Get("BLUE_FIELD");
        if ( blueField )
            blueField->Information( I_SET_VALUE, &rgbRecord->blue ) ;
        greenField = (UIW_SPIN_CONTROL *)object->Get("GREEN_FIELD");
        if ( greenField )
            greenField->Information( I_SET_VALUE, &rgbRecord->green ) ;
        sample = (SAMPLE_COLOR *)object->Get("COLOR_SAMPLE");
        if ( sample )
            sample->SetRGB(rgbRecord->red, rgbRecord->green, rgbRecord->blue);
    }
    else if (ccode == S_NON_CURRENT || ccode == L_SELECT)
    {
        redField = (UIW_SPIN_CONTROL *)object->Get("RED_FIELD");
        if ( redField )
            redField->Information( I_GET_VALUE, &rgbRecord->red ) ;
        blueField = (UIW_SPIN_CONTROL *)object->Get("BLUE_FIELD");
        if ( blueField )
            blueField->Information( I_GET_VALUE, &rgbRecord->blue ) ;
        greenField = (UIW_SPIN_CONTROL *)object->Get("GREEN_FIELD");
        if ( greenField )
            greenField->Information( I_GET_VALUE, &rgbRecord->green ) ;
        sample = (SAMPLE_COLOR *)object->Get("COLOR_SAMPLE");
        if ( sample )
            sample->SetRGB(rgbRecord->red, rgbRecord->green, rgbRecord->blue);
    }
    return (ccode);
}

// KD_LIST_ITEM Constructor for K_DIALOG_GUI_SETTINGS
//
//

K_DIALOG_GUI_SETTINGS::
K_DIALOG_GUI_SETTINGS( KD_LIST_ITEM * entry, enum ENTRYMODE mode )
   : K95_SETTINGS_DIALOG("DIALOG_GUI_SETTINGS",mode),
   _entry(entry),
   _initialized(0)
{
    printf("Entering K_DIALOG_GUI_SETTINGS::K_DIALOG_GUI_SETTINGS(entry)\n");

    directoryService = _serviceManager->Get(ID_DIRECTORY_SERVICE);
    windowManager->Center(this) ;

    ZIL_ICHAR dialogName[256];
    sprintf(dialogName,"GUI Settings: %s",entry->_name);
    Information( I_SET_TEXT, dialogName ) ;

    // Set the RGB Values
    for ( int i=0; i < 16 ; i++ ) {
        _rgbData[i].red = entry->_rgb[i][0];
        _rgbData[i].green = entry->_rgb[i][1];
        _rgbData[i].blue = entry->_rgb[i][2];
    }

    UIW_TABLE_RECORD *tableRecord;
    UIW_SPIN_CONTROL *redField, *blueField, *greenField;
    UIW_STRING *      colorText;
//    SAMPLE_COLOR *      sample;

    UIW_TABLE_HEADER * header = new UIW_TABLE_HEADER(THF_COLUMN_HEADER);

    tableRecord = new UIW_TABLE_RECORD(71, 1);
    *tableRecord 
        + new UIW_PROMPT(2, 0, "Color")
        + new UIW_PROMPT(17, 0, "Red")
        + new UIW_PROMPT(31, 0, "Green")
        + new UIW_PROMPT(45, 0, "Blue") 
		+ new UIW_PROMPT(59, 0, "");
	   
    *header + tableRecord;

    tableRecord = new UIW_TABLE_RECORD(71, 1, RecordFunction);
    tableRecord->wnFlags |= WNF_SELECT_MULTIPLE;
    tableRecord->StringID("RGB_TABLE_RECORD");

    colorText = new UIW_STRING(0, 0, 16, "", 32);
    colorText->woFlags |= WOF_VIEW_ONLY ;
    colorText->Information(I_CHANGED_FLAGS,NULL) ;

    int defaultInt = 0;
    redField = new UIW_SPIN_CONTROL(16, 0, 14, new UIW_INTEGER(20, 4, 10, &defaultInt, 
                                                                "0..255", NMF_NO_FLAGS, 
                                                                WOF_VIEW_ONLY | WOF_BORDER));
    greenField = new UIW_SPIN_CONTROL(30, 0, 14, new UIW_INTEGER(20, 4, 10, &defaultInt, 
                                                                  "0..255", NMF_NO_FLAGS, 
                                                                  WOF_VIEW_ONLY | WOF_BORDER));
    blueField = new UIW_SPIN_CONTROL(44,0, 14, new UIW_INTEGER(20, 4, 10, &defaultInt, 
                                                                "0..255", NMF_NO_FLAGS, 
                                                                WOF_VIEW_ONLY | WOF_BORDER));
//    sample = new SAMPLE_COLOR(58, 0, 10, 0, 0, 0);

    colorText->StringID("COLOR_FIELD");
    redField->StringID("RED_FIELD");
    blueField->StringID("BLUE_FIELD");
    greenField->StringID("GREEN_FIELD");
//    sample->StringID("COLOR_SAMPLE");

    *tableRecord + colorText;
    *tableRecord + redField;
    *tableRecord + greenField;
    *tableRecord + blueField;
//    *tableRecord + sample;

    table = new UIW_TABLE(0, 0, 0, 0, 1, sizeof(RGB_RECORD), 16, _rgbData,
                           16, TBLF_GRID, WOF_BORDER | WOF_NO_ALLOCATE_DATA | 
                           WOF_NON_FIELD_REGION);
    table->StringID("RGB_TABLE");
    *table + header;
    *table + new UIW_SCROLL_BAR(0, 0, 0, 0, SBF_VERTICAL);
    *table + tableRecord;

    *((UIW_GROUP *)Get(TABLE_GROUP)) + table;
    tableRecord->Information(I_CHANGED_FLAGS,NULL);

    UIW_BUTTON * button;
    if ( _mode == ADD_ENTRY ) {
        button = (UIW_BUTTON *) Get(BUTTON_SET_GUI_OK);
        button->DataSet("Save/Next");
        button = (UIW_BUTTON *) Get(BUTTON_SET_GUI_CANCEL);
        button->DataSet("Cancel");
    }

    button = (UIW_BUTTON *)Get(CHECK_GUI_USE);
    if ( entry->_use_gui )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;

    if ( !entry->_use_gui ) {
        redField->woFlags |= WOF_NON_SELECTABLE;
        redField->Information(I_CHANGED_FLAGS,NULL);
        blueField->woFlags |= WOF_NON_SELECTABLE;
        blueField->Information(I_CHANGED_FLAGS,NULL);
        greenField->woFlags |= WOF_NON_SELECTABLE;
        greenField->Information(I_CHANGED_FLAGS,NULL);
    }

    InitFacenameList();

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_FACENAME);
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get(LIST_GUI_FACENAME);
    SetComboToFacename(combo,entry->_facename);
    if ( !entry->_use_gui ) {
        combo->woFlags |= WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL);
    }

    UIW_SPIN_CONTROL * spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_FONT_SIZE);
    UIW_INTEGER * integer = (UIW_INTEGER *) Get(GUI_INT_FONT_SIZE);
    int value = _entry->_fontsize;
    integer->Information( I_SET_VALUE, &value ) ;
    if ( !entry->_use_gui ) {
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);
    }

    button = (UIW_BUTTON *) Get(CHECK_GUI_DEFAULT_POS);
    if ( entry->_startpos_auto )
        button->woStatus |= WOS_SELECTED ;
    else
        button->woStatus &= ~WOS_SELECTED ;
    button->Information( I_CHANGED_STATUS, NULL ) ;
    if ( !entry->_use_gui ) {
        button->woFlags |= WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);
    }
    spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_XPOS);
    integer = (UIW_INTEGER *) Get(INT_GUI_XPOS);
    value = _entry->_startpos_x;
    integer->Information( I_SET_VALUE, &value ) ;
    if ( !entry->_use_gui || entry->_startpos_auto ) {
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);
    }

    spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_YPOS);
    integer = (UIW_INTEGER *) Get(INT_GUI_YPOS);
    value = _entry->_startpos_y;
    integer->Information( I_SET_VALUE, &value ) ;
    if ( !entry->_use_gui || entry->_startpos_auto ) {
        spin->woFlags |= WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);
    }

    combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_RESIZE);
    list = (UIW_VT_LIST *) Get(LIST_GUI_RESIZE);
    SetResizeMode(combo,entry->_gui_resize);

    combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_RUN);
    list = (UIW_VT_LIST *) Get(LIST_GUI_RUN);
    SetRunMode(combo,entry->_gui_run);

    UI_EVENT event(OPT_INITIALIZED);
    event.windowObject = this ;
    eventManager->Put(event);

    printf("Exiting K_DIALOG_GUI_SETTINGS::K_DIALOG_GUI_SETTINGS(entry,mode)\n");
};

K_DIALOG_GUI_SETTINGS::~K_DIALOG_GUI_SETTINGS(void)
{
}

EVENT_TYPE K_DIALOG_GUI_SETTINGS::Event( const UI_EVENT & event )
{
    printf("K_DIALOG_GUI_SETTINGS::Event()\n");

    extern K_CONNECTOR * connector ; 
    EVENT_TYPE retval = event.type ; 
    UIW_STRING * string = NULL ;
    UIW_BUTTON * button = NULL ;
    UIW_BUTTON * button2 = NULL ;
    UIW_COMBO_BOX * combo = NULL ;
    UIW_TEXT * text = NULL ;
    UIW_INTEGER * integer = NULL ;
    UIW_SPIN_CONTROL * spin = NULL ;
    UIW_GROUP * group = NULL ;
    UIW_VT_LIST * list = NULL ;
    UIW_FORMATTED_STRING * format = NULL;
    UIW_TABLE_RECORD * tableRecord = NULL;
    UIW_SPIN_CONTROL *redField, *blueField, *greenField;
    int value = 0 ;
    KD_LIST_ITEM * _template = NULL ;
    K_MODEM * modem = NULL;
    static EVENT_TYPE DirServRequestor = 0 ;
    UI_EVENT newevt(event);

    if ( !_initialized ) {
	if ( event.type == OPT_INITIALIZED )
	    _initialized = 1 ;
	else 
	    retval = UIW_WINDOW::Event(event);
    }
    else 
    switch( event.type ) 
    {
    case OPT_GUI_USE:
        value = FlagSet(Get(CHECK_GUI_USE)->woStatus,WOS_SELECTED);

        redField = (UIW_SPIN_CONTROL *) Get("RED_FIELD");
        blueField = (UIW_SPIN_CONTROL *) Get("BLUE_FIELD");
        greenField = (UIW_SPIN_CONTROL *) Get("GREEN_FIELD");
        if ( !value ) {
            redField->woFlags |= WOF_NON_SELECTABLE;
            blueField->woFlags |= WOF_NON_SELECTABLE;
            greenField->woFlags |= WOF_NON_SELECTABLE;
        } else {
            redField->woFlags &= ~WOF_NON_SELECTABLE;
            blueField->woFlags &= ~WOF_NON_SELECTABLE;
            greenField->woFlags &= ~WOF_NON_SELECTABLE;
        }
        redField->Information(I_CHANGED_FLAGS,NULL);
        blueField->Information(I_CHANGED_FLAGS,NULL);
        greenField->Information(I_CHANGED_FLAGS,NULL);

        combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_FACENAME);
        if ( !value )
            combo->woFlags |= WOF_NON_SELECTABLE;
        else
            combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL);

        combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_RESIZE);
        if ( !value )
            combo->woFlags |= WOF_NON_SELECTABLE;
        else
            combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL);

        combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_RUN);
        if ( !value )
            combo->woFlags |= WOF_NON_SELECTABLE;
        else
            combo->woFlags &= ~WOF_NON_SELECTABLE;
        combo->Information(I_CHANGED_FLAGS,NULL);

        spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_FONT_SIZE);
        if ( !value )
            spin->woFlags |= WOF_NON_SELECTABLE;
        else
            spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);

        button = (UIW_BUTTON *) Get(CHECK_GUI_DEFAULT_POS);
        if ( !value )
            button->woFlags |= WOF_NON_SELECTABLE;
        else
            button->woFlags &= ~WOF_NON_SELECTABLE;
        button->Information(I_CHANGED_FLAGS,NULL);

        spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_XPOS);
        if ( !value )
            spin->woFlags |= WOF_NON_SELECTABLE;
        else
            spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);

        spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_YPOS);
        if ( !value )
            spin->woFlags |= WOF_NON_SELECTABLE;
        else
            spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);
        break;

    case OPT_STARTUP_POS:
        value = FlagSet(Get(CHECK_GUI_DEFAULT_POS)->woStatus,WOS_SELECTED);
        spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_XPOS);
        if ( value )
            spin->woFlags |= WOF_NON_SELECTABLE;
        else
            spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);

        spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_YPOS);
        if ( value )
            spin->woFlags |= WOF_NON_SELECTABLE;
        else
            spin->woFlags &= ~WOF_NON_SELECTABLE;
        spin->Information(I_CHANGED_FLAGS,NULL);
        break;

    case OPT_HELP_SET_GUI:
        helpSystem->DisplayHelp( windowManager, HELP_SET_GUI ) ;
        break;

    case OPT_GUI_OK: {
       ApplyChangesToEntry() ;
       ZIL_STORAGE_OBJECT _fileObj( *K_CONNECTOR::_userFile, _entry->_name,
                                      ID_KD_LIST_ITEM, UIS_OPENCREATE | UIS_READWRITE ) ;
       if ( _fileObj.objectError )
       {
            // What went wrong?
	   ZAF_MESSAGE_WINDOW * message =
               new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                       "Unable to Open %s: errorno %d", _entry->_name, _fileObj.objectError ) ;
	   message->Control() ;
	   delete message ;
       }  
       else 
       {
	   _entry->Store( _entry->_name, K_CONNECTOR::_userFile, &_fileObj,
			  ZIL_NULLP(UI_ITEM), ZIL_NULLP(UI_ITEM) ) ;
	   K_CONNECTOR::_userFile->Save() ;
	   if ( K_CONNECTOR::_userFile->storageError )
	   {
               // What went wrong?
               ZAF_MESSAGE_WINDOW * message =
		   new ZAF_MESSAGE_WINDOW( "Error", "EXCLAMATION", ZIL_MSG_OK, ZIL_MSG_OK,
                                          "Unable to Open %s: errorno %d", _entry->_name, 
                                          K_CONNECTOR::_userFile->storageError ) ;
               message->Control() ;
               delete message ;
	   }

	   connector->AddEntry( _entry ) ;
       }
        _cancelled = 0;
        UI_EVENT event(S_CLOSE);
        event.windowObject = this ;
        eventManager->Put(event);
        break;
    }

    default:
       retval = UIW_WINDOW::Event(event);
   }	

   return retval ;
}

// Apply Current Settings of Notebook to Entry
//
//

void K_DIALOG_GUI_SETTINGS::
ApplyChangesToEntry( void )
{
    _entry->_use_gui = FlagSet(Get(CHECK_GUI_USE)->woStatus,WOS_SELECTED);

    UIW_COMBO_BOX * combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_FACENAME);
    strcpy(_entry->_facename, GetFacenameFromCombo(combo)) ;

    combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_RESIZE);
    _entry->_gui_resize = GetResizeMode(combo);

    combo = (UIW_COMBO_BOX *) Get(COMBO_GUI_RUN);
    _entry->_gui_run = GetRunMode(combo);

    UIW_SPIN_CONTROL * spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_FONT_SIZE);
    UIW_INTEGER * integer = (UIW_INTEGER *) Get(GUI_INT_FONT_SIZE);
    int value;
    integer->Information( I_GET_VALUE, &value);
    _entry->_fontsize = value;

    _entry->_startpos_auto = FlagSet(Get(CHECK_GUI_DEFAULT_POS)->woStatus,WOS_SELECTED);

    spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_XPOS);
    integer = (UIW_INTEGER *) Get(INT_GUI_XPOS);
    integer->Information( I_GET_VALUE, &value);
    _entry->_startpos_x = value;

    spin = (UIW_SPIN_CONTROL *) Get(SPIN_GUI_YPOS);
    integer = (UIW_INTEGER *) Get(INT_GUI_YPOS);
    integer->Information( I_GET_VALUE, &value );
    _entry->_startpos_y = value;

    for ( int i=0 ; i<16; i++ ) {
        _entry->_rgb[i][0] = _rgbData[i].red;
        _entry->_rgb[i][1] = _rgbData[i].green;
        _entry->_rgb[i][2] = _rgbData[i].blue;
    }
}

#ifdef WIN32
#define MAXFNTS 1024
static struct _font_list {
    int count;
    char * name[MAXFNTS];
} * font_list = NULL;

static int CALLBACK
EnumFontFamExProc( ENUMLOGFONTEX *lpelfe,    // logical-font data
                   NEWTEXTMETRICEX *lpntme,  // physical-font data
                   DWORD FontType,           // type of font
                   LPARAM lParam             // application-defined data
                   )
{
    int i;
    char * name;

    if ( font_list == NULL)
        return(0);

    if (font_list->count == MAXFNTS )
        return(MAXFNTS);

    /* Windows 95/98 handles the font names different than NT/2000 */
    /* Why?  You know better than to ask that question.            */
    OSVERSIONINFO osverinfo ;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx( &osverinfo ) ;
    if ( osverinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
        if ( FontType == 4 /* True Type */ )
            name = (char *)lpelfe->elfFullName;
        else
            name = (char *)lpelfe->elfLogFont.lfFaceName;
    } else
        name = (char *)lpelfe->elfFullName;

    for ( i=0; i < font_list->count; i++ ) {
        if ( !strcmp(name,font_list->name[i]) )
            return(font_list->count);
    }
    font_list->name[font_list->count++] = strdup(name);
    return(font_list->count);
}

static int
EnumerateFonts()
{
    LOGFONT lf;
    HDC hdc;
    int i,rc=0;

    if ( font_list != NULL ) {
        for ( i=0 ; i < font_list->count; i++ ) {
            free(font_list->name[i]);
            font_list->name[i] = NULL;
        }
        font_list->count = 0;
    } else {
        font_list = (struct _font_list *)malloc(sizeof(struct _font_list));
        if ( !font_list )
            return(0);
        memset(font_list,0,sizeof(struct _font_list));
    }

    hdc = CreateDC("DISPLAY",NULL,NULL,NULL);
    if ( hdc ) {
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfFaceName[0] = '\0';
        lf.lfPitchAndFamily = 0;

        rc = EnumFontFamiliesEx( (HDC) hdc, (LPLOGFONT)&lf,
                                 (FONTENUMPROC) EnumFontFamExProc,
                                 0, 0);
        DeleteDC(hdc);
    }
    return rc;
}
#endif /* WIN32 */

void K_DIALOG_GUI_SETTINGS::
InitFacenameList(void)
{
#ifdef WIN32
    int i;
    UIW_BUTTON * button;
    UIW_VT_LIST * list = (UIW_VT_LIST *) Get( LIST_GUI_FACENAME ) ;

    EnumerateFonts();

    for ( i=0; i<font_list->count; i++ ) {
        button = new UIW_BUTTON( 0,0,0,font_list->name[i],BTF_NO_3D, WOF_NO_FLAGS );
        button->StringID(button->DataGet());
        *list + button;
    }
#endif /* WIN32 */
}

char * K_DIALOG_GUI_SETTINGS::
GetFacenameFromCombo( UIW_COMBO_BOX * combo )
{
    ZIL_ICHAR * value = combo->DataGet() ;
    return value;
}

void K_DIALOG_GUI_SETTINGS::
SetComboToFacename( UIW_COMBO_BOX * combo, char * facename )
{
    int i, x = 0;

#ifdef WIN32
    for (i = 0; i < font_list->count; i++) {
       if (!stricmp(font_list->name[i], facename)) {
           x = 1;
           break;
       }
    }

    if (x)
        combo->DataSet( font_list->name[i] ) ;
    else
        combo->DataSet( "Courier New" ) ;
#endif /* WIN32 */
}

enum GUI_RESIZE  K_DIALOG_GUI_SETTINGS::
GetResizeMode(UIW_COMBO_BOX * combo)
{
    ZIL_ICHAR * value = combo->DataGet() ;

    if ( !strcmp(value,"Scale Font") ) 
        return RES_SCALE_FONT;
    if ( !strcmp(value,"Change Dimensions") )
        return RES_CHANGE_DIM;
    return RES_CHANGE_DIM;
}

void  K_DIALOG_GUI_SETTINGS::
SetResizeMode(UIW_COMBO_BOX * combo, enum GUI_RESIZE mode)
{
    switch (mode) {
    case RES_SCALE_FONT:
        combo->DataSet("Scale Font");
        return;
    case RES_CHANGE_DIM:
    default:
        combo->DataSet("Change Dimensions");
        return;
    }
}

enum GUI_RUN  K_DIALOG_GUI_SETTINGS::
GetRunMode(UIW_COMBO_BOX * combo)
{
    ZIL_ICHAR * value = combo->DataGet() ;

    if ( !strcmp(value,"Normal window") ) 
        return RUN_RES;
    if ( !strcmp(value,"Maximized") )
        return RUN_MAX;
    if ( !strcmp(value,"Minimized") )
        return RUN_MIN;
    return RUN_RES;
}

void  K_DIALOG_GUI_SETTINGS::
SetRunMode(UIW_COMBO_BOX * combo, enum GUI_RUN mode)
{
    switch ( mode ) {
    case RUN_MAX:
        combo->DataSet("Maximized");
        return;
    case RUN_MIN:
        combo->DataSet("Minimized");
        return;
    case RUN_RES:
    default:
        combo->DataSet("Normal window");
        return;
    }
}

