/*
 * file ckmpri.c
 *
 * Module of mackermit containing code for handling printing.  This code was
 * originally put into ckmusr by John A. Oberschelp of Emory U.
 *
 */

/*
 Copyright (C) 1989, Trustees of Columbia University in the City of New York.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as it is not sold for profit, provided this
 copyright notice is retained.
*/

#include "ckcdeb.h"
#include "ckcker.h"

#define	__SEG__ ckmpri
#include <desk.h>
#include <files.h>
#include <windows.h>
#include <events.h>
#include <dialogs.h>
#include <fonts.h>
#include <menus.h>
#include <toolutils.h>
#include <serial.h>
#include <textedit.h>
#include <segload.h>
#include <ctype.h>

#include <printing.h>	/*JAO*/

/* here is what is different */
#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif
#include <osutils.h>
/* PWP: put the #include for the script manager here! */

#include "ckmdef.h"		/* General Mac defs */
#include "ckmres.h"		/* Mac resource equates */
#include "ckmasm.h"		/* new A8 and A9 traps */
#include "ckmcon.h"		/* for fonts */


int		to_printer = FALSE;				/*JAO*/
int		to_screen = TRUE;				/*JAO*/
int		printer_is_on_line_num;				/*JAO*/
Handle	hPrintBuffer = 0L;					/*JAO*/
long	lPrintBufferSize;					/*JAO*/
long	lPrintBufferChars;					/*JAO*/
long	lPrintBufferAt;						/*JAO*/

extern	MenuHandle menus[];	/* handle on our menus */  /*JAO*/

DialogPtr	printingDialog;					/*JAO*/
DialogPtr	bufferingDialog;				/*JAO*/
DialogPtr	overflowingDialog;				/*JAO*/
DialogPtr	overflowedDialog;				/*JAO*/
DialogPtr	pauseDialog;					/*JAO*/

#define MIN(a,b) ((a)<(b))?(a):(b)										/*JAO*/
#define MAX(a,b) ((a)>(b))?(a):(b)										/*JAO*/

