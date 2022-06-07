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

/* Doxygen format:*/
/************************** SelectLCDMessage() *******************************************************************//**
* Select a message to display on the redaer display.
*
* This command is a way for the POS terminal to tell the reader to display message number (Message Index) 
* from the reader message storage. The message can be configured by the StoreLCDMessage() command. \n
* For LEDs and beeping, see the SetOutPortControl() command. \n
* This API requires the reader’s OutPortControl mode to be set to HOST or HOST_LCD.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS SelectLCDMessage(LCD_MESSAGE *index,			/**< [in out] LCD default Message index to select, change,
																	  or that was retrieved from the reader */
							LCDMSG_SELECT_ACT action,	/**< [in] DISPLAY, GET_DEFAULT or CHANGE_DEFAULT */
							PMSG_BUFF msgBuf) //!< [in] Buffer for building message

{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE lcdSelCmd[] = {0xDF, 0x2C, 0x00, 0x00};
    BYTE cmdLen;
    BYTE ops[] = {OP_DO, OP_SET, OP_GET};

    WORD msgBufLen = msgBuf->Max_Length;

    // Build 'Select LCD' message 
    switch(action)
    {
        case LCD_DISPLAY:
        case LCD_CHANGE_DEFAULT:
            lcdSelCmd[2] = 0x01;
            lcdSelCmd[3] = *index;
            cmdLen = 4;
            break;

        case LCD_GET_DEFAULT:
            cmdLen = 3;
            break;

        default:
            return OTI_INVALID_PARAM;
    }

    ret = ExchangeCmd(ops[action], lcdSelCmd, cmdLen, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("SELECT LCDMSG CMD ERROR");
        return ret;
    }

    if(resp.fail != NULL)
    {
        return OTI_FAILED;
    }
    else if(resp.unSupp != NULL)
    {
        return OTI_UNSUPPORTED;
    }

    if(action == LCD_GET_DEFAULT)
    {
        if(resp.succLen != 4)
        {
            return OTI_INVALID_DATA;
        }
        *index = (LCD_MESSAGE)resp.succ[3];      // Response should be DF 2C 01 IDX
    }

    if(action == LCD_CHANGE_DEFAULT)
    {
        msgBufLen = PCD_MAX_BUFFER_SIZE;
        ret = ExchangeCmd(OP_DO, lcdSelCmd, cmdLen, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);
        if(ret != OTI_OK)
        {
            return ret;
        }
    }

    return ret;
}// SelectLCDMessage()
// end
