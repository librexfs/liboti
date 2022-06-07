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

FUNCTIONS:   GraphicDisplay - Asynchronous LCD display control
             GraphSetPen    - Color selection
             GraphSetX      - Set cursor position
             GraphSetY      - Set cursor position
             GraphClear     - Clear screen
             GraphNewLine   - Go to next line
             GraphSetFont   - Set text font type
             GraphLine      - Draw straight line
             GraphBox       - Draw empty box
             GraphFillBox   - Draw filled box
             GraphText      - Draw ASCII text

*********************************************************************************************************************
*/
#include <string.h>
#include <stdlib.h>
#include "protocol.h"
#include "frames.h"
#include "otilib.h"
#include "tlv.h"

/* Doxygen format:*/
/************************** GraphicDisplay() **********************************************************************//**
* Draw on the reader's LCD display.
*
* This API can only be used if OutPort control is set to Host or Host-LCD. 
* It enables the host to control the LCD display and draw directly on it – text, boxes or lines – using a list of internal commands. 
* See the other Graphic APIs to see how to build a byte array of LCD Graphics commands.
* The Graphic commands are designed to be used with a pointer on a big command buffer, such a pointer being used as an iterator. /n
* Given a pointer on the beginning of a buffer and the size of this buffer, the following APIs can be called several times, 
* passing the same address of the pointer and of the length. 
* Both arguments will be updated automatically until the command is ready, or the until buffer is full.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, resp is filled 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_TLV_ERROR			| BuilTLV failed																|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphicDisplay(	BYTE *commands,	//!< [in] A byte array containing internal Graphics commands
							WORD cmdsLen,	//!< [in] The size of the commands byte array
							PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    OTI_TLV cmdTlv;

    WORD msgBufLen = msgBuf->Max_Length;

    if(BuildTlv(0xDF32, cmdsLen, commands, NULL, 0x00,&cmdTlv))
    {
        return OTI_TLV_ERROR;
    }

    ret = ExchangeCmd(OP_DO, cmdTlv.rawTlv, cmdTlv.size, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);
    
    if(ret != OTI_OK)
    {
        LOG_DEBUG("GRAPHICS LCD CMD ERROR");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 32 01 ERR
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

    return ret;
}// GraphicDisplay()

/* Doxygen format:*/
/************************** GraphSetPen() **************************************************************************//**
* Builds a GraphicDisplay command to change pen color for drawing pixels.
*
* This API can only be used if OutPort control is set to Host or Host-LCD.
* It enables the host to control the LCD display and draw directly on it – text, boxes or lines – using a list of internal commands.
* See the other Graphic APIs to see how to build a byte array of LCD Graphics commands.
* The Graphic commands are designed to be used with a pointer on a big command buffer, such a pointer being used as an iterator. /n
* Given a pointer on the beginning of a buffer and the size of this buffer, the following APIs can be called several times,
* passing the same address of the pointer and of the length.
* Both arguments will be updated automatically until the command is ready, or the until buffer is full.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, resp is filled 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_TLV_ERROR			| BuilTLV failed																|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphSetPen(BYTE **buf, WORD *bufLen, GFX_PEN pen)
{
    if(*bufLen < 2)
        return OTI_BUFFER_OVERFLOW;

    **buf = GFX_SET_PEN;
    (*buf)++;
    **buf = pen;
    (*buf)++;
    *bufLen -= 2;

    return OTI_OK;
}

/* Doxygen format:*/
/************************** GraphSetX() ***************************************************************************//**
* Moves the cursor to the desired position along the X axis.
*
* This API is used to build a byte array of commands. \n 
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      | 
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphSetX(BYTE **buf,	/**< [in out] A pointer to a byte array where to store the command. \n
													 When the function returns, this pointer will have been
													 updated to point right after the stored command.*/
					 WORD *bufLen,	/**< [in out] The size of the commands byte array used for bounds checking. \n
													 When the function returns successfully, the size will have been 
													 decremented by the number of bytes written to the buffer.*/
					 BYTE xPos)		//!< [in] Position along the X axis where to move the pen (from 0 to 9Fh).
{
    if(*bufLen < 3)
        return OTI_BUFFER_OVERFLOW;

    if(xPos > GFX_MAX_X)
        return OTI_INVALID_PARAM;

    **buf = GFX_SET_POS;
    (*buf)++;
    **buf = 'X';
    (*buf)++;
    **buf = xPos;
    (*buf)++;
    *bufLen -= 3;

    return OTI_OK;
} // GraphSetX()

