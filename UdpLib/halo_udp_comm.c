#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "myHaloUdp.h"
#include "halo_udp_comm.h"
#include "udp_lib.h"
#include "tx_buffer.h"
#include "../HaloMsgHelperFunctions.h"
#include "halo_udp_tx_mgmt.h"
#include "crc16.h"

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
    struct sockaddr_in socketAddr;
    socklen_t socketAddrLen;
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
 .socketAddr = {0}, \
 .socketAddrLen  = 0, \
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
}
HaloUdpCommData;

#define HALO_UDP_COMM_DATA_INIT() { \
 .haloUdpCommStruct = {0}, \
 .txMgmt = HALO_UDP_TX_MGMT_INIT(), \
 .tickCount = 0, \
 .userData  = NULL, \
 .currentSessionIndex = 0, \
}

typedef struct
{
    MyHaloUdpHeader header;
    uint16 crc;
}
HaloUdpAckPkt;

//Stores the data used to handle the halo UDP communications stack
static HaloUdpCommData haloUdpCommData = HALO_UDP_COMM_DATA_INIT();

//Function Prototypes
void udp_recv_handler(void *data);
void halo_msg_rexmit(void);
int getSessionIndex( struct sockaddr_in socketAddress, socklen_t socketAddressLength, int *sessionIndex);

//Handler for successfully sent msgs
void halo_msg_tx_sent( struct sockaddr_in socketAddress, socklen_t socketAddressLength, uint16 txSeqNum);

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
        haloUdpCommData.sessionData[haloUdpCommData.currentSessionIndex].socketAddr = commStruct->socketAddress;
        haloUdpCommData.sessionData[haloUdpCommData.currentSessionIndex].socketAddrLen = commStruct->socketAddressLength;
    }

    //Debug TO DO: Figure out a better way of handling this. (New Session should not require you to know the session index)
    halo_msg_new_session(haloUdpCommData.currentSessionIndex);
}

//NOTE: All sending and receiving of data needs to be halted when this is called
//to keep packet information correct and in-sync without loss.
void halo_msg_new_session(int sessionIndex)
{
    SessionData *currentSessionPtr = NULL;
    uint8 sessionNum;
    HaloUdpTxMgmt *txMgmt = NULL;
    uint8 *data;
    int dataLen;
    uint16 seqNum;
    struct sockaddr_in socketAddress;
    socklen_t socketAddressLength;
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
            if (peek_tx_packet(txMgmt, i, &data, &dataLen, &seqNum, &socketAddress, &socketAddressLength) == SUCCESS)
            {
                if ((currentSessionPtr->socketAddr.sin_addr.s_addr == socketAddress.sin_addr.s_addr) &&
                        (currentSessionPtr->socketAddr.sin_port == socketAddress.sin_port) )
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
                    dequeue_tx_packet(txMgmt, socketAddress, socketAddressLength, seqNum); //removes with old seq num
                    enqueue_tx_packet(txMgmt, data, dataLen, header->seqNum, socketAddress, socketAddressLength); //Adds new seq num
                }
            }
        }
    }
}

//NOTE: Access in the send queue needs to be thread-protected so that additions/ removal are locked
int halo_msg_sendto(const HaloMessage *msg,
                    struct sockaddr_in *sockAddrPtr, socklen_t sockAddrLen)
{
    int result = FAIL;
    int sessionIndex;

    if (getSessionIndex(*sockAddrPtr, sockAddrLen, &sessionIndex) == SUCCESS)
    {
        result = halo_msg_send_to_index(msg, sessionIndex);
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
                haloUdpCommData.sessionData[firstAvailableSessionSlot].socketAddr = *sockAddrPtr;
                haloUdpCommData.sessionData[firstAvailableSessionSlot].socketAddrLen = sockAddrLen;

                //Set up the acknowledgement queue
                haloUdpCommData.sessionData[firstAvailableSessionSlot].ackStackSize  = 0;
                haloUdpCommData.sessionData[firstAvailableSessionSlot].ackStackIndex = 0;
                haloUdpCommData.sessionData[firstAvailableSessionSlot].sessionTickCount = 0;
                haloUdpCommData.sessionData[firstAvailableSessionSlot].used = 1;

                halo_msg_new_session(firstAvailableSessionSlot);

                //Send using the new entry
                result = halo_msg_send_to_index(msg, firstAvailableSessionSlot);
                break;
            }
        }
    }

    return result;
}

