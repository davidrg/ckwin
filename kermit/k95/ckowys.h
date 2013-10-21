/*  C K O W Y S . H  --  Header for Wyse Emulations */

/*
  Author: Jeffrey E Altman <jaltman@secure-endpoints.com>,
            Secure Endpoints Inc., New York City.

  Copyright (C) 1985, 2004, Trustees of Columbia University in the City of New
  York.
*/
_PROTOTYP( void wyseascii, ( int ) ) ;


/* Display Field                n  */
#define WY_FIELD_APPL           0       /* Application Display Area   */
#define WY_FIELD_FNKEY          1       /* Function Key Labeling Line */
#define WY_FIELD_LOCAL_MSG      2       /* Local Message Field        */
#define WY_FIELD_HOST_MSG       3       /* Host Message Field         */

/* End of Block Terminator  */
#define EOB_US_CR               0       /* Lines end with US; Blocks with CR */
#define EOB_CRLF_ETX            1       /* Lines end with CRLF; Blocks with ETX */
