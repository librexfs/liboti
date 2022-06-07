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

    Description:    Definitions for RestoreFactorySettings command.
    Author:         On Track Innovations Ltd.
	Version:        4.0.00

*/
#ifndef __RESTOREFACTORY_H__
#define __RESTOREFACTORY_H__

/** @brief Builds a Restore Factory Settings command into a buffer.
 *
 *	As Restore Factory Settings is actually a very simple and short command, 
 *  this function is only marginally useful, but it's provided for consistency.
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
 *	@returns	This function returns OTI_OK if the command was properly built
 *				into the given (or dynamically allocated) buffer.
 *
 *				If the provided buffer was too small, OTI_BUFFER_OVERFLOW is
 *				returned.
 *
 *				If no correct buffer pointer was given, or if a dynamic 
 *				buffer couldn't be allocated, OTI_MEMORY_ERROR is returned.
 */
OTI_STATUS cmdRestoreFactorySettings(BYTE **buf, WORD *bufLen);

/** @brief Sends a Restore Factory Settings command to the reader.
 *
 *	The reader will reset all its internal settings to factory. On later
 *  of the protocol, the baudrate and LCD messages won't be changed by
 *  this command.
 *
 *  Note that this function takes no parameters at all, as this command can 
 *  only be used with the DO opcode and has no arguments.
 *
 *	@returns	This function returns OTI_OK if the command was succesful. 
 *				Otherwise it returns an error code.
 */
OTI_STATUS RestoreFactorySettings(BYTE *msgBuf, WORD *msgBufLen);

#endif // __RESTOREFACTORY_H__
