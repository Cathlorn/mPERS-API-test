log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;

exports.MyHaloUdpHeader = MyHaloUdpHeader;
function MyHaloUdpHeader()
{
    //Members
    this.version = 1;
    this.status = 0;
    this.seqNum = 0;
    this.ackSeqNum = 0;
    this.payloadLength = 0;

    //Getters/Setters
    this.getVersion = function() {
        return this.version;
    }

    this.setVersion = function(version) {
        this.version = version;
    }

    this.getStatus = function() {
        return this.status;
    }

    this.setStatus = function(status) {
        this.status = status;
    }

    //Modifies status bits instead of replacing
    this.setStatusBits = function(statusBits) {
        this.status |= statusBits;
    }

    this.clrStatusBits = function(statusBits) {
        this.status &= ~statusBits;
    }

    this.getSeqNum = function() {
        return this.seqNum;
    }

    this.setSeqNum = function(seqNum) {
        this.seqNum = seqNum;
    }

    this.getAckSeqNum = function() {
        return this.ackSeqNum;
    }

    this.setAckSeqNum = function(ackSeqNum) {
        this.ackSeqNum = ackSeqNum;
    }

    this.getPayloadLength = function() {
        return this.payloadLength;
    }

    this.setPayloadLength = function(payloadLength) {
        this.payloadLength = payloadLength;
    }

    //Byte Processing
    this.getLength = function ()
    {
        return MyHaloUdpHeader.MIN_LENGTH;
    }

    this.encode = function()
    {
        var buf = new Buffer(this.getLength());
        var tmp = null;

        buf[0] = this.version;
        buf[1] = this.status;

        tmp = ByteUtilities.uint16ToBytes_LE(this.seqNum);
        buf[2] = tmp[0];
        buf[3] = tmp[1];

        tmp = ByteUtilities.uint16ToBytes_LE(this.ackSeqNum);
        buf[4] = tmp[0];
        buf[5] = tmp[1];

        tmp = ByteUtilities.uint16ToBytes_LE(this.payloadLength);
        buf[6] = tmp[0];
        buf[7] = tmp[1];

        return buf;
    }

    this.decode = function(buf,offset)
    {
        var success = false;
        var tmp = null;

        if(buf == null)
        {
            //Do Nothing
        }
        else if(buf.length >= MyHaloUdpHeader.MIN_LENGTH)
        {
            this.version = buf[offset + 0];
            this.status  = buf[offset + 1];
            this.seqNum = ByteUtilities.bytesToUint16_LE(buf,offset + 2);
            this.ackSeqNum = ByteUtilities.bytesToUint16_LE(buf,offset + 4);
            this.payloadLength = ByteUtilities.bytesToUint16_LE(buf,offset + 6);

            success = true;
        }

        return success;
    }
}

//Constants
MyHaloUdpHeader.MIN_LENGTH = 8;
