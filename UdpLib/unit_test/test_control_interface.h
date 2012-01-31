#ifndef TEST_CONTROL_INTERFACE_H
#define TEST_CONTROL_INTERFACE_H

void setTestRcvdCallback(TestFunction testFunction);
void setTestSentCallback(TestFunction testFunction);
void setTestDroppedCallback(TestFunction testFunction);

//myHalo UDP Protocol Tx notification interface
void setTestSendCallback(TestFunction testFunction);
int getLastTestSendCallbackResult();
void setSendFuncCalled(int called);
int getSendFuncCalled();
int wasSendFuncCalled();

void setRcvFuncCalled(int called);
int getRcvFuncCalled();
int wasRcvFuncCalled();

void setSentFuncCalled(int called);
int getSentFuncCalled();
int wasSentFuncCalled();

void setDroppedFuncCalled(int called);
int wasDroppedFuncCalled();

void enableAckGenToRxData();
void disableAckGenToRxData();

void enableSessionResponseData();
void disableSessionResponseData();

int getNextTxSeqNum();

#endif //TEST_CONTROL_INTERFACE_H
