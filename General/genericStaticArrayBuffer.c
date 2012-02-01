//2/1/2012
//CLS (Halo Monitoring)
//Generic Buffer pool that returns points to different statically allocated segments of memory

#include <assert.h>

#include "types.h"
#include "genericStaticArrayBuffer.h"

int initBufferTable(BufferTable *pBufferTable, void *pEntries, int maxNumberOfEntries)
{
    int success = 1;

    assert(pBufferTable);

    pBufferTable->entries = pEntries;
    pBufferTable->maxNumberOfEntries = maxNumberOfEntries;

    return success;
}

int assignData(BufferTable *pBufferTable, unsigned int tableIndex, void *pData, int dataSize)
{
    int success = 0;

    assert(pBufferTable);

    if(tableIndex < pBufferTable->maxNumberOfEntries)
    {
        pBufferTable->entries[tableIndex].data = pData;
        pBufferTable->entries[tableIndex].dataSize = dataSize;
        success = 1;
    }

    return success;
}

int getBufferEntry(BufferTable *pBufferTable, int requested_length, void **buf, int *max_length)
{
    int result = FAIL;
    int i;

    //Space on the Buffer
    if (pBufferTable->numberOfAllocatedEntries < pBufferTable->maxNumberOfEntries)
    {
        //Find first free spot
        for (i=0; i< pBufferTable->maxNumberOfEntries; i++)
        {
            if ((!pBufferTable->entries[i].used)&&(requested_length <= pBufferTable->entries[i].dataSize))
            {
                //Match found
                //Point to the correct buffer
                *buf = pBufferTable->entries[i].data;
                //Report the defined maximum length
                *max_length = pBufferTable->entries[i].dataSize;

                pBufferTable->entries[i].used = 1;
                //Update position
                pBufferTable->numberOfAllocatedEntries++;

                //Update result
                result = SUCCESS;
                break;
            }
        }
    }

    return result;
}

int freeBufferEntry(BufferTable *pBufferTable, void *buf)
{
    int result = FAIL;
    int i;

    if (pBufferTable->numberOfAllocatedEntries > 0)
    {
        //Find pointer that has been freed
        for (i=0; i< pBufferTable->maxNumberOfEntries; i++)
        {
            if (buf == pBufferTable->entries[i].data)
            {
                //Match found
                pBufferTable->entries[i].used = 0;
                //Update position
                pBufferTable->numberOfAllocatedEntries--;

                //Update result
                result = SUCCESS;
                break;
            }
        }
    }

    return result;
}
