You can use the following code to send raw data directly to a printer in
Windows NT or Windows 95.
 
// RawDataToPrinter - sends binary data directly to a printer
//
// Params:
//   szPrinterName - NULL terminated string specifying printer name
//   lpData        - Pointer to raw data bytes
//   dwCount       - Length of lpData in bytes
//
// Returns: TRUE for success, FALSE for failure
//
BOOL RawDataToPrinter( LPSTR szPrinterName, LPBYTE lpData, DWORD dwCount )
{
  HANDLE     hPrinter;
  DOC_INFO_1 DocInfo;
  DWORD      dwJob;
  DWORD      dwBytesWritten;
 
  // Need a handle to the printer
  if( ! OpenPrinter( szPrinterName, &hPrinter, NULL ) )
    return FALSE;
 
  // Fill in the structure with info about this "document"
  DocInfo.pDocName = "My Document";
  DocInfo.pOutputFile = NULL;
  DocInfo.pDatatype = "RAW";
  // Inform the spooler the document is beginning
  if( (dwJob = StartDocPrinter( hPrinter, 1, (LPSTR)&DocInfo )) == 0 )
  {
    ClosePrinter( hPrinter );
    return FALSE;
  }
  // Start a page
  if( ! StartPagePrinter( hPrinter ) )
  {
    EndDocPrinter( hPrinter );
    ClosePrinter( hPrinter );
    return FALSE;
  }
  // Send the data to the printer
  if( ! WritePrinter( hPrinter, lpData, dwCount, &dwBytesWritten ) )
  {
    EndPagePrinter( hPrinter );
    EndDocPrinter( hPrinter );
    ClosePrinter( hPrinter );
    return FALSE;
  }
  // End the page
  if( ! EndPagePrinter( hPrinter ) )
  {
    EndDocPrinter( hPrinter );
    ClosePrinter( hPrinter );
    return FALSE;
  }
  // Inform the spooler that the document is ending
  if( ! EndDocPrinter( hPrinter ) )
  {
    ClosePrinter( hPrinter );
    return FALSE;
  }
  // Tidy up the printer handle
  ClosePrinter( hPrinter );
  // Check to see if correct number of bytes writen
  if( dwBytesWritten != dwCount )
    return FALSE;
  return TRUE;
}
