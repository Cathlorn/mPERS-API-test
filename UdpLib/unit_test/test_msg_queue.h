#ifndef TEST_MSG_QUEUE_H
#define TEST_MSG_QUEUE_H

//returns BOOL success / fail
int enqueueMsg(void *data, int len);

int dequeueMsg();

void clearMsgs();

//returns int denoting size of the queue
int msgsAvailable();

//returns BOOL success / fail
int hasMsgs();

//returns BOOL success / fail
int peekMsg(void **data, int *len);

#endif //TEST_MSG_QUEUE_H
