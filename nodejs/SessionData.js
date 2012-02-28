log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;
var Queue = require('./Queue').Queue;

exports.SessionData = SessionData;

function SequenceNumberEntry(seqNum) {
    this.seqNum = seqNum;
    this.equals = function(sequenceNumberEntry) {
        return (this.seqNum == sequenceNumberEntry.seqNum);
    }
}

function SessionData(address,port)
{
    //Members
    TxSessionState = {
        UNKNOWN : -1,
        UNINITIATED : 0,
        START_NEW_SESSION : 1,
        NEW_SESSION_IN_PROGRESS : 2,
        SESSION_IN_USE : 3
    }
    
    this.acknowledgementQueue = new Queue();
    this.address = address;
    this.port = port;
    this.ticksInactive = 0;
    this.fixed = 0;
    this.txSeqNum = 0;
    this.newTxSession = true; // Indicates that a new session is starting
    this.newRxSession = false; // Indicates that remote end is waiting for an
                               // acknowledgment
    this.txSessionState = TxSessionState.UNINITIATED;

    //Getters/Setters
    this.getTicksInactive = function() {
        return this.ticksInactive;
    }

    this.resetTicksInactive = function() {
        this.ticksInactive = 0;
    }

    this.incrementTicksInactive = function() {
        this.ticksInactive++;
    }

    this.getFixed = function() {
        return this.fixed;
    }

    this.setFixed = function(fixed) {
        this.fixed = fixed;
    }

    this.getNextTxSeqNum = function() {
        return this.txSeqNum++;
    }

    //Session Mgmt Functions
    // Use this to start a new myHaloUDP session
    this.startNewTxSession = function() {
        this.txSessionState = TxSessionState.START_NEW_SESSION;
        this.clearAcknowledgementQueue();
        this.newTxSession = true;
                log('new tx session started');
    }

    // Use this to reset the state machine when a new session is pending
    this.resetNewTxSessionState = function() {
        if (this.newTxSession) {
            this.txSessionState = TxSessionState.START_NEW_SESSION;
        }
    }

    // Use this to advance the state machine when notification has been sent to
    // the remote target
    // of a new session request
    this.requestedNewTxSession = function() {
        if (this.newTxSession) {
            this.txSessionState = TxSessionState.NEW_SESSION_IN_PROGRESS;
        }
    }

    // Use this to advance state machine when confirmation has been received
    // from the remote target
    // of the new session request
    this.newTxSessionComplete = function() {
        this.newTxSession = false;
        this.txSessionState = TxSessionState.SESSION_IN_USE;
        log('new tx session complete');
    }

    // Indicates if new session handshaking is taking place
    this.newTxSessionPending = function() {
        return this.newTxSession;
    }

    this.allowNewTxSessionNotification = function() {
        var allowNotification = false;

        if (this.newTxSession) {
            allowNotification = (this.txSessionState == TxSessionState.START_NEW_SESSION);
        }

        return allowNotification;
    }

    this.requestedNewRxSession = function() {
        if (!this.newRxSession) {
            this.newRxSession = true;
        }
    }

    this.newRxSessionComplete = function() {
        this.newRxSession = false;
        this.clearAcknowledgementQueue();
    }

    // Indicates if new session handshaking is taking place
    this.newRxSessionPending = function() {
        return this.newRxSession;
    }

    // Returns whether an acknowledgement has already been received for a given
    // sequence number
    this.rxSequenceNumberFound = function(seqNum) {
        var found = false;

        if (this.acknowledgementQueue.contains(new SequenceNumberEntry(seqNum))) {
            found = true;
        }

        return found;
    }

    // This non-threadsafe function is used to clearing removing all entries of
    // the acknowledgement stack
    // This primarily should be used when starting a new session
    this.clearAcknowledgementQueue = function() {
        this.acknowledgementQueue.clear();
    }

    // Updates the acknowledgement queue to include a new sequence number
    // received
    this.addRxSequenceNumber = function(seqNum) {
        var added = false;
        var sequenceNumberEntry = new SequenceNumberEntry(seqNum);

        if (!this.acknowledgementQueue.contains(sequenceNumberEntry)) {
            //Remove the least recent seqNum from the queue to keep the size constant
            if(this.acknowledgementQueue.size() >= SessionData.MAX_ACK_QUEUE_SIZE)
            {
                this.acknowledgementQueue.remove();
            }
            // Add back to the stack
            this.acknowledgementQueue.add(sequenceNumberEntry);
            added = true;
        }

        return added;
    }
}

//Constants
SessionData.MAX_ACK_QUEUE_SIZE = 128; //Maximum amount of entries to allow in the queue
