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

	Description:    Functions for low-level COM port handling.
	Author:         On Track Innovations Ltd.
	Version:        4.0.00
    
	Notes:          This file contains three generic APIs that must be
					re-implemented for each platform (Posix, Win32, ...)
					if there is no implementation in the platform/ folder.

*/
#ifndef __SERIAL_H__
#define __SERIAL_H__            // this avoids multiple inclusions

#include "otilib.h"
#include "frames.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief	Initializes COM port with the content of configuration structure. 
 *			This function may be a no-op on some systems. Any port-setting 
 *			code such as port choosing, speed setting and so on should be 
 *			put in this function.
 *
 *			The legacy behavior is to set global configuration with GetConfig
 *			and UpdateConfigFile, and then to call this function to open a
 *			COM port and provide a global variable with the port handle for
 *			the Send and Receive functions to use.
 *
 *			See the implemented SerialComm.c file in platform/linux-gcc/src
 *			for more details.
 *
 * @return	This function should return OTI_OK if the COM port could be
 *			initialized properly, or an appropriate error otherwise, such as 
 *			OTI_COM_ERROR or OTI_CONFIG_ERROR.
 */
OTI_STATUS InitPort(void);

/**
 * @brief	Sends data for Saturn device to the initialized COM port. 
 *
 * @param   data     [IN] Pointer to a BYTE array containing the data to send
 *                   to the COM port.
 * @param   dataLen  [IN] Length of the data BYTE array to send.
 *
 * @return	This function should return OTI_OK if writing to the COM port
 *			was successful, or an appropriate error otherwise, such as
 *			OTI_COM_ERROR or OTI_COM_TIMEOUT.
 */
OTI_STATUS Send(BYTE *data, WORD dataLen);

/**
 * @brief	Receives data from Saturn device through the initialized COM port. 
 *
 * @param   outBuf			  [OUT] Pointer to a BYTE array in which received
 *							  data will be stored.
 * @param   outBufLen		  [IN/OUT] Length of the _outBuf_ BYTE array. When
 *							  the function returns, this variable holds the
 *							  actual size of the data that was read.
 * @param   firstCharTimeout  [IN] Delay allowed before the first byte from the
 *							  reader is received (in milliseconds).
 *
 * @return	This function should return OTI_OK if reading from the COM port
 *			was successful, or an appropriate error otherwise, such as
 *			OTI_COM_ERROR or OTI_COM_TIMEOUT.
 */
OTI_STATUS Receive(BYTE *outBuf, WORD *outBufLen, DWORD firstCharTimeout);

OTI_STATUS Sat_Clear_Incoming(void);

/**
 * @brief	Closing a Serial Port.
 */
void OtiClosePort(void);

#ifdef  __cplusplus
}
#endif

#endif // __SERIAL_H__
