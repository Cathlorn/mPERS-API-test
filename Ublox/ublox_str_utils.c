#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "General/str_utils.h"
#include "ublox_str_utils.h"

int decodeUbloxHexString(char *hexString, int len, unsigned char *data, int max_len)
{
    int bytesInterpreted;
    int i;

    for(i = 0; (i < max_len)&&(i < (len/2)); i++)
    {
        readHexChar(&hexString[2*i], &data[i]);
    }
    bytesInterpreted = i;

    return bytesInterpreted;
}

void createUbloxHexString(unsigned char *data, int len, char *outputString, int outputStringMaxLen)
{
    //char hexCharString[10] = "";
    int i;
    int outputStringOffset = 0;

    for (i=0; i < len; i++)
    {
        //sprintf(hexCharString,"%x", data[i]);
        //strncat(outputString, hexCharString, outputStringMaxLen);
        //writeHexChar(data[i], hexCharString);
        writeHexChar(data[i], &outputString[outputStringOffset]);
        outputStringOffset += 2;
        outputString[outputStringOffset] = '\0';
    }
}

//Assumes IP is stored in big endian
int ParseIPString(char *ipString)
{
    int len = strlen(ipString);
    int i=0;
    char digitString[100];
    int digitLen = 0;
    char c='\0';
    int ipPosition = 0;
    int ipValue = 0;

    for (i = 0; i < len; i++)
    {
        c = ipString[i];
        if (c == '.')
        {
            if (digitLen > 0)
            {
                unsigned int num;

                if (ipPosition < 4)
                {
                    num = atoi(digitString);
                    ipValue |= (num << ((3 - ipPosition)*8));
                    ipPosition++;
                }

                digitLen = 0;
            }
        }
        else if ((c >= '0')&&(c <= '9')) //Only process decimal digits
        {
            digitString[digitLen++] = c;
        }
    }

    if (digitLen > 0)
    {
        unsigned int num;

        if (ipPosition < 4)
        {
            num = atoi(digitString);
            ipValue |= (num << ((3 - ipPosition)*8));
            ipPosition++;
        }

        digitLen = 0;
    }

    if (ipPosition < 4)
    {
        ipValue = 0; //Default to this if incomplete IP is received
    }

    return ipValue;
}

//Assumes big endian
void GetIpString(int ip, char *outputString, int maxOutputStringLen)
{
    int digits[4];

    digits[0] = (ip >> 24) & 0xff;
    digits[1] = (ip >> 16) & 0xff;
    digits[2] = (ip >>  8) & 0xff;
    digits[3] = (ip      ) & 0xff;
    sprintf(outputString, "%d.%d.%d.%d", digits[0], digits[1], digits[2], digits[3]);
}