/* Doxygen format:*/
/************************** GraphSetY() ***************************************************************************//**
* Moves the cursor to the desired position along the Y axis.
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      | 
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphSetY(BYTE **buf,	/**< [in out] A pointer to a byte array where to store the command. \n
													When the function returns, this pointer will have been
													updated to point right after the stored command.*/
					 WORD *bufLen,	/**< [in out] The size of the commands byte array used for bounds checking. \n
													When the function returns successfully, the size will have been
													decremented by the number of bytes written to the buffer.*/
					 BYTE yPos)		//!< [in] Position along the Y axis where to move the pen (from 0 to 3Fh).
{
    if(*bufLen < 3)
        return OTI_BUFFER_OVERFLOW;

    if(yPos > GFX_MAX_Y)
        return OTI_INVALID_PARAM;

    **buf = GFX_SET_POS;
    (*buf)++;
    **buf = 'Y';
    (*buf)++;
    **buf = yPos;
    (*buf)++;
    *bufLen -= 3;

    return OTI_OK;
} // GraphSetY()

/* Doxygen format:*/
/************************** GraphClear() ***************************************************************************//**
* Clears the entire display screen.
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      |
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphClear(BYTE **buf,	/**< [in out] A pointer to a byte array where to store the command. \n
												  When the function returns, this pointer will have been
												  updated to point right after the stored command.*/
					  WORD *bufLen)	/**< [in out] The size of the commands byte array used for bounds checking. \n
												  When the function returns successfully, the size will have been.*/

{
    if(*bufLen < 1)
        return OTI_BUFFER_OVERFLOW;

    **buf = GFX_CLEAR;
    (*buf)++;
    *bufLen -= 1;

    return OTI_OK;
} //GraphClear()

/* Doxygen format:*/
/************************** GraphNewLine() ***************************************************************************//**
* Moves the text cursor to go to the beginning of the next line.
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      |
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphNewLine(BYTE **buf,		/**< [in out] A pointer to a byte array where to store the command. \n
													  When the function returns, this pointer will have been
													  updated to point right after the stored command.*/
						WORD *bufLen)	/**< [in out] The size of the commands byte array used for bounds checking. \n
													  When the function returns successfully, the size will have been.*/
{
    if(*bufLen < 1)
        return OTI_BUFFER_OVERFLOW;

    **buf = GFX_CRLF;
    (*buf)++;
    *bufLen -= 1;

    return OTI_OK;
}// GraphNewLine

/* Doxygen format:*/
/************************** GraphSetFont() ***************************************************************************//**
* Sets the font used for future text display.
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      |
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphSetFont(BYTE **buf,		/**< [in out] A pointer to a byte array where to store the command. \n
													  When the function returns, this pointer will have been
													  updated to point right after the stored command.*/
						WORD *bufLen,	/**< [in out] The size of the commands byte array used for bounds checking. \n
													  When the function returns successfully, the size will have been.*/
						GFX_FONT font)	/**< [in] Font value (FONT_EN_SMALL, FONT_EN_MEDIUM or FONT_EN_LARGE) */
{
    if(*bufLen < 2)
        return OTI_BUFFER_OVERFLOW;

    **buf = GFX_SET_FONT;
    (*buf)++;
    **buf = font;
    (*buf)++;
    *bufLen -= 2;

    return OTI_OK;
}//GraphSetFont()

/* Doxygen format:*/
/************************** GraphLine() **************************************************************************//**
* Draws a line on the screen. Note that it draws only straight lines (horizontal and vertical).
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      |
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphLine(BYTE **buf,	/**< [in out] A pointer to a byte array where to store the command. \n
												  When the function returns, this pointer will have been
												  updated to point right after the stored command.*/
					 WORD *bufLen,	/**< [in out] The size of the commands byte array used for bounds checking. \n
												  When the function returns successfully, the size will have been.*/
					 BYTE xFrom,	/**< [in] X coordinate for the starting point of the line.	*/
					 BYTE yFrom,	/**< [in] Y coordinate for the starting point of the line.	*/
					 BYTE xTo,		/**< [in] X coordinate for the ending point of the line.	*/
					 BYTE yTo)		/**< [in] Y coordinate for the ending point of the line.	*/
{
    if(*bufLen < 5)
        return OTI_BUFFER_OVERFLOW;

    if(xFrom > GFX_MAX_X || xTo > GFX_MAX_X || yFrom > GFX_MAX_Y || yTo > GFX_MAX_Y)
        return OTI_INVALID_PARAM;

    **buf = GFX_LINE;
    (*buf)++;
    **buf = xFrom;
    (*buf)++;
    **buf = yFrom;
    (*buf)++;
    **buf = xTo;
    (*buf)++;
    **buf = yTo;
    (*buf)++;
    *bufLen -= 5;
    return OTI_OK;
} //GraphLine()