int halo_msg_send(const HaloMessage *msg)
{
    return halo_msg_send_to_index(msg, haloUdpCommData.currentSessionIndex);
}

int halo_msg_send_to_index(const HaloMessage *msg, int sessionIndex)
{
    SessionData *currentSessionPtr = NULL;
    MyHaloUdpHeader header;
    UdpCommStruct *commStruct = NULL;
    HaloUdpTxMgmt *txMgmt = NULL;
    int msgLen;
    int payloadLen;
    uint8 *data;
    int dataLen;
    uint16 pktCrc;
    int result = FAIL;

    commStruct = &haloUdpCommData.haloUdpCommStruct;
    txMgmt     = &haloUdpCommData.txMgmt;
    payloadLen = getMsgLength(msg);

    msgLen = payloadLen;
    //Update the msg length
    msgLen += sizeof(MyHaloUdpHeader); //header: udp data
    msgLen += sizeof(uint16);  //tail: crc

    currentSessionPtr = &haloUdpCommData.sessionData[sessionIndex];

    if (currentSessionPtr->used)
    {
        //Allocate a tx buffer
        if (getBuffer(msgLen, (void **) &data, &dataLen) != SUCCESS)
        {
            printf("Unable to allocate buffer!\n");
            assert(0);
        }

        //Fill header with all zeroes
        memset(&header, 0, sizeof(header));
        header.status |= DATA_AVAILABLE;
        header.payloadLength = payloadLen;

        header.seqNum = currentSessionPtr->txSeqNum;

        //Copy data into the tx buffer
        memcpy(data, &header, sizeof(header));
        memcpy(&data[sizeof(header)], msg, payloadLen);

        pktCrc = hdlcFcs16(hdlc_init_fcs16, data, msgLen - 2);

        //Intentionally make CRCs bad
        if (haloUdpCommData.userData->dbgTestCtrls.badCrc)
            pktCrc++;

        //Copy CRC
        memcpy(&data[msgLen - 2], &pktCrc, sizeof(pktCrc));

        //Advance the sequence number
        if (haloUdpCommData.userData->dbgTestCtrls.outOfSeqTx)
            currentSessionPtr->txSeqNum = (uint16) (rand() % 65536);
        else
            currentSessionPtr->txSeqNum++;

        if (enqueue_tx_packet(txMgmt, data, msgLen, header.seqNum, currentSessionPtr->socketAddr, currentSessionPtr->socketAddrLen) == SUCCESS)
        {
            result = SUCCESS;
            if (haloUdpCommData.userData->dbgTestCtrls.duplicateTx)
                enqueue_tx_packet(txMgmt, data, msgLen, header.seqNum, currentSessionPtr->socketAddr, currentSessionPtr->socketAddrLen);
        }
        else
        {
            printf("Unable to place data into the tx queue!\n");
            assert(0);
        }
    }

    return result;
}

