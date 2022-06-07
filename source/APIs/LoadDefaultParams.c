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
#include "otilib.h"
#include "config.h"
#include "frames.h"
#include "protocol.h"

/* Doxygen format:*/
/************************** LoadDefaultParams() **********************************************************************//**
* Activates changed settings in Saturn reader (with SET) or restores them after DO commands that changed settings temporarily.
*
* The reader holds 2 copies of the configuration parameters:
* - "Runtime" values are stored in RAM \n
*	- The DO command set the parameter value in RAM but does not change it's value in the EEPROM, so calling the LoadDefaultParams()
	  will restore the default values and overwrite the "DO" values \n
* - "Default" values are stored in EEPROM \n
*	- The SET command changes the value in the EEPROM but does not change it's value in the RAM until LoadDefaultParams() is called
*	  or until the reader is reset.
*
* @return	 OTI_STATUS				\n
| Value					|	Description																	|
|-----------------------|-------------------------------------------------------------------------------|
| OTI_OK				| Function completed successfully				 								|
| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup CFG
********************************************************************************************************************/
OTI_STATUS LoadDefaultParams(PMSG_BUFF msgBuf) //!< [in] Buffer for building message
	
{
    OTI_STATUS ret;
    RESP_MSG_STRUCT resp;

    WORD msgBufLen = msgBuf->Max_Length;

    ret = ExchangeCmd(OP_DO, (BYTE *)"\xDF\x04\x00", 3, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);
    if(ret != OTI_OK)
    {
        return ret;
    }

    if(resp.succ == NULL)   // VERY unlikely here
    {
        if(resp.fail != NULL)
        {
            return OTI_FAILED;
        }
        else
        {
            return OTI_UNSUPPORTED;
        }
    }

    return OTI_OK;
}// LoadDefaultParams()
// end
