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

	Description:    Win32 implementation of COM port communication.
	Author:         On Track Innovations Ltd.
	Version:        4.0.00

*/

#include <windows.h>
#include "serial.h"
#include "oti_types.h"
#include "frames.h"
#include "config.h"

/* Global descriptor for our serial port */
static HANDLE gComHandle = INVALID_HANDLE_VALUE; // Closed

OTI_STATUS InitPort(void)
{
    DCB	current_dcb;
	COMMTIMEOUTS com_timeout;

    /* Mapping between our own baudrate/stop bits structures and windows.
     *
     * This can be adapted to other serial libraries.
     */
    DWORD rates[] = {	0,
						50,
						75,
						CBR_110,
						134,
						150,
						200,
						CBR_300,
						CBR_600,
						CBR_1200,
						1800,
						CBR_2400,
						CBR_4800,
						CBR_9600,
						CBR_19200,
						CBR_38400,
						CBR_57600,
						CBR_115200,
						230400};

	/* Check our handle, close port if it's already in use */
    if(gComHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(gComHandle);
        gComHandle = INVALID_HANDLE_VALUE;
    }

    if(!GetConfig())
        return OTI_CONFIG_ERROR;

    gComHandle = CreateFile((char *)gConf.port, 
		                    GENERIC_READ | GENERIC_WRITE, 
							0, 
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL);

    if (gComHandle == INVALID_HANDLE_VALUE)   // Couldn't open port
    {
        LOG_TRACE("OpenPort: Unable to open COM\r\n");
        return OTI_COM_ERROR;
    }

    /* Get current configuration of serial communication port. */
    if (GetCommState(gComHandle, &current_dcb) == 0) {
        return OTI_COM_ERROR;	// TODO(?): Map windows error to OTI status
    }

    /* Set our configured parameters */
    current_dcb.ByteSize = 8;
    current_dcb.StopBits = ((gConf.stop_bits == TWO_STOPBITS)?TWOSTOPBITS:ONESTOPBIT);
    current_dcb.Parity   = NOPARITY;
    current_dcb.BaudRate = rates[gConf.baud_rate];

	LOG_DEBUG("SerialComm: Set COM port parameters\r\n");

    /* Set current configuration of serial communication port. */
    if (SetCommState(gComHandle, &current_dcb) == 0) {
        return OTI_COM_ERROR;	// TODO(?): Map windows error to OTI status
    }

    /* set the time-out parameter into device control. */

	/* Specify time-out between charactor for receiving. */
    com_timeout.ReadIntervalTimeout = NEXT_BYTE_TIMEOUT * 100;	// Initial value in tenths of second

    /* Specify value that is multiplied by the requested 
	 * number of bytes to be read. 
	 */
    com_timeout.ReadTotalTimeoutMultiplier = 1;

    /* Specify value to be added to the product of the 
     * ReadTotalTimeoutMultiplier member.
	 */
    com_timeout.ReadTotalTimeoutConstant = FIRST_BYTE_TIMEOUT;

    /* Specify value that is multiplied by the requested 
	 * number of bytes to be sent. 
	 */
    com_timeout.WriteTotalTimeoutMultiplier = 0;
    
	/* Specify value to be added to the product of the 
     * WriteTotalTimeoutMultiplier member.
	 */
    com_timeout.WriteTotalTimeoutConstant = 0;

    SetCommTimeouts(gComHandle, &com_timeout);

	LOG_DEBUG("SerialComm: Set COM port timeouts\r\n");
    
    return OTI_OK;
}

OTI_STATUS Send(BYTE *data, WORD dataLen)
{
    /* Send data to COM port */
	DWORD bytes_written=0, total_bytes_written=0;

    if (data == NULL) {
        LOG_DEBUG("SerialComm: Sending NULL buffer");
        return OTI_OK;
    }

    if (dataLen == 0) {
        LOG_DEBUG("SerialComm: Sending empty buffer");
        return OTI_OK;
    }

	/* Reinitialize port if needed */
	if(gComHandle == INVALID_HANDLE_VALUE)
        if(InitPort() != OTI_OK)
            return OTI_COM_ERROR;

	/* Flush buffers before sending new data */
	PurgeComm(gComHandle, PURGE_TXCLEAR|PURGE_RXCLEAR);

    while (total_bytes_written < dataLen) 
	{
        if(WriteFile(gComHandle, data, dataLen, &bytes_written, NULL) == 0) 
		{
            return OTI_COM_ERROR;	// TODO(?): Map windows error to OTI status
        }

        total_bytes_written += bytes_written;
		data				+= bytes_written; // Move ptr forward
		dataLen				-= (WORD)bytes_written; // Compute remaining length
	}

	return OTI_OK;
}

OTI_STATUS Receive(BYTE *outBuf, WORD *outBufLen, DWORD firstCharTimeout)
{
    /* Receive data from COM port and store it in output buffer. */
	DWORD bytes_read=0, total_bytes_read=0;
	COMMTIMEOUTS com_timeout;
	BOOLEAN first = TRUE;

	/* Reinitialize port if needed */
	if(gComHandle == INVALID_HANDLE_VALUE)
        if(InitPort() != OTI_OK)
            return OTI_COM_ERROR;

	/* Change timeout if requested */
	if(firstCharTimeout && firstCharTimeout != FIRST_BYTE_TIMEOUT)
	{
		if(!GetCommTimeouts(gComHandle, &com_timeout))
			return OTI_COM_ERROR;

		com_timeout.ReadTotalTimeoutConstant = firstCharTimeout;

		if(!SetCommTimeouts(gComHandle, &com_timeout))
			return OTI_COM_ERROR;
	}

	while( total_bytes_read < *outBufLen ) 
	{
		if (ReadFile(gComHandle, outBuf+total_bytes_read, *outBufLen, &bytes_read, NULL) == 0) 
		{
			*outBufLen = (WORD)total_bytes_read;
			return OTI_COM_ERROR;	// TODO(?): Map windows error to OTI status
		}

		/* We may have asked for more than is available, it's okay except for the first read operation
		   (in this case we retry) */
		if (bytes_read == 0 && !first) 
		{
			*outBufLen = (WORD)total_bytes_read;
			return OTI_OK;
		}

		total_bytes_read += bytes_read;
		first = FALSE;
	}

	*outBufLen = (WORD)total_bytes_read;

	/* Restore default timeout if we had changed it */
	if(firstCharTimeout && firstCharTimeout != FIRST_BYTE_TIMEOUT)
	{
		if(!GetCommTimeouts(gComHandle, &com_timeout))
			return OTI_COM_ERROR;

		com_timeout.ReadTotalTimeoutConstant = FIRST_BYTE_TIMEOUT;  // Default timeout value

		if(!SetCommTimeouts(gComHandle, &com_timeout))
			return OTI_COM_ERROR;
	}

    return OTI_OK;
}

OTI_STATUS Sat_Clear_Incoming(void)
{
	PurgeComm(gComHandle, PURGE_RXCLEAR);
	return OTI_OK;
}
