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
#include "protocol.h"
#include "frames.h"
#include "otilib.h"

/* Doxygen format:*/
/************************** StoreChangeLCDMessage() ****************************************************************//**
* Tell the reader to modify message number # in the reader memory.
*
* This command is a way for a POS terminal to configure the LCD display message in ASCII format 
* and store the configuration data into the reader memory:
* - EEPROM memory using SET opcode.  
* - RAM memory using DO opcode.   
* 
* These new messages will be displayed then, either automatically by the reader, or explicitly through the SelectLCDMessage API. \n
* As the change should be immediate, it’s possible to display a custom message on the display by using this API 
* with the current displayed message’s index – which is usually IDLE_MSG. \n
* This API requires the reader’s OutPortControl mode to be set to HOST or HOST_LCD.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup INTERNAL
********************************************************************************************************************/
OTI_STATUS StoreChangeLCDMessage(	BYTE opcode,					 /**< [in] OPCODE: DO or SET  */
									LCDMSG_SETTINGS_STRUCT *message, /**< [in] LCD default Message index to select */
									PMSG_BUFF cmdBuf, //!< [in] Message to store
									PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE lcdCmd[] = {0xDF, 0x54, 0x00, 0x00, 0x00, 0x00};
    BYTE *bufPtr;
    BYTE yCtr;
    BYTE sLen[] = {0x00, 0x00, 0x00};

    /* Safety measure to avoid segfaults in case of incorrect (non NULL-terminated) string */
    message->L1[LCD_MAXLGTH] = 0x00;
    message->L2[LCD_MAXLGTH] = 0x00;
    message->L3[LCD_MAXLGTH] = 0x00;
    WORD cmdBufLen = 0;

    /* We can figure out exact command length: one to three text lines (VAR bytes each) with fixed
     * formatting information (6 bytes each).
     */
    if(message->setLines & LCD_LINE1)
    {
        sLen[0] = strlen((char *)message->L1);      // Compute string lenghts once and for all
        cmdBufLen += 6 + sLen[0];
    }

    if(message->setLines & LCD_LINE2)
    {
        sLen[1] = strlen((char *)message->L2);
        cmdBufLen += 6 + sLen[1];
    }

    if(message->setLines & LCD_LINE3)
    {    
        sLen[2] = strlen((char *)message->L3);
        cmdBufLen += 6 + sLen[2];
    }
    
    WORD msgBufLen = msgBuf->Max_Length;
    
    if (cmdBuf->Max_Length < cmdBufLen)
    {
        cmdBuf->Max_Length = LCD_MAXLGTH + 6;
        msgBuf->Max_Length = PCD_MAX_BUFFER_SIZE;
        return OTI_BUFFER_LENGTH;
    }


    /* Build 'Store LCD' message */
    bufPtr = cmdBuf->Data;
    if(message->yCenter)
        yCtr = 0x80;
    else
        yCtr = 0x00;

    /* Line 1 */
    if(message->setLines & LCD_LINE1)
    {
        lcdCmd[2] = sLen[0] + 3;
        lcdCmd[3] = message->index;
        lcdCmd[4] = 0x01;
        lcdCmd[5] = message->format[0] | yCtr;
        memcpy(bufPtr, lcdCmd, sizeof(lcdCmd));
        bufPtr += sizeof(lcdCmd);
        memcpy(bufPtr, message->L1, sLen[0]);
        bufPtr += sLen[0];
    }

    /* Line 2 */

    if(message->setLines & LCD_LINE2)
    {
        lcdCmd[2] = sLen[1] + 3;
        lcdCmd[3] = message->index;
        lcdCmd[4] = 0x02;
        lcdCmd[5] = message->format[1] | yCtr;
        memcpy(bufPtr, lcdCmd, sizeof(lcdCmd));
        bufPtr += sizeof(lcdCmd);
        memcpy(bufPtr, message->L2, sLen[1]);
        bufPtr += sLen[1];
    }
    
    /* Line 3 */
    if(message->setLines & LCD_LINE3)
    {
        lcdCmd[2] = sLen[2] + 3;
        lcdCmd[3] = message->index;
        lcdCmd[4] = 0x03;
        lcdCmd[5] = message->format[2] | yCtr;
        memcpy(bufPtr, lcdCmd, sizeof(lcdCmd));
        bufPtr += sizeof(lcdCmd);
        memcpy(bufPtr, message->L3, sLen[2]);
        bufPtr += sLen[2];
    }

    ret = ExchangeCmd(opcode, cmdBuf->Data, cmdBufLen, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);
    
    if(ret != OTI_OK)
    {
        LOG_DEBUG("STORE LCDMSG CMD ERROR");
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

    /* Activate new configuration immediately is needed */
    if(opcode == OP_SET)
        ret = LoadDefaultParams(msgBuf);
    

    if(ret != OTI_OK)
    {
        LOG_DEBUG("ACTIVATE STORE LCDMSG CMD ERROR");
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

    return ret;
}// StoreChangeLCDMessage()
// end

/* Doxygen format:*/
/************************** StoreLCDMessage() ****************************************************************//**
* Tell the reader to modify message number # in the reader EEPROM memory.
*
* This command is a way for a POS terminal to configure the LCD display message in ASCII format 
* and store the configuration data into the reader EEPROM memory (using SET opcode). 
* It also loads factory settings in order to ctivate the new configuration immediately.
* 
* These new messages will be displayed then, either automatically by the reader, or explicitly through the SelectLCDMessage API. \n
* As the change should be immediate, it’s possible to display a custom message on the display by using this API 
* with the current displayed message’s index – which is usually IDLE_MSG. \n
* This API requires the reader’s OutPortControl mode to be set to HOST or HOST_LCD, see SetOutPortControl() method.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS StoreLCDMessage(LCDMSG_SETTINGS_STRUCT *message, /**< [in] LCD default Message index to select */
	PMSG_BUFF cmdBuf, //!< [in] Message to store
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    return StoreChangeLCDMessage(OP_SET, message, cmdBuf, msgBuf);
}

/* Doxygen format:*/
/************************** ChangeLCDMessage() ****************************************************************//**
* Tell the reader to modify message number # in the reader RAM.
*
* This command is a way for a POS terminal to configure the LCD display message in ASCII format 
* and store the configuration data into the reader RAM (using DO opcode)
* 
* These new messages will be displayed then, either automatically by the reader, or explicitly through the SelectLCDMessage API. \n
* As the change should be immediate, it’s possible to display a custom message on the display by using this API 
* with the current displayed message’s index – which is usually IDLE_MSG. \n
* This API requires the reader’s OutPortControl mode to be set to HOST or HOST_LCD, see SetOutPortControl() method.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS ChangeLCDMessage(LCDMSG_SETTINGS_STRUCT *message, /**< [in] LCD default Message index to select */
	PMSG_BUFF cmdBuf, //!< [in] Message to store
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    return StoreChangeLCDMessage(OP_DO,  message, cmdBuf, msgBuf);
}
