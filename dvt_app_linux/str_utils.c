#include <assert.h>
#include <string.h>

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

