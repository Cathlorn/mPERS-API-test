#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "General/types.h"

#include "UdpLib/udp_lib.h"
#include "udp_tests.h"
#include "Test/tests.h"
#include "test_control_interface.h"

#define TRUE  1
#define FALSE 0

static uint8 runThreads = 1;

//Helper Functions

static int client_udp_data_rcvd_called    = 0;
static TestFunction client_test_rcvd_callback = NULL;
static int client_last_test_rcvd_callback_result = 0;
void setClientTestRcvdCallback(TestFunction testFunction)
{
    client_test_rcvd_callback = testFunction;
}
int getClientLastTestRcvdCallbackResult()
{
    return client_last_test_rcvd_callback_result;
}

void setClientRcvdFuncCalled(int called)
{
    client_udp_data_rcvd_called = called;
}

int getClientRcvdFuncCalled()
{
    return client_udp_data_rcvd_called;
}

int wasClientRcvdFuncCalled()
{
    return client_udp_data_rcvd_called;
}

static int client_udp_data_sent_called    = 0;
static TestFunction client_test_sent_callback = NULL;
static int client_last_test_sent_callback_result = 0;
void setClientTestSentCallback(TestFunction testFunction)
{
    client_test_sent_callback = testFunction;
}

int getClientLastTestSentCallbackResult()
{
    return client_last_test_sent_callback_result;
}

void setClientSentFuncCalled(int called)
{
    client_udp_data_sent_called = called;
}

int getClientSentFuncCalled()
{
    return client_udp_data_sent_called;
}

int wasClientSentFuncCalled()
{
    return client_udp_data_sent_called;
}

static int server_udp_data_rcvd_called    = 0;
static TestFunction server_test_rcvd_callback = NULL;
static int server_last_test_rcvd_callback_result = 0;
void setServerTestRcvdCallback(TestFunction testFunction)
{
    server_test_rcvd_callback = testFunction;
}
int getServerLastTestRcvdCallbackResult()
{
    return server_last_test_rcvd_callback_result;
}

void setServerRcvdFuncCalled(int called)
{
    server_udp_data_rcvd_called = called;
}

int getServerRcvdFuncCalled()
{
    return server_udp_data_rcvd_called;
}

int wasServerRcvdFuncCalled()
{
    return server_udp_data_rcvd_called;
}

static int server_udp_data_sent_called    = 0;
static TestFunction server_test_sent_callback = NULL;
static int server_last_test_sent_callback_result = 0;
void setServerTestSentCallback(TestFunction testFunction)
{
    server_test_sent_callback = testFunction;
}
int getServerLastTestSentCallbackResult()
{
    return server_last_test_sent_callback_result;
}

void setServerSentFuncCalled(int called)
{
    server_udp_data_sent_called = called;
}

int getServerSentFuncCalled()
{
    return server_udp_data_sent_called;
}

int wasServerSentFuncCalled()
{
    return server_udp_data_sent_called;
}

//UDP layer callbacks
void client_data_rcvd(void *args)
{
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if (client_test_rcvd_callback)
    {
        client_last_test_rcvd_callback_result = client_test_rcvd_callback(pUdpEventData);
    }

    client_udp_data_rcvd_called++;
}

void client_data_sent(void *args)
{
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if (client_test_sent_callback)
    {
        client_last_test_sent_callback_result = client_test_sent_callback(pUdpEventData);
    }

    client_udp_data_sent_called++;
}

void server_data_rcvd(void *args)
{
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if (server_test_rcvd_callback)
    {
        server_last_test_rcvd_callback_result = server_test_rcvd_callback(pUdpEventData);
    }

    server_udp_data_rcvd_called++;
}

void server_data_sent(void *args)
{
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if (server_test_sent_callback)
    {
        server_last_test_sent_callback_result = server_test_sent_callback(pUdpEventData);
    }

    server_udp_data_sent_called++;
}

int doTest()
{
    return runAllTestItems(&udpProtocolTesting);
}

void * testThreadHandler(void *arg)
{
    int passed = -1;

    passed = doTest();

    *(int *) arg = passed;

    runThreads = 0;

    return arg;
}

UdpCommStruct clientUdpCommStruct = { \
                                      .hostname = "localhost", \
                                      .port = "53778", \
                                      .debug = 1, \
                                      .actAsServer = 0, \
                                      .dataSent = client_data_sent, \
                                      .dataReceived = client_data_rcvd, \
                                    };
UdpCommStruct serverUdpCommStruct = { \
                                      .hostname = "localhost", \
                                      .port = "53778", \
                                      .debug = 1, \
                                      .actAsServer = 1, \
                                      .dataSent = server_data_sent, \
                                      .dataReceived = server_data_rcvd, \
                                    };

int udp_test(void)
{
    pthread_t testThr;
    int testResult = -1;

    udp_init(&clientUdpCommStruct);
    udp_init(&serverUdpCommStruct);

    if (pthread_create(&testThr, NULL, &testThreadHandler, &testResult))
    {
        printf("Could not create thread\n");
        exit(-1);
    }

    while (runThreads)
    {
        udp_tick(&clientUdpCommStruct);
        udp_tick(&serverUdpCommStruct);
        usleep(2000); // 2ms sleep
    }

    runThreads = 0;

    if (pthread_join(testThr, NULL))
    {
        printf("Could not join thread\n");
        exit(-1);
    }

    udp_cleanup(&clientUdpCommStruct);
    udp_cleanup(&serverUdpCommStruct);

    return testResult;
}
