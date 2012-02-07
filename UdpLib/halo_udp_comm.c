#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "myHaloUdp.h"
#include "halo_udp_comm.h"
#include "udp_lib.h"
#include "tx_buffer.h"
#include "mpers/HaloMsgHelperFunctions.h"
#include "halo_udp_tx_mgmt.h"
#include "crc16.h"
#include "halo_udp_stats.h"

#define ACK_STACK_SIZE 50

enum TxSessionState
{
    UNINITIATED,
    START_NEW_SESSION,
    NEW_SESSION_IN_PROGRESS,
    SESSION_IN_USE
};

typedef struct
{
    GenericIP socketAddr;
    uint16 acknowledgementStack[ACK_STACK_SIZE];
    int ackStackSize;
    int ackStackIndex;
    int used;
    int sessionTickCount;
    uint16 txSeqNum;
    int newTxSession; //Indicates that a new session is starting
    enum TxSessionState txSessionState;
}
SessionData;

#define SESSION_DATA_INIT() { \
 .socketAddr = GENERIC_IP_INIT(), \
 .acknowledgementStack = {0}, \
 .ackStackSize  = 0, \
 .ackStackIndex = 0, \
 .used = 0, \
 .sessionTickCount = 0, \
 .txSeqNum  = 0, \
 .newTxSession = 0, \
 .txSessionState = UNINITIATED, \
}

typedef struct
{
    UdpCommStruct haloUdpCommStruct;
    HaloUdpTxMgmt txMgmt;
    int tickCount;
    HaloUdpUserData *userData;
    int currentSessionIndex;
    SessionData sessionData[MAX_CONCURRENT_CONNNECTIONS];
    HaloUdpStats stats;
}
HaloUdpCommData;

#define HALO_UDP_COMM_DATA_INIT() { \
 .haloUdpCommStruct = UDP_COMM_STRUCT_INIT(), \
 .txMgmt = HALO_UDP_TX_MGMT_INIT(), \
 .tickCount = 0, \
 .userData  = NULL, \
 .currentSessionIndex = 0, \
 .stats = HALO_UDP_STATS_INIT(), \
}

//Stores the data used to handle the halo UDP communications stack
static HaloUdpCommData haloUdpCommData = HALO_UDP_COMM_DATA_INIT();

//Function Prototypes
void udp_recv_handler(void *data);
void halo_msg_rexmit(void);
int getSessionIndex( GenericIP socketAddress, int *sessionIndex);

//Handler for successfully sent msgs
void halo_msg_tx_sent( GenericIP socketAddress, uint16 txSeqNum);

//Handler for failed transmitted msgs
void halo_msg_tx_dropped(int offset);

