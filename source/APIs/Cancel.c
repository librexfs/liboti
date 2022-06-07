/********************************************************************************************************************

                         OTI HostPCD Protocol C Library

          **********************************************************************
          **        Copyright (c) 2008-2015 On Track Innovations Ltd.         **
          ** This computer program includes confidential, proprietary         **
          ** information and is a trade secret of On Track Innovations Ltd.   **
          ** All use, disclosure, and/or reproduction is prohibited           **
          ** unless expressly authorized in writing.                          **
          **                    All rights reserved.                          **
          **********************************************************************

          PURPOSE:     This file contains a library function.

          PROGRAMMER:  On Track Innovations Ltd.

          REVISIONS:   4.0.00
*********************************************************************************************************************
*/
#include <stdlib.h>
#include "protocol.h"
#include "frames.h"
#include "otilib.h"
#include "tlv.h"
#include "config.h"
#include "serial.h"

/* Doxygen format:*/
/************************** CancelTransaction() *******************************************************************//**
* Makes the reader cancel the transaction processing and return to idle state.
* 
* This function is used from another task and doing as little as possible, so as not to take ressources of the main task. \n 
* The main task will receive the response to the Stop Macro cmd. See more details in
* "oti Host-PCD Communication Protocol" - Stop Macro Tag (DF 7D). \n
* Note that this command may not stop the transaction if it has already started, in such case the reader will response  
* to the Poll command.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_COM_ERROR	    	| Communication error with Saturn reader			 							|

* 
* \ingroup EMV
********************************************************************************************************************/

OTI_STATUS CancelTransaction(void)
{
    if(!Send((BYTE *)"\x02\x09\x00\x3E\xDF\x7D\x00\x97\x03", 9))
        return OTI_COM_ERROR;

	return OTI_OK;
}// CancelTransaction()

// end
