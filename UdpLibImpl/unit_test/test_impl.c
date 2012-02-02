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

static uint8 clientTxDataSample[] = {0xdd,0x1e,0x00,0x17,0x71};
static uint8 serverTxDataSample[] = {0xfe,0x23,0xa,0xbe,0x41,0x99,0x3};
static int clientDataSent = 0;
static int clientDataRcvd = 0;
static int serverDataSent = 0;
static int serverDataRcvd = 0;

int commTest_ClientSentCallback(void *args)
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

int commTest_ClientRcvdCallback(void *args)
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

int commTest_ServerSentCallback(void *args)
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

int commTest_ServerRcvdCallback(void *args)
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

int biDirectionalCommCheck()
{
    int passed = 0;
    int timeout = 5;
    int stop = 0;

    reset_test_state();

    setClientTestRcvdCallback(commTest_ClientRcvdCallback);
    setClientTestSentCallback(commTest_ClientSentCallback);
    setServerTestRcvdCallback(commTest_ServerRcvdCallback);
    setServerTestSentCallback(commTest_ServerSentCallback);

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

    return passed;
}

int commTest(void *args)
{
    int passed = 0;

    passed = biDirectionalCommCheck();

    printf("Bidirectional Communication Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int reOpenTest(void *args)
{
    int passed = 1;

    reset_test_state();

    printf("Confirming server and client sockets are open.\n");
    if (passed)
    {
        //Checking that both sockets are initially working and reporting correctly
        passed &= udp_isOpen(&clientUdpCommStruct);
        passed &= udp_isOpen(&serverUdpCommStruct);
    }

    //Confirming data comm works to start with
    if (passed)
        passed &= biDirectionalCommCheck();

    printf("Checking closing and reopening in client mode.\n");
    if (passed)
    {
        udp_cleanup(&clientUdpCommStruct);
        passed &= !udp_isOpen(&clientUdpCommStruct);
    }

    printf("Checking comm doesn't work on a closed client.\n");
    if (passed)
        passed &= !biDirectionalCommCheck();

    if (passed)
    {
        udp_init(&clientUdpCommStruct);
        passed &= udp_isOpen(&clientUdpCommStruct);
    }

    if (passed)
        passed &= biDirectionalCommCheck();

    printf("Checking closing and reopening in server mode.\n");
    if (passed)
    {
        udp_cleanup(&clientUdpCommStruct);
        passed &= !udp_isOpen(&clientUdpCommStruct);
    }

    printf("Checking comm doesn't work on a closed server.\n");
    if (passed)
        passed &= !biDirectionalCommCheck();

    if (passed)
    {
        udp_init(&clientUdpCommStruct);
        passed &= udp_isOpen(&clientUdpCommStruct);
    }

    if (passed)
        passed &= biDirectionalCommCheck();

    printf("Closing and Reopening Socket Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int clientBadWriteTest(void)
{
    int passed = 0;
    int success = 1;
    int timeout = 5;
    int stop = 0;
    GenericIP dstIP = GENERIC_IP_INIT();

    reset_test_state();

    clientDataSent = 0;
    clientDataRcvd = 0;
    serverDataSent = 0;
    serverDataRcvd = 0;

    printf("Confirming server and client sockets are open.\n");
    if (success)
    {
        //Checking that both sockets are initially working and reporting correctly
        success &= udp_isOpen(&clientUdpCommStruct);
        success &= udp_isOpen(&serverUdpCommStruct);
    }

    if (success)
    {
        udp_sendto(&clientUdpCommStruct, clientTxDataSample, sizeof(clientTxDataSample), dstIP);

        do
        {
            if ((clientDataSent)||(serverDataRcvd))
            {
                //Confirmed there is a stop
                stop = 1;
            }
            sleep(1);
            timeout--;
        }
        while ((!stop)&&(timeout));

        if ((!clientDataSent)&&(!serverDataRcvd))
        {
            passed = 1;
        }
    }

    return passed;
}

int serverBadWriteTest(void)
{
    int passed = 0;
    int success = 1;
    int timeout = 5;
    int stop = 0;
    GenericIP dstIP = GENERIC_IP_INIT();

    reset_test_state();

    clientDataSent = 0;
    clientDataRcvd = 0;
    serverDataSent = 0;
    serverDataRcvd = 0;

    printf("Confirming server and client sockets are open.\n");
    if (success)
    {
        //Checking that both sockets are initially working and reporting correctly
        success &= udp_isOpen(&clientUdpCommStruct);
        success &= udp_isOpen(&serverUdpCommStruct);
    }

    if (success)
    {
        udp_sendto(&serverUdpCommStruct, serverTxDataSample, sizeof(serverTxDataSample), dstIP);

        do
        {
            if ((serverDataSent)||(clientDataRcvd))
            {
                //Confirmed there is a stop
                stop = 1;
            }
            sleep(1);
            timeout--;
        }
        while ((!stop)&&(timeout));

        if ((!serverDataSent)&&(!clientDataRcvd))
        {
            passed = 1;
        }
    }

    return passed;
}

int badAddrWriteTest(void *args)
{
    int passed = 1;

    reset_test_state();

    printf("Confirming server and client sockets are open.\n");
    if (passed)
    {
        //Checking that both sockets are initially working and reporting correctly
        passed &= udp_isOpen(&clientUdpCommStruct);
        passed &= udp_isOpen(&serverUdpCommStruct);
    }

    if (passed)
        passed &= serverBadWriteTest();

    if (passed)
        passed &= clientBadWriteTest();

    printf("Bad Addr Write Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int serverNoDataAvailableTest(void)
{
    unsigned char buffer[1024];
    int passed = 0;
    int timeout = 5;
    int stop = 0;
    int dataFound = 0;

    reset_test_state();

    do
    {
        if (udp_recv(&serverUdpCommStruct, buffer, sizeof(buffer)) > 0)
        {
            //Confirmed there is a stop
            stop = 1;
            dataFound = 1;
        }
        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if ((!dataFound)&&(!serverDataRcvd))
    {
        passed = 1;
    }

    return passed;
}

int clientNoDataAvailableTest(void)
{
    unsigned char buffer[1024];
    int passed = 0;
    int timeout = 5;
    int stop = 0;
    int dataFound = 0;

    reset_test_state();

    do
    {
        if (udp_recv(&clientUdpCommStruct, buffer, sizeof(buffer)) > 0)
        {
            //Confirmed there is a stop
            stop = 1;
            dataFound = 1;
        }
        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if ((!dataFound)&&(!clientDataRcvd))
    {
        passed = 1;
    }

    return passed;
}

int noDataAvailableTest(void *args)
{
    int passed = 1;

    reset_test_state();

    printf("Confirming server and client sockets are open.\n");
    if (passed)
    {
        //Checking that both sockets are initially working and reporting correctly
        passed &= udp_isOpen(&clientUdpCommStruct);
        passed &= udp_isOpen(&serverUdpCommStruct);
    }

    if (passed)
    {
        passed &= clientNoDataAvailableTest();
    }

    if (passed)
    {
        passed &= serverNoDataAvailableTest();
    }

    printf("No Data Read Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int clientBogusNameCheck(void)
{
    int passed = 1;
    char tempHostName[] = "badName";
    char oldHostName[1024] = "";

    reset_test_state();

    clientDataSent = 0;
    clientDataRcvd = 0;
    serverDataSent = 0;
    serverDataRcvd = 0;

    printf("Checking closing socket in client mode.\n");
    if (passed)
    {
        udp_cleanup(&clientUdpCommStruct);
        passed &= !udp_isOpen(&clientUdpCommStruct);
    }

    if (passed)
    {
        //Backup old host name
        strcpy(oldHostName, clientUdpCommStruct.hostname);

        //Swapping char * to place bogus name
        strcpy(clientUdpCommStruct.hostname, tempHostName);

        printf("Confirming bogus name does not work.\n");
        if (passed)
        {
            udp_init(&clientUdpCommStruct);
            passed &= !udp_isOpen(&clientUdpCommStruct);
        }

        if (passed)
            passed &= !biDirectionalCommCheck();

        //Restore old host name
        strcpy(clientUdpCommStruct.hostname, oldHostName);

        //Reconnect with old host name
        if (passed)
        {
            udp_init(&clientUdpCommStruct);
            passed &= udp_isOpen(&clientUdpCommStruct);
        }
    }

    return passed;
}

int clientBogusPortCheck(void)
{
    int passed = 1;
    char tempPortName[] = "-53";
    char oldPortName[1024] = "";

    reset_test_state();

    clientDataSent = 0;
    clientDataRcvd = 0;
    serverDataSent = 0;
    serverDataRcvd = 0;

    printf("Checking closing socket in client mode.\n");
    if (passed)
    {
        udp_cleanup(&clientUdpCommStruct);
        passed &= !udp_isOpen(&clientUdpCommStruct);
    }

    if (passed)
    {
        //Backup old host name
        strcpy(oldPortName, clientUdpCommStruct.port);

        //Swapping char * to place bogus name
        strcpy(clientUdpCommStruct.port, tempPortName);

        printf("Confirming bogus port does not work.\n");
        if (passed)
        {
            udp_init(&clientUdpCommStruct);
            passed &= !udp_isOpen(&clientUdpCommStruct);
        }

        if (passed)
            passed &= !biDirectionalCommCheck();

        //Restore old host name
        strcpy(clientUdpCommStruct.port, oldPortName);

        //Reconnect with old host name
        if (passed)
        {
            udp_init(&clientUdpCommStruct);
            passed &= udp_isOpen(&clientUdpCommStruct);
        }
    }

    return passed;
}


int serverBogusNameCheck(void)
{
    int passed = 1;
    char tempHostName[] = "badName";
    char oldHostName[1024] = "";

    reset_test_state();

    clientDataSent = 0;
    clientDataRcvd = 0;
    serverDataSent = 0;
    serverDataRcvd = 0;

    printf("Checking closing socket in server mode.\n");
    if (passed)
    {
        udp_cleanup(&serverUdpCommStruct);
        passed &= !udp_isOpen(&serverUdpCommStruct);
    }

    if (passed)
    {
        //Backup old host name
        strcpy(oldHostName, serverUdpCommStruct.hostname);

        //Swapping char * to place bogus name
        strcpy(serverUdpCommStruct.hostname, tempHostName);

        printf("Confirming bogus name does not work.\n");
        if (passed)
        {
            udp_init(&serverUdpCommStruct);
            passed &= !udp_isOpen(&serverUdpCommStruct);
        }

        if (passed)
            passed &= !biDirectionalCommCheck();

        //Restore old host name
        strcpy(serverUdpCommStruct.hostname, oldHostName);

        //Reconnect with old host name
        if (passed)
        {
            udp_init(&serverUdpCommStruct);
            passed &= udp_isOpen(&serverUdpCommStruct);
        }
    }

    return passed;
}

int serverBogusPortCheck(void)
{
    int passed = 1;
    char tempPortName[] = "-53";
    char oldPortName[1024] = "";

    reset_test_state();

    clientDataSent = 0;
    clientDataRcvd = 0;
    serverDataSent = 0;
    serverDataRcvd = 0;

    printf("Checking closing socket in server mode.\n");
    if (passed)
    {
        udp_cleanup(&serverUdpCommStruct);
        passed &= !udp_isOpen(&serverUdpCommStruct);
    }

    if (passed)
    {
        //Backup old host name
        strcpy(oldPortName, serverUdpCommStruct.port);

        //Swapping char * to place bogus name
        strcpy(serverUdpCommStruct.port, tempPortName);

        printf("Confirming bogus port does not work.\n");
        if (passed)
        {
            udp_init(&serverUdpCommStruct);
            passed &= !udp_isOpen(&serverUdpCommStruct);
        }

        if (passed)
            passed &= !biDirectionalCommCheck();

        //Restore old host name
        strcpy(serverUdpCommStruct.port, oldPortName);

        //Reconnect with old host name
        if (passed)
        {
            udp_init(&serverUdpCommStruct);
            passed &= udp_isOpen(&serverUdpCommStruct);
        }
    }

    return passed;
}

int badNetworkConfigTest(void *args)
{
    int passed = 1;

    reset_test_state();

    printf("Confirming server and client sockets are open.\n");
    if (passed)
    {
        //Checking that both sockets are initially working and reporting correctly
        passed &= udp_isOpen(&clientUdpCommStruct);
        passed &= udp_isOpen(&serverUdpCommStruct);
    }

    if (passed)
    {
        passed &= clientBogusNameCheck();
    }

    if (passed)
    {
        passed &= clientBogusPortCheck();
    }

    if (passed)
    {
        passed &= serverBogusNameCheck();
    }

    if (passed)
    {
        passed &= serverBogusPortCheck();
    }

    printf("Invalid Network Config Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}