void
now_print()
{
    short	itemhit;												/*JAO*/
    long	lPrintBufferIndex;										/*JAO*/
    char	PrintBufferChar;										/*JAO*/
    CursHandle	watchcurs;												/*JAO*/
    int		typeOfDriver;											/*JAO*/
    int		chrExtra;												/*JAO*/
    int		leftMargin;												/*JAO*/
    int		pe;														/*JAO*/
    GrafPtr	oldPort;
    int		temp;
    TPPrPort	myPrPort;
    THPrint	PrintStuff;
    TPrStatus	myStRec;
																				/*JAO*/
	if (lPrintBufferChars >= lPrintBufferSize) {
		overflowedDialog = GetNewDialog(OVERFLOWEDBOXID, NILPTR, (WindowPtr) - 1);	/*JAO*/
		circleOK(overflowedDialog);												/*JAO*/
																				/*JAO*/
		ModalDialog (NILPROC, &itemhit);										/*JAO*/
		DisposDialog(overflowedDialog);											/*JAO*/
		if (itemhit == 2) return;	/* Cancel */												/*JAO*/
		if (itemhit == 3) {														/*JAO*/
			DisableItem(menus[PRNT_MENU], 0);									/*JAO*/
			DrawMenuBar();														/*JAO*/
			DisposHandle(hPrintBuffer);											/*JAO*/
			hPrintBuffer = 0L;													/*JAO*/
			return;																/*JAO*/
		}																		/*JAO*/
																				/*JAO*/
	}	  																		/*JAO*/
																				/*JAO*/
	PrintStuff = (THPrint)NewHandle(sizeof(TPrint));							/*JAO*/
	GetPort(&oldPort);															/*JAO*/
	PrOpen();																	/*JAO*/
	if (PrError() == noErr) {													/*JAO*/
		temp = PrValidate(PrintStuff);											/*JAO*/
		temp = PrJobDialog(PrintStuff);											/*JAO*/
		if (!temp) { 															/*JAO*/
			PrClose();															/*JAO*/
			SetPort(oldPort);													/*JAO*/
			return;																/*JAO*/
		}																		/*JAO*/
		printingDialog = GetNewDialog(PRINTINGBOXID, NILPTR, (WindowPtr) - 1);	/*JAO*/
		DrawDialog (printingDialog);											/*JAO*/
		watchcurs = GetCursor(watchCursor);										/*JAO*/
		SetCursor(*watchcurs);													/*JAO*/
																				/*JAO*/
		myPrPort = PrOpenDoc(PrintStuff, nil, nil);								/*JAO*/
		typeOfDriver = ((*PrintStuff)->prStl.wDev) >> 8;						/*JAO*/
		if (typeOfDriver == 3) { /*laser*/										/*JAO*/
			TextFont(courier);													/*JAO*/
			TextSize(10);														/*JAO*/
			chrExtra = 0;														/*JAO*/
			leftMargin = 36;													/*JAO*/
		} else {																/*JAO*/
			TextFont(VT100FONT);												/*JAO*/
			TextSize(9);														/*JAO*/
			chrExtra = 1;														/*JAO*/
			leftMargin = 36;													/*JAO*/
		}																		/*JAO*/
		printer_is_on_line_num = 1;												/*JAO*/
		lPrintBufferIndex = (lPrintBufferChars > lPrintBufferSize)				/*JAO*/
					  		? lPrintBufferAt : 0L;								/*JAO*/
		PrOpenPage(myPrPort, nil);												/*JAO*/
		MoveTo(leftMargin, 1 * 12);												/*JAO*/
		do {																	/*JAO*/
			if (PrError() != noErr) break;										/*JAO*/
			PrintBufferChar = (*hPrintBuffer)[lPrintBufferIndex];				/*JAO*/
			switch (PrintBufferChar) {											/*JAO*/
			  case 13:															/*JAO*/
				if (++printer_is_on_line_num > 60) {							/*JAO*/
					PrClosePage(myPrPort);										/*JAO*/
					PrOpenPage(myPrPort, nil);									/*JAO*/
					printer_is_on_line_num = 1;									/*JAO*/
				}																/*JAO*/
				MoveTo(leftMargin, printer_is_on_line_num * 12);				/*JAO*/
			  break;															/*JAO*/
																				/*JAO*/
			  case 14:															/*JAO*/
				if (printer_is_on_line_num != 1) {								/*JAO*/
					PrClosePage(myPrPort);										/*JAO*/
					PrOpenPage(myPrPort, nil);									/*JAO*/
					printer_is_on_line_num = 1;									/*JAO*/
				}																/*JAO*/
				MoveTo(leftMargin, printer_is_on_line_num * 12);				/*JAO*/
			  break;															/*JAO*/
																				/*JAO*/
			  default:															/*JAO*/
				DrawChar(PrintBufferChar);										/*JAO*/
				Move(chrExtra, 0);												/*JAO*/
			  break;															/*JAO*/
			}																	/*JAO*/
																				/*JAO*/
			if (++lPrintBufferIndex == lPrintBufferSize) {						/*JAO*/
				lPrintBufferIndex = 0L;											/*JAO*/
			}																	/*JAO*/
																				/*JAO*/
		} while (lPrintBufferIndex != lPrintBufferAt); 							/*JAO*/
		PrClosePage(myPrPort);													/*JAO*/
																				/*JAO*/
		PrCloseDoc(myPrPort);													/*JAO*/
		if ((PrError() == noErr) &&												/*JAO*/
			((**PrintStuff).prJob.bJDocLoop == bSpoolLoop))						/*JAO*/
			PrPicFile(PrintStuff, nil, nil, nil, &myStRec);						/*JAO*/
		if ((PrError() != noErr) && (PrError() != 128)) {						/*JAO*/
			printerr ("Printer error: ", pe);									/*JAO*/
		}																		/*JAO*/
																				/*JAO*/
		DisposDialog(printingDialog);											/*JAO*/
	}																			/*JAO*/
	PrClose();																	/*JAO*/
	SetPort(oldPort);															/*JAO*/
	InitCursor();																/*JAO*/
	DisableItem(menus[PRNT_MENU], 0);											/*JAO*/
	DrawMenuBar();																/*JAO*/
	DisposHandle(hPrintBuffer);													/*JAO*/
	hPrintBuffer = 0L;															/*JAO*/
}																				/*JAO*/

void
pr_stat()
{
    DialogPtr	printDialog;					/*JAO*/
    char	arg1[256], arg2[256], arg3[256];						/*JAO*/
    short	itemhit;												/*JAO*/


    printDialog = GetNewDialog(PRINTBOXID, NILPTR, (WindowPtr) - 1);			/*JAO*/
    circleOK(printDialog);														/*JAO*/
    																			/*JAO*/
    NumToString(MAX(lPrintBufferChars - lPrintBufferSize, 0L), arg1);			/*JAO*/
    NumToString(MIN(lPrintBufferSize, lPrintBufferChars), arg2);				/*JAO*/
    NumToString(lPrintBufferSize, arg3);										/*JAO*/
    ParamText (arg1, arg2, arg3, "");											/*JAO*/
																				/*JAO*/
    do {																		/*JAO*/
	ModalDialog (NILPROC, &itemhit);										/*JAO*/
																				/*JAO*/
	switch (itemhit) {														/*JAO*/
	  case 1:																/*JAO*/
	  case 2:																/*JAO*/
	  case 3:																/*JAO*/
	  break;															/*JAO*/
																				/*JAO*/
	}																		/*JAO*/
    } while (itemhit > 3);														/*JAO*/
	 																			/*JAO*/
    DisposDialog(printDialog);													/*JAO*/
}
