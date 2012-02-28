log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;

exports.MyHaloUdpTail = MyHaloUdpTail;
function MyHaloUdpTail()
{
    //Members
    this.crc = 0;

    //Getters/Setters
    this.getCrc = function ()
    {
        return this.crc;
    }

    this.setCrc = function(crc) {
        this.crc = crc;
    }

    //Byte Processing
    this.getLength = function ()
    {
        return MyHaloUdpTail.MIN_LENGTH;
    }

    this.encode = function()
    {
        var buf = new Buffer(this.getLength());
        var tmp = null;

        tmp = ByteUtilities.uint16ToBytes_LE(this.crc);
        buf[0] = tmp[0];
        buf[1] = tmp[1];

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
        else if(buf.length >= MyHaloUdpTail.MIN_LENGTH)
        {
            this.crc = ByteUtilities.bytesToUint16_LE(buf,offset);
            
            success = true;
        }

        return success;
    }
}

//Constants
MyHaloUdpTail.MIN_LENGTH = 2;
