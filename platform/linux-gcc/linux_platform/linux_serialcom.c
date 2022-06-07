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

    Description:    POSIX implementation of COM port communication.
    Author:         On Track Innovations Ltd.
	Version:        4.0.00

*/

#include <stdlib.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/time.h>   /* For timeouts */
#include <sys/types.h>
#include "serial.h"
#include "frames.h"
#include "otilib.h"
#include "config.h"

/* Global descriptor for our serial port */
int gSerialFd = 0;        // Closed

OTI_STATUS InitPort(void)
{
    struct termios tio;
    /* Mapping between our own baudrate/stop bits structures and termios.
     *
     * This can be adapted to other serial libraries.
     */
    speed_t rates[] = { B0,
                        B50,
                        B75,
                        B110,
                        B134,
                        B150,
                        B200,
                        B300,
                        B600,
                        B1200,
                        B1800,
                        B2400,
                        B4800,
                        B9600,
                        B19200,
                        B38400,
                        B57600,
                        B115200,
                        B230400};

    /* Check our descriptor */
    if(gSerialFd > 0)
    {
        close(gSerialFd);
        gSerialFd = 0;
    }

    if(!GetConfig())
        return OTI_CONFIG_ERROR;

    gSerialFd = open((char *)gConf.port, O_RDWR | O_NOCTTY);

    if (gSerialFd == -1)   // Couldn't open port
    {
//        perror("OpenPort: Unable to open tty");
        return OTI_COM_ERROR;
    }

    fcntl(gSerialFd, F_SETFL, 0);  // blocking mode

    /* Setup port parameters */
    bzero(&tio, sizeof(tio));
    cfsetspeed(&tio, rates[gConf.baud_rate]);
    tio.c_cflag     |= CS8 | CLOCAL | CREAD;

    /* Handle stopbits */
    if(gConf.stop_bits == TWO_STOPBITS)
        tio.c_cflag |= CSTOPB;

    tio.c_iflag     |= IGNPAR;
    //tio.c_oflag     = 0;
    //tio.c_lflag     = 0;

    tio.c_cc[VTIME] = NEXT_BYTE_TIMEOUT;   /* timeout for next byte read */
    tio.c_cc[VMIN]  = 0;                   /* don't block for first character -- we'll use select() to make sure reading is safe */

    tcflush(gSerialFd, TCIFLUSH);
    tcflush(gSerialFd, TCOFLUSH);
    tcsetattr(gSerialFd, TCSANOW, &tio);

    return OTI_OK;
}

OTI_STATUS Send(BYTE *data, WORD dataLen)
{
    int nSent = 0;
    int ready = 0;
    int retries = 0;
    struct timeval selTimeout;
    fd_set fdSet;
    WORD rLen;
    BYTE *ptr;


    if(gSerialFd <= 0)
        if(!InitPort())
            return OTI_COM_ERROR;

    tcflush(gSerialFd, TCIFLUSH);
    tcflush(gSerialFd, TCOFLUSH);

    FD_ZERO(&fdSet);
    FD_SET(gSerialFd, &fdSet);

    /* Arbitrarily set output maximum delay to 1 second */
    selTimeout.tv_sec = 1;
    selTimeout.tv_usec = 0;

    /* CDC-ACM driver doesn't do buffering, so we need to make sure every chunk
     * of 64 bytes we send is on its way before we send the next, or some data
     * may get lost when using USB readers.
     */
    rLen = dataLen;
    ptr  = data;
    while((nSent >= 0) && (rLen > 0) && (retries < 3))
    {
        /* Check that it's safe to write to COM port now */
        ready = select(gSerialFd + 1, NULL, &fdSet, NULL, &selTimeout);
        if(!ready)
        {
            retries++;
            continue;
        }

        if(rLen >= 64)
        {
            nSent = write(gSerialFd, ptr, 64);
            rLen -= 64;
            ptr  += 64;
        }
        else
        {
            nSent = write(gSerialFd, ptr, rLen);
            rLen = 0;
        }
        retries = 0;
    }

    if (nSent < 0)
    {
//        printf("write() of %d bytes failed!\n", dataLen);
        return OTI_COM_ERROR;
    }

    if(retries == 3)
    {
//        printf("Send() failed, port not ready (%d/%d bytes sent)\n",
//               dataLen - rLen, dataLen);
        return OTI_COM_TIMEOUT;
    }

    return OTI_OK;
}

OTI_STATUS Receive(BYTE *outBuf, WORD *outBufLen, DWORD firstCharTimeout)
{
    int ready = 0;
    int dataLen = 0;
    struct timeval selTimeout;
    unsigned char *ptr;
    WORD maxLen = *outBufLen;   // Save initial maximum size of output buffer
    fd_set fdSet;
    int secs;
    int msecs;

    if(gSerialFd <= 0)
        if(!InitPort())
            return OTI_COM_ERROR;

    /* Wait firstCharTimeout to see if we'll get data at all */
    if(!firstCharTimeout)
        firstCharTimeout = FIRST_BYTE_TIMEOUT;  // Default timeout value

    /* We may have more than 1 second timeout, handle it */
    secs  = firstCharTimeout / 1000;
    msecs = firstCharTimeout % 1000;

    //printf("secs: %d, msecs: %d\n", secs, msecs);

    selTimeout.tv_sec = secs;
    selTimeout.tv_usec = msecs * 1000;  // structure takes microseconds and we give millisecond

    FD_ZERO(&fdSet);
    FD_SET(gSerialFd, &fdSet); /* We only have one source to test */

    ptr = outBuf;
    *outBufLen = 0;

    /* We use select() here to act as a timeout before the first byte is read (if available). If select returns without a
     * timeout, then it's safe to read from our descriptor until it doesn't return anymore data.
     */
    ready = select(gSerialFd + 1, &fdSet, NULL, NULL, &selTimeout);
    if(!ready)
        return OTI_COM_TIMEOUT; // Timeout error

    /* Data's ready to read but it can still come in several chunks, so loop until the read buffer is really empty. */
    do
    {
        dataLen = read(gSerialFd, ptr, maxLen - *outBufLen);
        *outBufLen += dataLen;
        ptr += dataLen;

        /* We may need a little time to receive more */
        //if((dataLen > 0) && (ptr < (outBuf + maxLen)))
           // usleep(NEXT_BYTE_TIMEOUT * 100000);
        //printf("dataLen: %d\n", dataLen);
    }
    while((dataLen > 0) && (ptr < (outBuf + maxLen)));

    return OTI_OK;
}

OTI_STATUS Sat_Clear_Incoming(void)
{
	// Not implemented yet.
	return OTI_OK;
}

void OtiClosePort(void)
{
    /* Check our descriptor */
    if(gSerialFd > 0)
    {
        close(gSerialFd);
        gSerialFd = 0;
    }
}
