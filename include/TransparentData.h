/**
                         OTI HostPCD Protocol C Library

          **********************************************************************
          **        Copyright (c) 2008-2015 On Track Innovations Ltd.         **
          ** This computer program includes confidential, proprietary         **
          ** information and is a trade secret of On Track Innovations Ltd.   **
          ** All use, disclosure, and/or reproduction is prohibited           **
          ** unless expressly authorized in writing.                          **
          **                    All rights reserved.                          **
          **********************************************************************

    Description:    Definitions for TransparentData command.
    Author:         On Track Innovations Ltd.
	Version:        2.0.00

*/
#ifndef __TRANSPARENTDATA_H__
#define __TRANSPARENTDATA_H__

#include "otilib.h"

/** @brief Builds a Transparent Data command into a buffer.
 *
 *  If no buffer is provided, one will be dynamically allocated. In which
 *	case, the calling function is responsible for calling free() on the
 *  returned pointer.
 *
 *	@param buf		[IN/OUT] Buffer into which the command will be built. It
 *					must either be the address of a pointer to an existing
 *					array, or the address of a NULL-initialized pointer. In 
 *					that last case,	this parameter will be set to the address
 *					of a dynamically allocated array containing the final 
 *					command.
 *
 *	@param bufLen	[IN/OUT] Pointer to the size of the provided buffer. This
 *					parameter can be NULL if _buf_ points to a NULL pointer.
 *					After the function returns, this parameter is set to the
 *					actual, final length of buffer _buf_.
 *
 *	@param data		[IN] Transparent data to send to the card, as a pointer to
 *					a byte array.
 *
 *	@param dataLen	[IN] Length of the _data_ byte array.
 *
 *	@returns	This function returns OTI_OK if the command was properly built
 *				into the given (or dynamically allocated) buffer.
 *
 *				If the provided buffer was too small, OTI_BUFFER_OVERFLOW is
 *				returned.
 *
 *				If no correct buffer pointer was given, or if a dynamic 
 *				buffer couldn't be allocated, OTI_MEMORY_ERROR is returned.
 */
OTI_STATUS cmdTransparentData(BYTE **buf, WORD *bufLen, BYTE *data, WORD dataLen);

/** @brief Sends a Transparent Data command to the reader.
 *
 *  This command transmits data directly to a card in the reader's RF field.
 *	It must be sent after a successful Poll command. The card's reply will
 *	be embedded in this command's response.
 *
 *	@param data			[IN] Transparent data to send to the card, as a pointer
 *						to a byte array.
 *
 *	@param dataLen		[IN] Length of the _data_ byte array.
 *
 *	@param respBuf		[IN/OUT] Buffer into which the command's response will
 *						be stored. It must either be the address of a pointer
 *						to an existing array, or the address of a 
 *						NULL-initialized pointer. In that last case, this
 *						parameter will be set to the address of a dynamically
 *						allocated array containing the response.
 *
 *	@param respBufLen	[IN/OUT] Pointer to the size of the provided buffer.
 *						This parameter can be 0 if _respBuf_ points to a NULL
 *						pointer. After the function returns, this parameter is
 *						set to the actual, final length of buffer _respBuf_.
 *
 *	@returns	This function returns OTI_OK if the command was successful.
 *				Otherwise it returns an error code.
 */
OTI_STATUS TransparentData(BYTE *data, WORD dataLen, BYTE **respBuf, WORD *respBufLen);

#endif // __TRANSPARENTDATA_H__
