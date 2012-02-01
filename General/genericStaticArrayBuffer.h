#ifndef GENERIC_STATIC_ARRAY_BUFFER_H
#define GENERIC_STATIC_ARRAY_BUFFER_H

#include "types.h"

#ifndef NULL
#define NULL 0
#endif

typedef struct
{
    uint8 used;
    void  *data; //Holds the pointer to the item
    int   dataSize;
}
BufferTableEntry;

#define BUFFER_TABLE_ENTRY_INIT_DEFAULT() { \
 .used     = 0, \
 .data     = NULL, \
 .dataSize = 0, \
}

#define BUFFER_TABLE_ENTRY_INIT(a,b) { \
 .used     = 0, \
 .data     = a, \
 .dataSize = b, \
}

typedef struct
{
    int numberOfAllocatedEntries;
    BufferTableEntry *entries;
    int maxNumberOfEntries;
}
BufferTable;

#define BUFFER_TABLE_INIT_DEFAULT() { \
.numberOfAllocatedEntries = 0, \
.entries                  = NULL, \
.maxNumberOfEntries       = 0, \
}

#define BUFFER_TABLE_INIT(a,b) { \
.numberOfAllocatedEntries = 0, \
.entries                  = a, \
.maxNumberOfEntries       = b, \
}

int initBufferTable(BufferTable *pBufferTable, void *pEntries, int maxNumberOfEntries);
int assignData(BufferTable *pBufferTable, unsigned int tableIndex, void *pData, int dataSize);
int getBufferEntry(BufferTable *pBufferTable, int requested_length, void **buf, int *max_length);
int freeBufferEntry(BufferTable *pBufferTable, void *buf);

#endif //GENERIC_STATIC_ARRAY_BUFFER_H
