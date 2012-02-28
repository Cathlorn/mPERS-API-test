#ifndef POSIX_UART_H
#define POSIX_UART_H

typedef enum
{
    PROFILE_56000_8N1,
    PROFILE_115200_8N1,
}
UartProfile;

typedef struct
{
    char * deviceName;
    UartProfile modemConfiguration;
    int uartFd;
    int allowBlocking;
}
UartControl;

#define UART_CONTROL_INIT(a) {\
  .deviceName=a, \
  .modemConfiguration=PROFILE_56000_8N1, \
  .allowBlocking=1, \
  .uartFd=-1, \
} \

int configureUart(UartControl *uartControl, UartProfile uartProfile, int blocking);
int UartTx(UartControl *uartControl, const void *data, const int len);
int UartRxBytesAvailable(UartControl *uartControl);
int UartRx(UartControl *uartControl, void *data, const int max_read, int *bytesRead);

#endif //POSIX_UART_H
