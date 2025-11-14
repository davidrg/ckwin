#include <ui_win.hpp>
#define USE_K95_INIT
#undef COMMENT
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include "k95cinit.hpp"
#include "cinit.hpp"

K_CINIT::K_CINIT(ZIL_ICHAR * portname)
   : UIW_WINDOW("K95_INIT",defaultStorage)
{
    windowManager->Center(this) ;

    _fPort = (UIW_STRING *) Get(FIELD_PORT);
    _fStatus = (UIW_STRING *) Get(FIELD_STATUS);
    _portname = strdup(portname);
}

K_CINIT::~K_CINIT(void)
{
}

EVENT_TYPE 
K_CINIT::Event( const UI_EVENT & event )
{
    ZIL_ICHAR msg[128] ;
    EVENT_TYPE retval = event.type;
    UI_EVENT open_evt( OPEN_COMM );
    UI_EVENT send_evt( SEND_DATA );
    UI_EVENT close_evt( CLOSE_COMM );
    UI_EVENT done_evt( COMM_DONE );
    UI_EVENT quit_evt( S_CLOSE );
    int err;
    DCB dcb;

   switch ( event.type ) {
   case OPEN_COMM:
       _fPort->DataSet(_portname);
       _fStatus->DataSet("Opening");

       idComDev = OpenComm(_portname, 1024, 128);
       if ( idComDev < 0 ) {
	   sprintf(msg, "Unable to Open: %d",idComDev);
	   _fStatus->DataSet(msg);
	   // eventManager->Put(done_evt);
	   break;
       }

#ifdef COMMENT
       err = BuildCommDCB("COM1:9600,n,8,1",&dcb);
       if ( err < 0 ) {
	   sprintf(msg, "Unable to BuildCommDCB: %d", err);
	   _fStatus->DataSet(msg);
	   break;
       }
#else
       err = GetCommState(idComDev,&dcb);
       if ( err < 0 ) {
	   sprintf(msg, "Unable to GetCommState: %d", err);
	   _fStatus->DataSet(msg);
	   break;
       }

       dcb.BaudRate = CBR_9600;
       dcb.Parity   = NOPARITY;
       dcb.StopBits = ONESTOPBIT;
       dcb.ByteSize = 8;
#endif

       err = SetCommState(&dcb);
       if ( err < 0 ) {
	   sprintf( msg, "Unable to SetCommState: %d", err );
	   _fStatus->DataSet(msg);
	   break;
       }
       eventManager->Put(send_evt);
       break;
       
   case SEND_DATA:
       _fStatus->DataSet("Sending data");
       err = TransmitCommChar( idComDev, '\r' );
       if ( err < 0 ) {
	   sprintf(msg, "Unable to Transmit: %d", err);
	   _fStatus->DataSet(msg);
       }
       else
       {
	   eventManager->Put(close_evt);
       }
       break;

   case CLOSE_COMM:
       _fStatus->DataSet("Closing port");
       CloseComm(idComDev);
       eventManager->Put(done_evt);
       break;

   case COMM_DONE:
       _fStatus->DataSet("Done");
       eventManager->Put(quit_evt);
       break;

   default:
       retval = UIW_WINDOW::Event(event) ;
   }	
    return retval ; 
}
