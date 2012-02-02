#ifndef CLIENT_TEST_IMPL_H
#define CLIENT_TEST_IMPL_H

//Function Prototypes
int sendDynamicVitalsPkt(void *args);
int sendPanicMsgPkt(void *args);
int sendFallMsgPkt(void *args);
int sendOperatorAckMsgPkt(void *args);
int sendInvalidMsgPkt(void *args);
int listenMsg(void *args);

#endif //CLIENT_TEST_IMPL_H
