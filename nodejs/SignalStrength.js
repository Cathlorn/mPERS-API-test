log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;

exports.SignalStrength = SignalStrength;
function SignalStrength()
{
    //Members
    this.cellularStrength  = 0; //4 bytes

    //Getters/Setters
    this.getCellularStrength = function() {
        return this.cellularStrength;
    }

    this.setCellularStrength = function(cellularStrength) {
        this.cellularStrength = cellularStrength;
    }

    //Byte Processing
    this.getLength = function ()
    {
        return SignalStrength.MIN_LENGTH;
    }

    this.encode = function()
    {
        var buf = new Buffer(this.getLength());
        var tmp = null;

        tmp = ByteUtilities.uint32ToBytes_LE(this.cellularStrength);
        buf[ 0] = tmp[0];
        buf[ 1] = tmp[1];
        buf[ 2] = tmp[2];
        buf[ 3] = tmp[3];

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
        else if(buf.length >= SignalStrength.MIN_LENGTH)
        {
            this.cellularStrength = ByteUtilities.bytesToUint32_LE(buf,offset);

            success = true;
        }

        return success;
    }
}

//Constants
SignalStrength.MIN_LENGTH = 4;