int getSessionIndex( struct sockaddr_in socketAddress, socklen_t socketAddressLength, int *sessionIndex)
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
            if ((haloUdpCommData.sessionData[i].socketAddr.sin_addr.s_addr == socketAddress.sin_addr.s_addr) &&
                    (haloUdpCommData.sessionData[i].socketAddr.sin_port == socketAddress.sin_port) )
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
    const int MAX_BURST_SIZE = 5;
    static int burstOffset = 0;
    SessionData *currentSessionPtr = NULL;
    UdpCommStruct *commStruct = NULL;
    HaloUdpTxMgmt *txMgmt = NULL;
    uint8 *data;
    int dataLen;
    uint16 seqNum;
    int sessionIndex;
    struct sockaddr_in socketAddress;
    socklen_t socketAddressLength;
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
            if (peek_tx_packet(txMgmt, burstOffset, &data, &dataLen, &seqNum, &socketAddress, &socketAddressLength) == SUCCESS)
            {
                get_tx_retries(txMgmt, burstOffset, &retries);
                if (( retries < MAX_REXMIT)&&(MAX_REXMIT > 0))
                {
                    if (getSessionIndex(socketAddress, socketAddressLength, &sessionIndex) == SUCCESS)
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
                            udp_sendto(commStruct, data, dataLen, &currentSessionPtr->socketAddr, currentSessionPtr->socketAddrLen);

                            incr_tx_retries(txMgmt, burstOffset);
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
                        udp_sendto(commStruct, data, dataLen, &socketAddress, socketAddressLength);

                        incr_tx_retries(txMgmt, burstOffset);
                    }
                }
            }

            burstOffset = (burstOffset + 1) % txPktQueueSize; //Increment the burst with roll over
        }

        //Cleans up any entries where the maximum transmissions have happened
        //Send everything that's presently available in the queue (giant burst of UDP)
        for (count = 0; (count < MAX_BURST_SIZE)&&(count < txPktQueueSize); count++)
        {
            if (peek_tx_packet(txMgmt, tempOffset, &data, &dataLen, &seqNum, &socketAddress, &socketAddressLength) == SUCCESS)
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
void halo_msg_tx_sent( struct sockaddr_in socketAddress, socklen_t socketAddressLength, uint16 txSeqNum)
{
    HaloUdpTxMgmt *txMgmt = NULL;
    uint8 *data;
    int dataLen;
    uint16 seqNum;
    struct sockaddr_in pktSocketAddress;
    socklen_t pktSocketAddressLength;
    int i;
    int queueSize;

    txMgmt     = &haloUdpCommData.txMgmt;

    //TO DO: Make it so that navigation seeks, add generic return structure
    //Add for random dequeue
    queueSize = tx_packet_queue_size(txMgmt);
    for (i=0; i < queueSize; i++)
    {
        if (peek_tx_packet(txMgmt, i, &data, &dataLen, &seqNum, &pktSocketAddress, &pktSocketAddressLength) == SUCCESS)
        {
            if ((socketAddress.sin_addr.s_addr == pktSocketAddress.sin_addr.s_addr) &&
                    (socketAddress.sin_port == pktSocketAddress.sin_port)&&(txSeqNum == seqNum))
            {
                //TO DO: Redo how this drops
                dequeue_tx_packet(txMgmt, pktSocketAddress, pktSocketAddressLength, txSeqNum); //Add function to unconditionally drop

                //Notify to user that msg has been sent
                if (haloUdpCommData.userData)
                    haloUdpCommData.userData->msg_tx_sent(&data[sizeof(MyHaloUdpHeader)]);

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
    struct sockaddr_in pktSocketAddress;
    socklen_t pktSocketAddressLength;

    txMgmt     = &haloUdpCommData.txMgmt;

    if (peek_tx_packet(txMgmt, offset, &data, &dataLen, &seqNum, &pktSocketAddress, &pktSocketAddressLength) == SUCCESS)
    {
        //TO DO: Redo how this drops (Make it so that the SeqNumber is actually used to know which packet to drop)
        dequeue_tx_packet(txMgmt, pktSocketAddress, pktSocketAddressLength, seqNum); //Add function to unconditionally drop

        //Notify to user that msg has been dropped
        haloUdpCommData.userData->msg_tx_dropped(&data[sizeof(MyHaloUdpHeader)]);

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
    UdpRecvArgs args = *(UdpRecvArgs *) data;
    MyHaloUdpHeader *header = (MyHaloUdpHeader *) args.data;
    uint8 *payloadPtr;
    int payloadLength;
    uint16 pktCrc;  //CRC Includes the header and the payload to make sure everything's correct.
    uint16 calculatedCrc;
    uint8 processPkt = 1;
    int pktSessionIndex = -1;

    payloadLength = args.length;
    payloadLength -= sizeof(MyHaloUdpHeader); //Remove header size
    payloadLength -= sizeof(pktCrc);          //Remove CRC size
    payloadPtr = &args.data[sizeof(MyHaloUdpHeader)];

    pktCrc = ((uint16) (payloadPtr[payloadLength] & 0xff)) | ((uint16) payloadPtr[payloadLength + 1] << 8);

    //Calculate CRC
    calculatedCrc = hdlcFcs16(hdlc_init_fcs16, args.data, args.length - 2);

    if (payloadLength < 0)
    {
        if (haloUdpCommData.userData->debug)
            printf("Runt: Negative payload calculated.\n");
        processPkt = 0;
    }

    if (pktCrc != calculatedCrc)
    {
        if (haloUdpCommData.userData->debug)
            printf("CRC Error: Pkt contains incorrect CRC.\n");
        processPkt = 0;
    }

    if (processPkt)
    {
        int i;
        int sessionFound = 0;
        int firstAvailableSessionSlot = -1;

        //Checks to see if we know about this session and adds it if possible. If not possible, it drops
        for (i = 0; i < MAX_CONCURRENT_CONNNECTIONS; i++)
        {
            if (haloUdpCommData.sessionData[i].used)
            {
                //Checks that IPs match (received data and IP/port tied to session entry)
                if ((args.commStruct->rcvAddrPtr->sin_addr.s_addr == haloUdpCommData.sessionData[i].socketAddr.sin_addr.s_addr) &&
                        (args.commStruct->rcvAddrPtr->sin_port == haloUdpCommData.sessionData[i].socketAddr.sin_port) )
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
            haloUdpCommData.sessionData[firstAvailableSessionSlot].socketAddr = *args.commStruct->rcvAddrPtr;
            haloUdpCommData.sessionData[firstAvailableSessionSlot].socketAddrLen = *args.commStruct->rcvAddrLenPtr;

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
            halo_msg_tx_sent(currentSessionPtr->socketAddr, currentSessionPtr->socketAddrLen, header->ackSeqNum);

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
            HaloUdpAckPkt ackPkt;
            memset(&ackPkt.header, 0, sizeof(ackPkt.header));
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
            ackPkt.crc = hdlcFcs16(hdlc_init_fcs16, &ackPkt, sizeof(ackPkt) - sizeof(uint16));

            //Intentionally make CRCs bad
            if (haloUdpCommData.userData->dbgTestCtrls.badCrc)
                ackPkt.crc++;

                if (!haloUdpCommData.userData->dbgTestCtrls.neverAck)
                {
                    if (udp_sendto(args.commStruct, (uint8 *) &ackPkt, sizeof(ackPkt), &currentSessionPtr->socketAddr, currentSessionPtr->socketAddrLen) > 0)
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
                        break;
                    }

                    i--;
                }

                if (!duplicateFound) //Send ack and store seq # if not a duplicate, otherwise ignore
                {
                    HaloUdpRcvEventData rcvEventData = HALO_UDP_RCV_EVENT_DATA_INIT();

                    //Store the acknowledgement in the Acknowledgement queue for future protection
                    currentSessionPtr->acknowledgementStack[currentSessionPtr->ackStackIndex] = header->seqNum;
                    currentSessionPtr->ackStackIndex = (currentSessionPtr->ackStackIndex + 1) % ACK_STACK_SIZE;
                    if (currentSessionPtr->ackStackSize < ACK_STACK_SIZE)
                        currentSessionPtr->ackStackSize++;

                    if (haloUdpCommData.userData->debug)
                    {
                        printf("Binary Structure printout: \n");

                        for (i=0; i< args.length; i++)
                        {
                            printf("%02x ", args.data[i]);
                            if ((i % 8) == 7)
                                printf("\n");
                        }
                        printf("\n");
                    }

                    //Notify that new data has been received
                    rcvEventData.data = payloadPtr;
                    rcvEventData.dataLength = payloadLength;
                    rcvEventData.socketAddress = *args.commStruct->rcvAddrPtr;
                    rcvEventData.socketAddressLength = *args.commStruct->rcvAddrLenPtr;
                    haloUdpCommData.userData->msg_rx_received(&rcvEventData);

                    if (haloUdpCommData.userData->dbgTestCtrls.loopback)
                        halo_msg_sendto(payloadPtr, args.commStruct->rcvAddrPtr, *args.commStruct->rcvAddrLenPtr);
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
                printf("IP: 0x%x\n", haloUdpCommData.sessionData[i].socketAddr.sin_addr.s_addr);
                printf("Port: %d\n", haloUdpCommData.sessionData[i].socketAddr.sin_port);
                printf("ticks inactive: %d\n", haloUdpCommData.sessionData[i].sessionTickCount);
                printf("\n");
            }
            count++;
        }
    }
}

void halo_msg_cleanup(void)
{
}
