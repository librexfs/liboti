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

#include <string.h>
#include <stdlib.h>
#include "otilib.h"
#include "protocol.h"

/************************** SetDoExternalDisplay() *****************************************************************//**
* * Configure the Redaer to send its preset LCD messages to the terminal.
*
* This API enables the use of external display module.
* This command enables terminals using the PCD as an OEM (Without Display) to configure the PCD to send its 
* Preset LCD messages to the terminal through its serial ports (Standard UART or USB).
* The API can change the settings in RAM (using DO opcode) or in the EEPROM (using SET opcode)
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, resp is filled 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
* 
* \ingroup INTERNAL
********************************************************************************************************/
OTI_STATUS SetDoExternalDisplay(BYTE opcode, EXT_DISPLAY_MODE mode, 
								PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE extDisplayCmd[] = {0xDF, 0x46, 0x01, 0x00};

    WORD msgBufLen = msgBuf->Max_Length;

    /* Build 'External Display' message */
    extDisplayCmd[3] = (BYTE)mode;
    ret = ExchangeCmd(opcode, extDisplayCmd, 4, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("SetDoExternalDisplay CMD ERROR");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 46 01 ERR
                ret = MapErrorToStatus(resp.fail[3]);
            else
                ret = OTI_FAILED;

            return ret;
        }
        else
        {
            return OTI_UNSUPPORTED;
        }
    }

    return OTI_OK;

} // SetDoExternalDisplay()

/************************** SetDoExternalDisplay() *****************************************************************//**
* Configure the Redaer to send its preset LCD messages to the terminal.
*
* This command enables terminals using the PCD as an OEM (Without Display) to configure the PCD to send its 
* Preset LCD messages to the terminal through its serial ports (Standard UART or USB).
* The API can change the configuration in RAM (using DO opcode), so it will go back to default after reset
* or after calling LoadDefaultparams() API.
*
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, resp is filled 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
* 
* \ingroup UI
********************************************************************************************************/
OTI_STATUS DoExternalDisplay(EXT_DISPLAY_MODE mode,
							PMSG_BUFF msgBuf) //!< [in] Buffer for building message

{
    return SetDoExternalDisplay(OP_DO, mode, msgBuf);
}
