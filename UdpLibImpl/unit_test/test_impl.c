#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "types.h"

#include "UdpLib/udp_lib.h"
#include "test_control_interface.h"
#include "udp_tests.h"

void reset_test_state()
{
    //Clear callback status state
    setClientRcvdFuncCalled(0);
    setClientSentFuncCalled(0);
    setServerRcvdFuncCalled(0);
    setServerSentFuncCalled(0);
    setClientTestRcvdCallback(NULL);
    setClientTestSentCallback(NULL);
    setServerTestRcvdCallback(NULL);
    setServerTestSentCallback(NULL);
}

static uint8 clientTxDataSample[] = {0xdd,0x1e,0x00,0x17, 0x71};
static uint8 serverTxDataSample[] = {0xfe,0x23,0xa,0xbe, 0x41,0x99,0x3};
static int clientDataSent = 0;
static int clientDataRcvd = 0;
static int serverDataSent = 0;
static int serverDataRcvd = 0;

int txGoodTest_ClientSentCallback(void *args)
{
    int passed = 0;
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if ((memcmp(pUdpEventData->data, clientTxDataSample, sizeof(clientTxDataSample)) == 0)
            &&(pUdpEventData->length == sizeof(clientTxDataSample)))
    {
        clientDataSent = 1;
    }

    return passed;
}

int txGoodTest_ClientRcvdCallback(void *args)
{
    int passed = 0;
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if ((memcmp(pUdpEventData->data, serverTxDataSample, sizeof(serverTxDataSample)) == 0)
            &&(pUdpEventData->length == sizeof(serverTxDataSample)))
    {
        serverDataRcvd = 1;
    }

    return passed;
}

int txGoodTest_ServerSentCallback(void *args)
{
    int passed = 0;
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if ((memcmp(pUdpEventData->data, serverTxDataSample, sizeof(serverTxDataSample)) == 0)
            &&(pUdpEventData->length == sizeof(serverTxDataSample)))
    {
        serverDataSent = 1;
    }

    return passed;
}

int txGoodTest_ServerRcvdCallback(void *args)
{
    int passed = 0;
    UdpEventData *pUdpEventData = (UdpEventData *) args;

    if ((memcmp(pUdpEventData->data, clientTxDataSample, sizeof(clientTxDataSample)) == 0)
            &&(pUdpEventData->length == sizeof(clientTxDataSample)))
    {
        clientDataRcvd = 1;
        udp_sendto(&serverUdpCommStruct, serverTxDataSample, sizeof(serverTxDataSample), pUdpEventData->commStruct->clientIP);
    }

    return passed;
}

int txGoodTest(void *args)
{
    int passed = 0;
    int timeout = 40;
    int stop = 0;

    reset_test_state();

    setClientTestRcvdCallback(txGoodTest_ClientRcvdCallback);
    setClientTestSentCallback(txGoodTest_ClientSentCallback);
    setServerTestRcvdCallback(txGoodTest_ServerRcvdCallback);
    setServerTestSentCallback(txGoodTest_ServerSentCallback);

    clientDataSent = 0;
    clientDataRcvd = 0;
    serverDataSent = 0;
    serverDataRcvd = 0;

    udp_send(&clientUdpCommStruct, clientTxDataSample, sizeof(clientTxDataSample));

    do
    {
        if ((clientDataSent)&&(clientDataRcvd)&&(serverDataSent)&&(serverDataRcvd))
        {
            //Confirmed there is a stop
            stop = 1;
            passed = 1;
        }

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Tx Good Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}
