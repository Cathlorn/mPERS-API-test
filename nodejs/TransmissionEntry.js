log = require('util').log
var ByteUtilities = require('./ByteUtils').ByteUtils;

exports.TransmissionEntry = TransmissionEntry;

function TransmissionEntry(dstAddr,dstPrt,seqNum,data,sock) {
    //Members
    this.timesTransmitted = 0;
    this.address = dstAddr;
    this.port = dstPrt;
    this.seqNum = seqNum;
    this.data = data;
    this.sock = sock;

    //Getters/Setters
    this.getTimesTransmitted = function() {
        return this.timesTransmitted;
    }

    this.setTimesTransmitted = function(timesTransmitted) {
        this.timesTransmitted = timesTransmitted;
    }

    this.incrementTimesTransmitted = function() {
        this.timesTransmitted++;
    }

    this.getDstAddress = function() {
        return this.address;
    }

    this.setDstAddress = function(address) {
        this.address = address;
    }

    this.getDstPort = function() {
        return this.port;
    }

    this.setDstPort = function(port) {
        this.port = port;
    }

    this.getSeqNum = function() {
        return this.seqNum;
    }

    this.setSeqNum = function(seqNum) {
        this.seqNum = seqNum;
    }

    this.getData = function() {
        return this.data;
    }

    this.setData = function(data) {
        this.data = data;
    }

    this.getSocket = function() {
        return this.sock;
    }

    this.setSocket = function(sock) {
        this.sock = sock;
    }

    this.pktMatch = function(address,port,seqNum) {

        return ((this.address == address)
        &&(this.port == port)
        &&(this.seqNum == seqNum));
    }
}
