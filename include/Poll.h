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

    Description:    Definitions for Poll command.
    Author:         On Track Innovations Ltd.
	Version:        4.0.00

*/
#ifndef __POLL_H__
#define __POLL_H__

#include <string.h>
#include <stdlib.h>
#include "otilib.h"
#include "protocol.h"
#include "tlv.h"
#include "oti_types.h"

/** @brief Polling modes available for non-EMV PICC */
typedef enum
{
	POLLING_CONTINUOUSLY = 0x01,
	POLLING_ONCE,
	ACTIVATE_ONCE,
	ACTIVATE_IMMEDIATELY
} POLL_MODE;

/** @brief Properties available in Poll response structure */
#define	POLL_PICC_TYPE	0x10000000

/** @brief Response data to non-EMV polling */
typedef struct
{
	DWORD    properties;
	BYTE	 type;
	BYTE	*track1_data;
	WORD	 track1_len;
	BYTE	*track2_data;
	WORD	 track2_len;

} POLL_RESP_STRUCT;

/** @brief Builds a Poll command into a buffer.
 *
 *  If no buffer is provided, one will be dynamically allocated. In which
 *	case, the calling function is responsible for calling free() on the
 *  returned pointer.
 *
 *	@param buf		[IN/OUT] Buffer into which Poll command will be built. It
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
 *	@param opcode	[IN] Opcode used for the command (OP_GET, OP_SET or OP_DO).
 *
 *	@param mode		[IN] Polling mode required. See the POLL_MODE structure
 *					for possible values. This parameter is not used if _opcode_
 *					is OP_GET. In that case, its value can be set to 0.
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
OTI_STATUS cmdPoll(PMSG_BUFF buf, BYTE opcode, POLL_MODE mode);

/** @brief Sends a Poll (for a non EMV PICC) command to the reader.
 *
 *  The reader will display 'Present Card' and wait for a contactless
 *  card to enter its RF field. Then, depending on the enabled applications,
 *  the reader will either activate the card or perform a full MSD transaction
 *	and store the results in a response structure.
 *
 *	This function can also be used to retrieve the default polling mode.
 *
 *	@param opcode		[IN] Opcode used for the command (OP_GET, OP_SET or 
 *						OP_DO).
 *
 *	@param mode			[IN/OUT] Pointer to a variable either containing the 
 *						polling mode required, or in which the retrieved mode
 *						will be stored if _opcode_ is OP_GET. See the POLL_MODE 
 *						enum for possible values.
 *
 *	@param poll_resp	[OUT] Pointer to a POLL_RESP_STRUCT structure in which
 *						the results of polling (and, if applicable, Magstripe
 *						transaction) will be stored.
 *
 *	@returns	This function returns OTI_OK if the command was successful.
 *
 *				If no card was presented to the reader within the Polling
 *				timeout, OTI_COM_TIMEOUT is returned.
 */
OTI_STATUS Poll(BYTE opcode, POLL_MODE *mode, POLL_RESP_STRUCT *poll_resp, PMSG_BUFF msgBuf);

/** @brief Extracts transaction data from a response to Poll command.
 *
 */
OTI_STATUS parsePollResp(PMSG_BUFF msgBuf, POLL_RESP_STRUCT *resp);

#endif // __POLL_H__
