#ifndef TEST_IMPL_H
#define TEST_IMPL_H

//Function Prototypes
int runtTest(void *args);
int goodPktTest(void *args);
int duplicatePktTest(void *args);
int rxOutOfSeqTest(void *args);
int badCrcTest(void *args);
int rxInvalidPayloadLengthTest(void *args);
int txDropTest(void *args);
int txGoodTest(void *args);
int txSeqNumBeyondSeqNumRange(void *args);
int rxSeqNumBeyondSeqNumRange(void *args);
int txSessionControlTest(void *args);
int rxSessionControlTest(void *args);

#endif //TEST_IMPL_H
