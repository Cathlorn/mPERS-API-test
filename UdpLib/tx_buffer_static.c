//10/24/2011
//CLS (Halo Monitoring)
//Generic Buffer pool that returns points to different statically allocated segments of memory
//Pool implements a circular queue from a static array

#include "General/types.h"
#include "General/genericStaticArrayBuffer.h"
#include "tx_buffer_static.h"

//Configuration
#define TX_BUFFER_SIZE 1492//10
#define TX_BUFFER_DATA_BLOCK_SIZE 256

static uint8 dataBuffer[TX_BUFFER_SIZE][TX_BUFFER_DATA_BLOCK_SIZE];
static BufferTableEntry bufferTableEntries[TX_BUFFER_SIZE];
static BufferTable bufferTable = BUFFER_TABLE_INIT(bufferTableEntries,sizeof(bufferTableEntries));
static int tableInitialized = 0;

void populateDataPtrsToTable()
{
    int i;

    for(i = 0; i < bufferTable.maxNumberOfEntries; i++)
    {
        assignData(&bufferTable, i, &dataBuffer[i][0], TX_BUFFER_DATA_BLOCK_SIZE);
    }
}

void initTable()
{
    initBufferTable(&bufferTable, bufferTableEntries, sizeof(bufferTableEntries)/sizeof(BufferTableEntry));
    populateDataPtrsToTable();
}

int getBuffer(int requested_length, void **data, int *max_length)
{
    if(!tableInitialized)
    {
        initTable();
        tableInitialized = 1;
    }

    return getBufferEntry(&bufferTable, requested_length, data, max_length);
}

int freeBuffer(void *data)
{
    return freeBufferEntry(&bufferTable, data);
}
