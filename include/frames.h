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

    Description:    Type of Content of OTI HostPCD Protocol low-level frames.
    Author:         On Track Innovations Ltd.
    Version:        4.0.00

*/

#ifndef __FRAMES_H__
#define __FRAMES_H__

// Frame Format
#define MSG_STX              0x02
#define MSG_ETX              0x03

// OpCodes
#define OP_SET				 0x3C
#define OP_GET				 0x3D
#define OP_DO				 0x3E
#define OP_NAK				 0x15

// Constants
#define FIRST_BYTE_TIMEOUT	 3000   // Milliseconds
#define NEXT_BYTE_TIMEOUT	 10     // Tenth of seconds
#define ACK_TIMEOUT			 300	// Milliseconds
#define ACK_MSG_LEN			 8
#define NAK_MSG_LEN			 7
#define MIN_MSG_LEN          6


// Response buffers and lengths
typedef struct
{
	BYTE *succ;			/**< @brief Success template content */
	DWORD succLen;		/**< @brief Success template content's length */
	BYTE *fail;			/**< @brief Failure template content */
	DWORD failLen;		/**< @brief Failure template content's length */
	BYTE *unSupp;		/**< @brief Unsupported template content */
	DWORD unSuppLen;		/**< @brief Unsupported template content's length */
} RESP_MSG_STRUCT;

#endif // __FRAMES_H__