/* Doxygen format:*/
/************************** GraphBox() **************************************************************************//**
* Draws four lines making a box, marked by the two points passed as parameters.
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      |
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphBox(BYTE **buf,		/**< [in out] A pointer to a byte array where to store the command. \n
												  When the function returns, this pointer will have been
												  updated to point right after the stored command.*/
					WORD *bufLen,	/**< [in out] The size of the commands byte array used for bounds checking. \n
												  When the function returns successfully, the size will have been.*/
					BYTE xFrom,		/**< [in] X coordinate for the starting point of the line.	*/
					BYTE yFrom,		/**< [in] Y coordinate for the starting point of the line.	*/
					BYTE xTo,		/**< [in] X coordinate for the ending point of the line.	*/
					BYTE yTo)		/**< [in] Y coordinate for the ending point of the line.	*/
{
    if(*bufLen < 5)
        return OTI_BUFFER_OVERFLOW;

    if(xFrom > GFX_MAX_X || xTo > GFX_MAX_X || yFrom > GFX_MAX_Y || yTo > GFX_MAX_Y)
        return OTI_INVALID_PARAM;

    **buf = GFX_BOX;
    (*buf)++;
    **buf = xFrom;
    (*buf)++;
    **buf = yFrom;
    (*buf)++;
    **buf = xTo;
    (*buf)++;
    **buf = yTo;
    (*buf)++;
    *bufLen -= 5;
 
    return OTI_OK;
} //GraphBox()

/* Doxygen format:*/
/************************** GraphFillBox() **************************************************************************//**
* Draws a filled box, marked by the two points passed as parameters.
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      |
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GraphFillBox(BYTE **buf,		/**< [in out] A pointer to a byte array where to store the command. \n
													  When the function returns, this pointer will have been
													  updated to point right after the stored command.*/
						WORD *bufLen,	/**< [in out] The size of the commands byte array used for bounds checking. \n
													  When the function returns successfully, the size will have been.*/
						BYTE xFrom,		/**< [in] X coordinate for the starting point of the line.	*/
						BYTE yFrom,		/**< [in] Y coordinate for the starting point of the line.	*/
						BYTE xTo,		/**< [in] X coordinate for the ending point of the line.	*/
						BYTE yTo)		/**< [in] Y coordinate for the ending point of the line.	*/
{
    if(*bufLen < 5)
        return OTI_BUFFER_OVERFLOW;

    if(xFrom > GFX_MAX_X || xTo > GFX_MAX_X || yFrom > GFX_MAX_Y || yTo > GFX_MAX_Y)
        return OTI_INVALID_PARAM;

    **buf = GFX_FILLBOX;
    (*buf)++;
    **buf = xFrom;
    (*buf)++;
    **buf = yFrom;
    (*buf)++;
    **buf = xTo;
    (*buf)++;
    **buf = yTo;
    (*buf)++;
    *bufLen -= 5;

    return OTI_OK;
}//GraphFillBox()

/* Doxygen format:*/
/************************** GraphText() **************************************************************************//**
* Displays the given text on the screen, starting from the current text cursor position and using the last set font.
*
* This API is used to build a byte array of commands. \n
* A big enough array should have been allocated beforehand and may have to be FREEd by the calling function.
*
* @return	 OTI_STATUS				\n
| Value					|	Description																	|
|-----------------------|-------------------------------------------------------------------------------|
| OTI_OK				| Function completed successfully, \n _buf_ was incremented by the command's length \n and bufLen was decremented by the number of bytes written to the buffer.      |
| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/

OTI_STATUS GraphText(	BYTE **buf,		/**< [in out] A pointer to a byte array where to store the command. \n
														When the function returns, this pointer will have been 
														updated to point right after the stored command */
						WORD *bufLen,	/**< [in out] The size of the commands byte array used for bounds checking. \n
														When the function returns successfully, the size will have been 
														updated to point right after the stored command.*/
						BYTE *txt)		/**< [in] The text to display on the screen */
{
    WORD txtLen;

    txtLen = (WORD)strlen((char *)txt);
    if(*bufLen < txtLen)
        return OTI_BUFFER_OVERFLOW;

    memcpy(*buf, txt, txtLen);
    *buf += txtLen;
    *bufLen -= txtLen;

    return OTI_OK;
}//GraphText

// end
