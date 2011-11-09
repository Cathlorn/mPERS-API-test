#ifndef TX_BUFFER_STATIC_H
#define TX_BUFFER_STATIC_H

int getBuffer(int requested_length, void **data, int *max_length);
int freeBuffer(void *data);

#endif //TX_BUFFER_STATIC_H
