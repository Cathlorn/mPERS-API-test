#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "str_utils.h"

int findStrMatch( char *matchString, char *valStr)
{
    int found = 0;
    int i = 0;
    int matchIndex = 0;
    int matchLength = strlen(matchString);
    int startingIndexOfMatch = -1;

    while ((i < strlen(valStr))&&(!found))
    {
        if (valStr[i] == matchString[matchIndex])
        {
            matchIndex++;
            if (matchIndex == matchLength)
            {
                found = 1;
                startingIndexOfMatch = (i - matchLength + 1);
            }
        }
        else
        {
            matchIndex = 0;
            //Checks if new pattern started
            if (valStr[i] == matchString[matchIndex])
            {
                matchIndex++;
                if (matchIndex == matchLength)
                {
                    found = 1;
                    startingIndexOfMatch = (i - matchLength + 1);
                }
            }
        }

        i++;
    }

    return startingIndexOfMatch;
}

int strMatchFound( char *matchString, char *valStr)
{
    return (findStrMatch(matchString, valStr) >= 0);
}

int isSeparator(char *separators, char c)
{
    int i;
    int separatorFound = 0;

    for(i=0; i < strlen(separators); i++)
    {
        if(separators[i] == c)
        {
            separatorFound = 1;
            break;
        }
    }

    return separatorFound;
}

///Gets arguments based on comma separation
///Return: Index of where the argument ends. Returns a negative value if nothing is found
///Copies into dstString the round argument if dstString is NOT NULL.
///Writes an empty string if nothing is found
///WARNING: Assumes dstString is big enough to fit what is written to it.
//TODO: Consider supporting a max_len argument
int getNextSubArgument(const char * argumentString, int offset, char * separators, char * dstString)
{
    int endIndex = -1;
    int i = offset;
    int argumentStringLength = 0;
    int strIndex = 0;

    assert(argumentString);

    argumentStringLength = strlen(argumentString);

    if(dstString)
        dstString[0] = '\0'; //Make string empty

    while ((i < argumentStringLength)&&(endIndex < 0))
    {
        if(isSeparator(separators, argumentString[i]))
        {
           endIndex = i;
        }
        else
        {
           if(dstString)
               dstString[strIndex++] = argumentString[i];
        }

        i++;
    }

    if(endIndex < 0)
    {
        //Update the end index if at least one character was found
        if(strIndex > 0)
        {
            //Marks index to be the end of the string
            endIndex = argumentStringLength;
        }
        else
        {
            if(dstString)
                dstString[0] = '\0'; //Make string empty
        }
    }

    return endIndex;
}

int isHexChar(char c)
{
    int isHex = 0;

    switch(c)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            isHex = 1;
        break;
    };

    return isHex;
}

int hexCharToInt(char c)
{
    int val = 0;

    switch(c)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            val = (c - '0');
        break;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            val = (c - 'A') + 10;
        break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            val = (c - 'a') + 10;
        break;
    };

    return val;
}

void readHexChar(char * hexString, unsigned char *c)
{
    if((isHexChar(hexString[0]))&&(isHexChar(hexString[1])))
    {
        char upperDigit = hexString[0];
        char lowerDigit = hexString[1];

        *c = (unsigned char)hexCharToInt(upperDigit) << 4;
        *c |= (unsigned char) hexCharToInt(lowerDigit);
    }
    else
    {
        *c = 0;
    }
}

void writeHexChar(unsigned char c, char * outputString)
{
    char upperDigit = (c>>4);
    char lowerDigit = (c&0x0f);

    upperDigit = (upperDigit < 10) ? (upperDigit + '0') : (upperDigit - 10 + 'a');
    lowerDigit = (lowerDigit < 10) ? (lowerDigit + '0') : (lowerDigit - 10 + 'a');

    outputString[0] = upperDigit;
    outputString[1] = lowerDigit;
    outputString[2] = '\0'; //NULL terminates
}

void shift_left_text_buffer(char *buf, int bufMaxLen, char *data, int len)
{
    /*    int i,j;

        //Generic Brute Force Shifting
        for(i = 0; (i < len); i++)
        {
            //Shift Every character by 1
            for(j = 0; (j < (bufMaxLen - 1)); j++)
            {
                buf[j] = buf[j+1];
            }
            //Add character at the end
            buf[(bufMaxLen - 1)] = data[i];
        } */

    //Calculated shift method
    if (len >= bufMaxLen)
    {
        memcpy(&buf[0], &data[len - bufMaxLen] , bufMaxLen);
    }
    else
    {
        memcpy(&buf[0], &buf[len] , bufMaxLen - len);
        memcpy(&buf[bufMaxLen - len], &data[0] , len);
    }
}

void shift_right_text_buffer(char *buf, int bufMaxLen, char *data, int len)
{
/*    int i,j;

    //Generic Brute Force Shifting
    for (i = (len - 1); (i >= 0); i--)
    {
        //Shift Every character by 1
        for (j = (bufMaxLen - 1); j ; j--)
        {
            buf[j] = buf[j-1];
        }
        //Add character at the end
        buf[0] = data[i];
    } */

    //Calculated shift method
    if (len >= bufMaxLen)
    {
        memcpy(&buf[0], &data[0] , bufMaxLen);
    }
    else
    {
        memcpy(&buf[len], &buf[0] , bufMaxLen - len);
        memcpy(&buf[0], &data[0] , len);
    }
}
