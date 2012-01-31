#ifndef TEST_CONTROL_INTERFACE_H
#define TEST_CONTROL_INTERFACE_H

#include "UdpLib/udp_lib.h"
#include "Test/tests.h"

void setClientTestRcvdCallback(TestFunction testFunction);
int getClientLastTestRcvdCallbackResult();
void setClientRcvdFuncCalled(int called);
int getClientRcvdFuncCalled();
int wasClientRcvdFuncCalled();
void setClientTestSentCallback(TestFunction testFunction);
int getClientLastTestSentCallbackResult();
void setClientSentFuncCalled(int called);
int getClientSentFuncCalled();
int wasClientSentFuncCalled();
void setServerTestRcvdCallback(TestFunction testFunction);
int getServerLastTestRcvdCallbackResult();
void setServerRcvdFuncCalled(int called);
int getServerRcvdFuncCalled();
int wasServerRcvdFuncCalled();
void setServerTestSentCallback(TestFunction testFunction);
int getServerLastTestSentCallbackResult();
void setServerSentFuncCalled(int called);
int getServerSentFuncCalled();
int wasServerSentFuncCalled();
void client_data_rcvd(void *args);
void client_data_sent(void *args);
void server_data_rcvd(void *args);
void server_data_sent(void *args);

extern UdpCommStruct clientUdpCommStruct;
extern UdpCommStruct serverUdpCommStruct;

#endif //TEST_CONTROL_INTERFACE_H