void halo_msg_init(HaloUdpUserData *userData)
{
    int i;
    UdpCommStruct *commStruct = NULL;

    assert(userData);
    haloUdpCommData.userData = userData;

    commStruct = &haloUdpCommData.haloUdpCommStruct;

    //Sets up UDP port

    if (userData->hostname)
        strcpy(commStruct->hostname, userData->hostname);
    else
        strcpy(commStruct->hostname, "localhost");

    strcpy(commStruct->port, userData->port);

    //Enables Debugging
    commStruct->debug = userData->debug;

    commStruct->actAsServer = userData->actAsServer;

    //Initialize the structures holding the session data
    for (i = 0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
    {
        haloUdpCommData.sessionData[i].ackStackSize  = 0;
        haloUdpCommData.sessionData[i].ackStackIndex = 0;
        haloUdpCommData.sessionData[i].used = 0;
        haloUdpCommData.sessionData[i].sessionTickCount = 0;
        haloUdpCommData.sessionData[i].txSeqNum  = 0;
        haloUdpCommData.sessionData[i].newTxSession = 0;
        haloUdpCommData.sessionData[i].txSessionState = UNINITIATED;
    }

    //Assigns event handlers
    commStruct->dataSent = NULL;
    commStruct->dataReceived = udp_recv_handler;

    udp_init(commStruct);

    //Preps client (add in an entry for the server it is intended to connect with)
    if (!userData->actAsServer)
    {
        haloUdpCommData.sessionData[haloUdpCommData.currentSessionIndex].used = 1;
        haloUdpCommData.sessionData[haloUdpCommData.currentSessionIndex].socketAddr = commStruct->socketIP;

        //Debug TO DO: Figure out a better way of handling this. (New Session should not require you to know the session index)
        halo_msg_new_session(haloUdpCommData.currentSessionIndex);
    }
}

//NOTE: All sending and receiving of data needs to be halted when this is called
//to keep packet information correct and in-sync without loss.
void halo_msg_new_session(int sessionIndex)
{
    SessionData *currentSessionPtr = NULL;
    HaloUdpTxMgmt *txMgmt = NULL;
    uint8 *data;
    int dataLen;
    uint16 seqNum;
    GenericIP socketAddress;
    int queueSize;
    int i;

    currentSessionPtr = &haloUdpCommData.sessionData[sessionIndex];

    //Check that the session is being used (TO DO: See about returning a failure if not performed)
    if (currentSessionPtr->used)
    {
        //This handles resetting all of the state information used to keep track of data being exchanged
        currentSessionPtr->newTxSession = 1;
        currentSessionPtr->txSessionState = UNINITIATED;
        currentSessionPtr->txSeqNum = 0;

        //Adjust the Tx Mgmt Queue with new Session information
        txMgmt    = &haloUdpCommData.txMgmt;

        //TO DO: Make it so that navigation seeks, add generic return structure
        //Add for random dequeue
        queueSize = tx_packet_queue_size(txMgmt);
        for (i=0; i < queueSize; i++)
        {
            if (peek_tx_packet(txMgmt, i, &data, &dataLen, &seqNum, &socketAddress) == SUCCESS)
            {
                if ((currentSessionPtr->socketAddr.address == socketAddress.address) &&
                        (currentSessionPtr->socketAddr.port == socketAddress.port) )
                {
                    //Reprocess entire packet with new session related information
                    MyHaloUdpHeader *header = (MyHaloUdpHeader *) data;
                    int msgLen;
                    uint16 pktCrc;

                    header->seqNum = currentSessionPtr->txSeqNum;
                    currentSessionPtr->txSeqNum++;

                    //Calculate total msg length
                    msgLen = header->payloadLength;
                    msgLen += sizeof(MyHaloUdpHeader); //header: udp data
                    msgLen += sizeof(uint16);  //tail: crc

                    //Recalculate the CRC
                    pktCrc = hdlcFcs16(hdlc_init_fcs16, data, msgLen - 2);

                    //Intentionally make CRCs bad
                    if (haloUdpCommData.userData->dbgTestCtrls.badCrc)
                        pktCrc++;

                    //Copy CRC
                    memcpy(&data[msgLen - 2], &pktCrc, sizeof(pktCrc));

                    //Put data into new position into the queue
                    dequeue_tx_packet(txMgmt, socketAddress, seqNum); //removes with old seq num
                    enqueue_tx_packet(txMgmt, data, dataLen, header->seqNum, socketAddress); //Adds new seq num
                }
            }
        }
    }
}

//NOTE: Access in the send queue needs to be thread-protected so that additions/ removal are locked
int halo_msg_sendto(const void *data, const int len,
                    GenericIP socketAddress)
{
    int result = FAIL;
    int sessionIndex;

    if (getSessionIndex(socketAddress, &sessionIndex) == SUCCESS)
    {
        result = halo_msg_send_to_index(data, len, sessionIndex);
    }
    else
    {
        //Create a new tx entry
        int i;
        int firstAvailableSessionSlot = -1;

        for (i = 0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
        {
            if (!haloUdpCommData.sessionData[i].used)
            {
                //Marks first place a new session could be added
                firstAvailableSessionSlot = i;

                //Save IP
                haloUdpCommData.sessionData[firstAvailableSessionSlot].socketAddr = socketAddress;

                //Set up the acknowledgement queue
                haloUdpCommData.sessionData[firstAvailableSessionSlot].ackStackSize  = 0;
                haloUdpCommData.sessionData[firstAvailableSessionSlot].ackStackIndex = 0;
                haloUdpCommData.sessionData[firstAvailableSessionSlot].sessionTickCount = 0;
                haloUdpCommData.sessionData[firstAvailableSessionSlot].used = 1;

                halo_msg_new_session(firstAvailableSessionSlot);

                //Send using the new entry
                result = halo_msg_send_to_index(data, len, firstAvailableSessionSlot);
                break;
            }
        }
    }

    return result;
}

int halo_msg_send(const void *data, const int len)
{
    return halo_msg_send_to_index(data, len, haloUdpCommData.currentSessionIndex);
}

int halo_msg_send_to_index(const void *data, const int len, int sessionIndex)
{
    SessionData *currentSessionPtr = NULL;
    MyHaloUdpHeader header = MY_HALO_UDP_HEADER_INIT();
    UdpCommStruct *commStruct = NULL;
    HaloUdpTxMgmt *txMgmt = NULL;
    int msgLen;
    //int payloadLen;
    uint8 *dataBuf;
    int dataBufLen;
    uint16 pktCrc;
    int result = FAIL;

    commStruct = &haloUdpCommData.haloUdpCommStruct;
    txMgmt     = &haloUdpCommData.txMgmt;
    //payloadLen = getMsgLength(msg);

    //msgLen = payloadLen;
    msgLen = len;
    //Update the msg length
    msgLen += sizeof(MyHaloUdpHeader); //header: udp data
    msgLen += sizeof(uint16);  //tail: crc

    currentSessionPtr = &haloUdpCommData.sessionData[sessionIndex];

    if (currentSessionPtr->used)
    {
        //Allocate a tx buffer
        if (getBuffer(msgLen, (void **) &dataBuf, &dataBufLen) != SUCCESS)
        {
            printf("Unable to allocate buffer!\n");
            assert(0);
        }

        header.status |= DATA_AVAILABLE;
        header.payloadLength = len;

        header.seqNum = currentSessionPtr->txSeqNum;

        //Copy data into the tx buffer
        memcpy(dataBuf, &header, sizeof(header));
        memcpy(&dataBuf[sizeof(header)], data, len);

        pktCrc = hdlcFcs16(hdlc_init_fcs16, dataBuf, msgLen - 2);

        //Intentionally make CRCs bad
        if (haloUdpCommData.userData->dbgTestCtrls.badCrc)
            pktCrc++;

        //Copy CRC
        memcpy(&dataBuf[msgLen - 2], &pktCrc, sizeof(pktCrc));

        //Advance the sequence number
        if (haloUdpCommData.userData->dbgTestCtrls.outOfSeqTx)
            currentSessionPtr->txSeqNum = (uint16) (rand() % 65536);
        else
            currentSessionPtr->txSeqNum++;

        if (enqueue_tx_packet(txMgmt, dataBuf, msgLen, header.seqNum, currentSessionPtr->socketAddr) == SUCCESS)
        {
            result = SUCCESS;
            if (haloUdpCommData.userData->dbgTestCtrls.duplicateTx)
                enqueue_tx_packet(txMgmt, dataBuf, msgLen, header.seqNum, currentSessionPtr->socketAddr);
        }
        else
        {
            printf("Unable to place data into the tx queue!\n");
            assert(0);
        }
    }
    else
    {
        printf("WARNING: Attempting to send to session index %d, where there is no entry!\n", sessionIndex);
    }

    return result;
}

int getSessionIndex( GenericIP socketAddress, int *sessionIndex)
{
    int result = FAIL;
    int i;

    *sessionIndex = -1;

    //Checks to see if we know about this session and adds it if possible. If not possible, it drops
    for (i = 0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
    {
        if (haloUdpCommData.sessionData[i].used)
        {
            //Checks that IPs match (received data and IP/port tied to session entry)
            if ((haloUdpCommData.sessionData[i].socketAddr.address == socketAddress.address) &&
                    (haloUdpCommData.sessionData[i].socketAddr.port == socketAddress.port) )
            {
                haloUdpCommData.sessionData[i].sessionTickCount = 0; //Reset the inactivity count
                *sessionIndex = i;
                result = SUCCESS;
                break;
            }
        }
    }

    return result;
}

void resetTxSessions(void)
{
    int i;

    //Makes it so that each session only sends one packet at a time when starting a new session
    for (i = 0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
    {
        if ((haloUdpCommData.sessionData[i].used)&&(haloUdpCommData.sessionData[i].newTxSession))
        {
            haloUdpCommData.sessionData[i].txSessionState = UNINITIATED;
        }
    }
}

//Manages retransmitting a pending msg
void halo_msg_rexmit(void)
{
    const int MAX_BURST_SIZE = 1000; //5;
    static int burstOffset = 0;
    SessionData *currentSessionPtr = NULL;
    UdpCommStruct *commStruct = NULL;
    HaloUdpTxMgmt *txMgmt = NULL;
    uint8 *data;
    int dataLen;
    uint16 seqNum;
    int sessionIndex;
    GenericIP socketAddress;
    int txPktQueueSize;
    int count;
    int retries = -1;

    commStruct = &haloUdpCommData.haloUdpCommStruct;
    txMgmt     = &haloUdpCommData.txMgmt;

    if (pending_tx_packet(txMgmt))
    {
        int tempOffset;
        resetTxSessions();
        txPktQueueSize = tx_packet_queue_size(txMgmt);
        burstOffset = burstOffset % txPktQueueSize; //Makes sure the offset is in bounds
        tempOffset = burstOffset;

        //Send everything that's presently available in the queue (giant burst of UDP)
        for (count = 0; (count < MAX_BURST_SIZE)&&(count < txPktQueueSize); count++)
        {
            if (peek_tx_packet(txMgmt, burstOffset, &data, &dataLen, &seqNum, &socketAddress) == SUCCESS)
            {
                get_tx_retries(txMgmt, burstOffset, &retries);
                if (( retries < MAX_REXMIT)&&(MAX_REXMIT > 0))
                {
                    if (getSessionIndex(socketAddress, &sessionIndex) == SUCCESS)
                    {
                        int allowSend = 0;

                        currentSessionPtr = &haloUdpCommData.sessionData[sessionIndex];

                        //Modify the header to request a new session if needed
                        if (currentSessionPtr->newTxSession)
                        {
                            if ((currentSessionPtr->txSessionState == UNINITIATED)||(currentSessionPtr->txSessionState == SESSION_IN_USE))
                            {
                                currentSessionPtr->txSessionState = START_NEW_SESSION;
                            }

                            if (currentSessionPtr->txSessionState == START_NEW_SESSION)
                            {
                                MyHaloUdpHeader *header = (MyHaloUdpHeader *) data;
                                int msgLen;
                                uint16 pktCrc;

                                currentSessionPtr->txSessionState = NEW_SESSION_IN_PROGRESS;
                                header->status |= NEW_SESSION;

                                //Calculate total msg length
                                msgLen = header->payloadLength;
                                msgLen += sizeof(MyHaloUdpHeader); //header: udp data
                                msgLen += sizeof(uint16);  //tail: crc

                                //Recalculate the CRC
                                pktCrc = hdlcFcs16(hdlc_init_fcs16, data, msgLen - 2);

                                if (haloUdpCommData.userData->dbgTestCtrls.badCrc)
                                    pktCrc++;

                                //Copy CRC
                                memcpy(&data[msgLen - 2], &pktCrc, sizeof(pktCrc));
                                allowSend = 1;
                            }
                        }
                        else
                        {
                            allowSend = 1;
                        }

                        if (allowSend) //Only send when not in a new session or the first packet in a new session
                        {
                            //Data transmission happens from here
                            udp_sendto(commStruct, data, dataLen, currentSessionPtr->socketAddr);

                            incr_tx_retries(txMgmt, burstOffset);

                            //Update Statistics
                            updateTxPkts(&haloUdpCommData.stats, 1);
                            updateTxBytes(&haloUdpCommData.stats, dataLen);
                            updateTxDataPkts(&haloUdpCommData.stats, 1);
                            updateTxDataBytes(&haloUdpCommData.stats, dataLen);
                        }

                        //NOTE: To support multi-sending, newTxSession will need to be an enum type that has initial, pending, and done as states
                        //So that a given session will only send one data msg per session while a new session is being sent out
                        //As long as the state is done, multiple can be send out (for a given session).
                    }
                    else //Must be able to transmit even if the rcv entry / acknowledgement stack entry is absent
                    {
                        //TO DO: Handle what happens if you cannot find the session entry when it is time to send
                        //Create a new session entry
                        //Recompute the CRC for the pending msg
                        //How this happens: Msg gets received, tx queue is so huge / communication problems that
                        //Session entry times out from rcv inactivity, then msg finally gets sent

                        //Since all of the data necessary to send the item is already present, then data can be sent unaltered
                        //A new session can only then have a chance to be recreated when there is an entry.
                        //TO DO: Figure out how to handle new session requests (for now the answer is we don't and indicate such)
                        //when there is not an acknowledgement queue entry
                        //This is set up intentionally so that the server cannot send data from a client it much later
                        //Unless it go the info from a handler

                        //Data transmission happens from here
                        udp_sendto(commStruct, data, dataLen, socketAddress);

                        incr_tx_retries(txMgmt, burstOffset);

                        //Update Statistics
                        updateTxPkts(&haloUdpCommData.stats, 1);
                        updateTxBytes(&haloUdpCommData.stats, dataLen);
                    }
                }
            }

            burstOffset = (burstOffset + 1) % txPktQueueSize; //Increment the burst with roll over
        }

        //Cleans up any entries where the maximum transmissions have happened
        //Send everything that's presently available in the queue (giant burst of UDP)
        for (count = 0; (count < MAX_BURST_SIZE)&&(count < txPktQueueSize); count++)
        {
            if (peek_tx_packet(txMgmt, tempOffset, &data, &dataLen, &seqNum, &socketAddress) == SUCCESS)
            {
                get_tx_retries(txMgmt, tempOffset, &retries);
                if (( retries >= MAX_REXMIT)||(MAX_REXMIT <= 0))
                {
                    if (haloUdpCommData.userData->dbgTestCtrls.neverTxDrop)
                    {
                        reset_tx_retries(txMgmt, tempOffset);
                    }
                    else
                    {
                        halo_msg_tx_dropped(tempOffset);

                        //NOTE: Dropping stats could go here instead of inside halo_msg_tx_dropped

                        ////Redo the search each time tx_dropped is called because the queue positions have changed
                        tempOffset = (tempOffset + txPktQueueSize - 1) % txPktQueueSize; //Go back by one when dequeuing
                    }
                }
            }

            tempOffset = (tempOffset + 1) % txPktQueueSize; //Increment the burst with roll over
        }
    }
}

//Handler for successfully sent msgs
void halo_msg_tx_sent( GenericIP socketAddress, uint16 txSeqNum)
{
    HaloUdpTxMgmt *txMgmt = NULL;
    uint8 *data;
    int dataLen;
    uint16 seqNum;
    GenericIP pktSocketAddress;
    int i;
    int queueSize;

    txMgmt     = &haloUdpCommData.txMgmt;

    //TO DO: Make it so that navigation seeks, add generic return structure
    //Add for random dequeue
    queueSize = tx_packet_queue_size(txMgmt);
    for (i=0; i < queueSize; i++)
    {
        if (peek_tx_packet(txMgmt, i, &data, &dataLen, &seqNum, &pktSocketAddress) == SUCCESS)
        {
            if ((socketAddress.address == pktSocketAddress.address) &&
                    (socketAddress.port == pktSocketAddress.port)&&(txSeqNum == seqNum))
            {
                HaloUdpEventData sentEventData = HALO_UDP_EVENT_DATA_INIT();

                //TO DO: Redo how this drops
                dequeue_tx_packet(txMgmt, pktSocketAddress, txSeqNum); //Add function to unconditionally drop

                sentEventData.dataLength = dataLen - sizeof(MyHaloUdpHeader) - 2; //2 -> MyHaloUdpTail
                sentEventData.data = &data[sizeof(MyHaloUdpHeader)];
                sentEventData.socketAddress = pktSocketAddress;

                //Notify to user that msg has been sent
                if (haloUdpCommData.userData)
                    haloUdpCommData.userData->msg_tx_sent(&sentEventData);

                //Update Statistics
                updateTxConfirmedPkts(&haloUdpCommData.stats, 1);
                updateTxConfirmedBytes(&haloUdpCommData.stats, dataLen);

                //Return the buffer
                freeBuffer(data);

                reset_tx_retries(txMgmt, i);
                break; //Only dequeue one.
            }
        }
    }
}

//Handler for failed transmitted msgs
//TO DO: Consider having dropped being handled similar to sent.
void halo_msg_tx_dropped(int offset)
{
    HaloUdpTxMgmt *txMgmt = NULL;
    uint8 *data;
    int dataLen;
    uint16 seqNum;
    GenericIP pktSocketAddress;

    txMgmt     = &haloUdpCommData.txMgmt;

    if (peek_tx_packet(txMgmt, offset, &data, &dataLen, &seqNum, &pktSocketAddress) == SUCCESS)
    {
        HaloUdpEventData droppedEventData = HALO_UDP_EVENT_DATA_INIT();

        //TO DO: Redo how this drops (Make it so that the SeqNumber is actually used to know which packet to drop)
        dequeue_tx_packet(txMgmt, pktSocketAddress, seqNum); //Add function to unconditionally drop

        //Update Statistics
        updateTxDroppedPkts(&haloUdpCommData.stats, 1);
        //updateDroppedTxBytes(&haloUdpCommData.stats, dataLen);

        droppedEventData.dataLength = dataLen - sizeof(MyHaloUdpHeader) - 2; //2 -> MyHaloUdpTail
        droppedEventData.data = &data[sizeof(MyHaloUdpHeader)];
        droppedEventData.socketAddress = pktSocketAddress;

        //Notify to user that msg has been dropped
        haloUdpCommData.userData->msg_tx_dropped(&droppedEventData);

        //Return the buffer
        freeBuffer(data);

        if (haloUdpCommData.userData->debug)
            printf("Dropping msg! Retransmit limit exceeded!\n");
    }
}

void udp_recv_handler(void *data)
{
    SessionData *currentSessionPtr = NULL;
    HaloUdpTxMgmt *txMgmt = NULL;
    UdpEventData udpEventData = *(UdpEventData *) data;
    MyHaloUdpHeader *header = (MyHaloUdpHeader *) udpEventData.data;
    uint8 *payloadPtr;
    int payloadLength;
    uint16 pktCrc;  //CRC Includes the header and the payload to make sure everything's correct.
    uint16 calculatedCrc;
    uint8 processPkt = 1;
    int pktSessionIndex = -1;

    payloadLength = udpEventData.length;
    payloadLength -= sizeof(MyHaloUdpHeader); //Remove header size
    payloadLength -= sizeof(pktCrc);          //Remove CRC size
    payloadPtr = &udpEventData.data[sizeof(MyHaloUdpHeader)];

    //Calculate CRC
    calculatedCrc = hdlcFcs16(hdlc_init_fcs16, udpEventData.data, udpEventData.length - 2);

    if (payloadLength < 0)
    {
        if (haloUdpCommData.userData->debug)
            printf("Runt: Negative payload calculated.\n");

        //Update Statistics
        updateRxRuntPkts(&haloUdpCommData.stats, 1);

        processPkt = 0;
    }

    if(processPkt)
    {
        pktCrc = ((uint16) (payloadPtr[payloadLength] & 0xff)) | ((uint16) payloadPtr[payloadLength + 1] << 8);
        if (pktCrc != calculatedCrc)
        {
            if (haloUdpCommData.userData->debug)
                printf("CRC Error: Pkt contains incorrect CRC.\n");

            //Update Statistics
            updateRxBadCrcPkts(&haloUdpCommData.stats, 1);

            processPkt = 0;
        }
    }

    if(payloadLength < header->payloadLength)
    {
        if (haloUdpCommData.userData->debug)
            printf("Payload Error: Payload is smaller than reported.\n");

        //Update Statistics
        updateRxInvalidPayloadPkts(&haloUdpCommData.stats, 1);

        processPkt = 0;
    }

    //This simulates spotty communications by intentionally discarding good packets.
    if (haloUdpCommData.userData->dbgTestCtrls.spottyRx)
    {
        int selectedVal = (rand() % 100);

        //Try to make it fail spottyRx % of the time
        if(selectedVal < haloUdpCommData.userData->dbgTestCtrls.spottyRx)
        {
            processPkt = 0;
        }
    }

    if (processPkt)
    {
        int i;
        int sessionFound = 0;
        int firstAvailableSessionSlot = -1;

        //Update Statistics
        //Packets should be good if they reach here
        updateRxGoodPkts(&haloUdpCommData.stats, 1);
        updateRxGoodBytes(&haloUdpCommData.stats, udpEventData.length);

        //Checks to see if we know about this session and adds it if possible. If not possible, it drops
        for (i = 0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
        {
            if (haloUdpCommData.sessionData[i].used)
            {
                //Checks that IPs match (received data and IP/port tied to session entry)
                if ((udpEventData.commStruct->rcvIP.address == haloUdpCommData.sessionData[i].socketAddr.address) &&
                        (udpEventData.commStruct->rcvIP.port == haloUdpCommData.sessionData[i].socketAddr.port) )
                {
                    haloUdpCommData.sessionData[i].sessionTickCount = 0; //Reset the inactivity count
                    pktSessionIndex = i;
                    sessionFound = 1;
                    break;
                }
            }
            else
            {
                //Marks first place a new session could be added
                if (firstAvailableSessionSlot < 0)
                    firstAvailableSessionSlot = i;
            }
        }

        if (sessionFound)
        {
            //Do Nothing
        }
        else if (firstAvailableSessionSlot >= 0)
        {
            //Save IP
            haloUdpCommData.sessionData[firstAvailableSessionSlot].socketAddr = udpEventData.commStruct->rcvIP;

            //Set up the acknowledgement queue
            haloUdpCommData.sessionData[firstAvailableSessionSlot].ackStackSize  = 0;
            haloUdpCommData.sessionData[firstAvailableSessionSlot].ackStackIndex = 0;
            haloUdpCommData.sessionData[firstAvailableSessionSlot].sessionTickCount = 0;
            haloUdpCommData.sessionData[firstAvailableSessionSlot].used = 1;

            halo_msg_new_session(firstAvailableSessionSlot);

            pktSessionIndex = firstAvailableSessionSlot;
            sessionFound = 1;
        }
        else
        {
            //Drop packets if we can't handle the sessions
            processPkt = 0;
        }

    }

    if (processPkt) //Validates the data
    {
        txMgmt     = &haloUdpCommData.txMgmt;
        currentSessionPtr = &haloUdpCommData.sessionData[pktSessionIndex];
        haloUdpCommData.currentSessionIndex = pktSessionIndex; //Saves the current session index being used

        //Handles Acknowledgements
        if ((header->status & MSG_RECEIVED_ACK) == MSG_RECEIVED_ACK)
        {
            //NOTE: Right now, the receive seqNum and sessionNum don't matter for acknowledgements
            //Make sure protocol stays consistent, should this change
            //Handles with acks to data transmitted from local point
            //Ack Received
            halo_msg_tx_sent(currentSessionPtr->socketAddr, header->ackSeqNum);

            //Update Statistics
            updateRxAcks(&haloUdpCommData.stats, 1);

            //Handles confirmation of a new local session
            if ((header->status & SESSION_RESTARTED) == SESSION_RESTARTED)
            {
                if (currentSessionPtr->newTxSession)
                {
                    currentSessionPtr->newTxSession = 0;
                    currentSessionPtr->txSessionState = SESSION_IN_USE;
                }
            }
        }

        //Handles a rejected session change
        if ((header->status & SESSION_REJECTED) == SESSION_REJECTED)
        {
            if (currentSessionPtr->newTxSession)
            {
                halo_msg_new_session(pktSessionIndex); //Change the session again
            }
        }

        //Handles a New Data Msg
        if ((header->status & DATA_AVAILABLE) == DATA_AVAILABLE)
        {
            int i;
            int tmpStackIndex;
            int duplicateFound = 0;

            //Send notification of receipt of ack to server
            HaloUdpAckPkt ackPkt = HALO_UDP_ACK_PKT_INIT();
            ackPkt.header.status    |= MSG_RECEIVED_ACK;

            //Handles notification that a new remote session has started
            if ((header->status & NEW_SESSION) == NEW_SESSION)
            {
                    //Purge the acknowledgement stack and confirm start of new session
                    currentSessionPtr->ackStackIndex = 0;
                    currentSessionPtr->ackStackSize  = 0;

                    //Indicate confirmation preparing for the new remote session
                    ackPkt.header.status |= SESSION_RESTARTED;
            }

            ackPkt.header.ackSeqNum  = header->seqNum;
            ackPkt.header.seqNum     = currentSessionPtr->txSeqNum;
            ackPkt.crc = hdlcFcs16(hdlc_init_fcs16, (const uint8 *) &ackPkt, sizeof(ackPkt) - sizeof(uint16));

            //Intentionally make CRCs bad
            if (haloUdpCommData.userData->dbgTestCtrls.badCrc)
                ackPkt.crc++;

            if (!haloUdpCommData.userData->dbgTestCtrls.neverAck)
            {
                //Update Statistics
                updateTxPkts(&haloUdpCommData.stats, 1);
                updateTxBytes(&haloUdpCommData.stats, sizeof(ackPkt));
                updateTxAcks(&haloUdpCommData.stats, 1);

                if (udp_sendto(udpEventData.commStruct, (uint8 *) &ackPkt, sizeof(ackPkt), currentSessionPtr->socketAddr) > 0)
                {
                    //Advance the sequence number
                    currentSessionPtr->txSeqNum++;
                }
            }

            //Checks to see if duplicate
            i=currentSessionPtr->ackStackSize;
            tmpStackIndex = currentSessionPtr->ackStackIndex;
            while (i > 0)
            {
                tmpStackIndex = (tmpStackIndex - 1 + ACK_STACK_SIZE) % ACK_STACK_SIZE;

                if (header->seqNum == currentSessionPtr->acknowledgementStack[tmpStackIndex])
                {
                    duplicateFound = 1;

                    //Update Statistics
                    updateRxDuplicatePkts(&haloUdpCommData.stats, 1);

                    break;
                }

                i--;
            }

            if (!duplicateFound) //Send ack and store seq # if not a duplicate, otherwise ignore
            {
                HaloUdpEventData rcvEventData = HALO_UDP_EVENT_DATA_INIT();

                //Store the acknowledgement in the Acknowledgement queue for future protection
                currentSessionPtr->acknowledgementStack[currentSessionPtr->ackStackIndex] = header->seqNum;
                currentSessionPtr->ackStackIndex = (currentSessionPtr->ackStackIndex + 1) % ACK_STACK_SIZE;
                if (currentSessionPtr->ackStackSize < ACK_STACK_SIZE)
                    currentSessionPtr->ackStackSize++;

                if (haloUdpCommData.userData->debug)
                {
                    printf("Binary Structure printout: \n");

                    for (i=0; i< udpEventData.length; i++)
                    {
                        printf("%02x ", udpEventData.data[i]);
                        if ((i % 8) == 7)
                            printf("\n");
                    }
                    printf("\n");
                }

                //Notify that new data has been received
                rcvEventData.data = payloadPtr;
                rcvEventData.dataLength = payloadLength;
                rcvEventData.socketAddress = udpEventData.commStruct->rcvIP;
                haloUdpCommData.userData->msg_rx_received(&rcvEventData);

                //Update Statistics
                updateRxDataPkts(&haloUdpCommData.stats, 1);
                updateRxDataBytes(&haloUdpCommData.stats, udpEventData.length);

                if (haloUdpCommData.userData->dbgTestCtrls.loopback)
                    halo_msg_sendto((HaloMessage *) payloadPtr, payloadLength, udpEventData.commStruct->rcvIP);
            }
        }
    }
}

void halo_msg_tick(void)
{
    UdpCommStruct *commStruct = NULL;
    HaloUdpTxMgmt *txMgmt = NULL;
    int i;

    commStruct = &haloUdpCommData.haloUdpCommStruct;
    txMgmt     = &haloUdpCommData.txMgmt;

    udp_tick(commStruct);

    //Retransmission
    if ((haloUdpCommData.tickCount % TICKS_PER_REXMIT) == (TICKS_PER_REXMIT - 1))
    {
        halo_msg_rexmit();
    }

    if (haloUdpCommData.userData->actAsServer) //Only deactivate sessions if server
    {
        //Manage the life of each session
        for (i=0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
        {
            if (haloUdpCommData.sessionData[i].used)
            {
                haloUdpCommData.sessionData[i].sessionTickCount++;

                if (haloUdpCommData.sessionData[i].sessionTickCount > TICKS_PER_INACTIVE_SESSION )
                    haloUdpCommData.sessionData[i].used = 0;
            }
        }
    }

    haloUdpCommData.tickCount++;
}

HaloUdpCommDbg get_halo_udp_comm_dbg(void)
{
    return haloUdpCommData.userData->dbgTestCtrls;
}

void set_halo_udp_comm_dbg(HaloUdpCommDbg dbgTestCtrls)
{
    haloUdpCommData.userData->dbgTestCtrls = dbgTestCtrls;
}

int halo_msg_session_count(void)
{
    int i;
    int count = 0;

    for (i=0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
    {
        if (haloUdpCommData.sessionData[i].used)
        {
            count++;
        }
    }

    return count;
}

void halo_msg_report_session(int offset)
{
    int i;
    int count = 0;

    for (i=0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
    {
        if (haloUdpCommData.sessionData[i].used)
        {
            if (count == offset)
            {
                printf("sessionIndex: %d\n", i);
                printf("IP: 0x%x\n", haloUdpCommData.sessionData[i].socketAddr.address);
                printf("Port: %d\n", haloUdpCommData.sessionData[i].socketAddr.port);
                printf("ticks inactive: %d\n", haloUdpCommData.sessionData[i].sessionTickCount);
                printf("\n");
            }
            count++;
        }
    }
}

void halo_msg_cleanup(void)
{
    UdpCommStruct *commStruct = NULL;

    commStruct = &haloUdpCommData.haloUdpCommStruct;

    udp_cleanup(commStruct);
}

void halo_msg_report_stats()
{
    HaloUdpStats *stats = &haloUdpCommData.stats;

	printf("txBytes             : %u\n", getTxBytes(stats));
	printf("txConfirmedBytes    : %u\n", getTxConfirmedBytes(stats));
	printf("txPkts              : %u\n", getTxPkts(stats));
	printf("txConfirmedPkts     : %u\n", getTxConfirmedPkts(stats));
	printf("txDroppedPkts       : %u\n", getTxDroppedPkts(stats));
	printf("txAcks              : %u\n", getTxAcks(stats));
	printf("txDataPkts          : %u\n", getTxDataPkts(stats));
	printf("txDataBytes         : %u\n", getTxDataBytes(stats));

	printf("rxGoodBytes         : %u\n", getRxGoodBytes(stats));
	printf("rxGoodPkts          : %u\n", getRxGoodPkts(stats));
	printf("rxBadCrcPkts        : %u\n", getRxBadCrcPkts(stats));
	printf("rxDuplicatePkts     : %u\n", getRxDuplicatePkts(stats));
	printf("rxRuntPkts          : %u\n", getRxRuntPkts(stats));
	printf("rxInvalidPayloadPkts: %u\n", getRxInvalidPayloadPkts(stats));
	printf("rxAcks              : %u\n", getRxAcks(stats));
	printf("rxDataPkts          : %u\n", getRxDataPkts(stats));
	printf("rxDataBytes         : %u\n", getRxDataBytes(stats));
}

void halo_msg_reset_stats()
{
    resetHaloUdpStats(&haloUdpCommData.stats);
}

HaloUdpStats get_halo_msg_stats()
{
    return haloUdpCommData.stats;
}
