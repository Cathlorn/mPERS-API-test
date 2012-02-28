#include <assert.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <poll.h>
#include <string.h>
#include <sys/ioctl.h>

#include "posix_uart.h"

//Function Prototypes

int UartTx_NonBlocking(UartControl *uartControl, const void *data, const int len);
int UartRx_NonBlocking(UartControl *uartControl, void *data, const int max_read, int *bytesRead);
int UartTx_Blocking(UartControl *uartControl, const void *data, const int len);
int UartRx_Blocking(UartControl *uartControl, void *data, const int max_read, int *bytesRead);

int getUartConfigFlags(UartProfile uartProfile)
{
    int uartConfigFlags = 0;

    switch(uartProfile)
    {
        case PROFILE_56000_8N1:
        default:
            uartConfigFlags = (B57600|CS8); //57600 bps, 8 data bits, 1 stop bit, no parity
        break;

        case PROFILE_115200_8N1:
            uartConfigFlags = (B115200|CS8); //57600 bps, 8 data bits, 1 stop bit, no parity
        break;
    };

    return uartConfigFlags;
}

int configureUart(UartControl *uartControl, UartProfile uartProfile, int blocking)
{
    int success = -1;
    int flags = (O_RDWR | O_NOCTTY);

    assert(uartControl);

    if(!uartControl->allowBlocking)
    {
        flags |= O_NONBLOCK;
    }

    uartControl->uartFd = open ( uartControl->deviceName, flags );

    if ( uartControl->uartFd >= 0 )
    {
        struct termios oldtio;
        struct termios newtio;

        uartControl->modemConfiguration = uartProfile;

        tcgetattr ( uartControl->uartFd, &oldtio ); // save current port settings

        newtio = oldtio;
        // set new port settings for canonical input processing
        newtio.c_cflag = getUartConfigFlags(uartControl->modemConfiguration) | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;       //ICANON;
        newtio.c_cc[VMIN] = 1;
        newtio.c_cc[VTIME] = 0;
        tcflush ( uartControl->uartFd, TCIFLUSH );
        tcsetattr ( uartControl->uartFd, TCSANOW, &newtio );
        success = 0;
    }

    return success;
}

int UartTx(UartControl *uartControl, const void *data, const int len)
{
    int amountSent = -1;

    assert(uartControl);

    if(uartControl->allowBlocking)
    {
        amountSent = UartTx_Blocking(uartControl, data, len);
    }
    else
    {
        amountSent = UartTx_NonBlocking(uartControl, data, len);
    }

    return amountSent;
}

int UartRx(UartControl *uartControl, void *data, const int max_read, int *bytesRead)
{
    int amountRead = -1;

    assert(uartControl);

    if(uartControl->allowBlocking)
    {
        amountRead = UartRx_Blocking(uartControl, data, max_read, bytesRead);
    }
    else
    {
        amountRead = UartRx_NonBlocking(uartControl, data, max_read, bytesRead);
    }

    return amountRead;
}

int UartTx_NonBlocking(UartControl *uartControl, const void *data, const int len)
{
    int sent = -1;

    sent = write ( uartControl->uartFd, data, len );

    return sent;
}

int UartRx_NonBlocking(UartControl *uartControl, void *data, const int max_read, int *bytesRead)
{
    int dataRead = -1;

    dataRead = read ( uartControl->uartFd, data, max_read );
    *bytesRead = dataRead;

    return dataRead;
}

int UartTx_Blocking(UartControl *uartControl, const void *data, const int len)
{
    int sent = -1;

    sent = write ( uartControl->uartFd, data, len );

    return sent;
}

int UartRx_Blocking(UartControl *uartControl, void *data, const int max_read, int *bytesRead)
{
    int dataRead = -1;

    dataRead = read ( uartControl->uartFd, data, max_read );
    *bytesRead = dataRead;

    return dataRead;
}

int UartRxBytesAvailable(UartControl *uartControl)
{
    int bytesAvailable = -1;

    if(ioctl(uartControl->uartFd, FIONREAD, &bytesAvailable) < 0) {
        perror("ioctl");
        assert(0);
    }
  return bytesAvailable;
}
