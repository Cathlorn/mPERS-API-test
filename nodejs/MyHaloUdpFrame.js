log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;
var Crc16 = require('./crc16').crc16;
var MyHaloUdpHeader = require('./MyHaloUdpHeader').MyHaloUdpHeader;
var MyHaloUdpTail = require('./MyHaloUdpTail').MyHaloUdpTail;

exports.MyHaloUdpFrame = MyHaloUdpFrame;
exports.MyHaloUdpEnums = MyHaloUdpEnums;

function MyHaloUdpEnums() {}
MyHaloUdpEnums.MyHaloUdpStatusBits = {
    MSG_RECEIVED_ACK : 1,  //Indicates that a message was received
    DATA_AVAILABLE :  2,   //Indicates that there is a new message in this datagram
    NEW_SESSION : 4,       //Indicates that a new session is starting and all old state date needs to be cleared
    SESSION_RESTARTED : 8, //Confirms that a session was successfully restarted
    SESSION_REJECTED : 16  //Rejects the proposed session number
}

function MyHaloUdpFrame()
{
    //Members
    this.currentHeader = null;
    this.payload = null;
    this.currentTail = null;

    //Getters/Setters
    this.getHeader = function()
    {
        if(this.currentHeader == null)
        {
            this.currentHeader = new MyHaloUdpHeader();
        }

        return this.currentHeader;
    }

    this.setHeader = function(header)
    {
        if(header != null)
        {
            this.currentHeader = header;
        }
    }

    this.getPayload = function() {
        return this.payload;
    }

    this.setPayload = function(payload) {
        this.payload = payload;
    }

    this.getTail = function()
    {
        if(this.currentTail == null)
        {
            this.currentTail = new MyHaloUdpTail();
        }

        return this.currentTail;
    }

    this.setTail = function(tail)
    {
        if(tail != null)
        {
            this.currentTail = tail;
        }
    }

    //Byte Processing
    this.getLength = function ()
    {
        var payloadLength = this.getPayloadLength();

        return this.getHeader().getLength() + payloadLength + this.getTail().getLength();
    }

    this.getPayloadLength = function() {
        var payloadLength = 0;

        if(this.payload == null)
        {
            payloadLength = 0;
        }
        else
        {
            payloadLength = this.payload.length;
        }

        return payloadLength;
    }

    this.encode = function(optionalHeader)
    {
        var buf = new Buffer(this.getLength());
        var offset = 0;

        //Header
        var header = this.getHeader();
        
        if(optionalHeader)
        {
            header = optionalHeader;
        }
        else
        {
            //Auto populate header params here

            header.setVersion(MyHaloUdpFrame.CURRENT_VERSION);

            //Manage status bits   
            header.setStatus(0); //clears all the bits to start with
        }

        //Makes sure the data available flag is always set if there is a payload
        if(this.getPayloadLength() > 0)
        {
            header.setPayloadLength(this.getPayloadLength());
            header.setStatusBits(MyHaloUdpEnums.MyHaloUdpStatusBits.DATA_AVAILABLE);
        }
        else
        {
            header.setPayloadLength(0);
        }

        //Update current header
        this.setHeader(header);

        var headerBuf = header.encode();
        for(var i = 0; i < headerBuf.length; i++)
        {
            buf[offset++] = headerBuf[i];
        }

        //Payload
        var payload = this.payload;
        for(var i = 0; i < this.getPayloadLength(); i++)
        {
            buf[offset++] = payload[i];
        }

        //Tail
        var tail = this.getTail();

        //Calculate CRC Here
        var crc = Crc16.calculateCrc(buf, 0, offset);

        //Update current tail
        tail.setCrc(crc);
        this.setTail(tail);

        //Add tail to frame
        var tailBuf = tail.encode();
        for(var i = 0; i < tailBuf.length; i++)
        {
            buf[offset++] = tailBuf[i];
        }

        return buf;
    }

    this.decode = function(buf,offset)
    {
        var success = false;
        var tmp = null;
        var MIN_PKT_SIZE = MyHaloUdpFrame.MIN_LENGTH;

        if((buf.length - offset) < MIN_PKT_SIZE)
        {
            log('MyHaloUDPFrame: Packet is too short!');
        }
        else
        {
            var header = this.getHeader();

            if(header.decode(buf,offset))
            {
                var payloadOffset = offset + header.getLength();
                var payloadLength = header.getPayloadLength();
                var tail = this.getTail();
                var tailOffset = payloadOffset + payloadLength;

                //Sets Tail
                if(tail.decode(buf,tailOffset))
                {
                    var calculatedCrc = Crc16.calculateCrc(buf,offset, tailOffset);
                    var pktCrc = tail.getCrc();

                    //Check CRC
                    if(pktCrc == calculatedCrc)
                    {
                        this.payload = new Buffer(payloadLength);

                        if((buf.length - offset - MIN_PKT_SIZE) < payloadLength)
                        {
                            log('MyHaloUDPFrame: Invalid Payload Length!');
                        }
                        else
                        {
                            //Sets Payload
                            for(var i = 0; i < this.payload.length; i++ )
                            {
                                this.payload[i] = buf[payloadOffset + i];
                            }
                            success = true;
                        }
                    }
                }
            }
        }

        return success;
    }
}

//Constants
MyHaloUdpFrame.MIN_LENGTH = MyHaloUdpHeader.MIN_LENGTH 
                            + MyHaloUdpTail.MIN_LENGTH;

MyHaloUdpFrame.CURRENT_VERSION = 2;

