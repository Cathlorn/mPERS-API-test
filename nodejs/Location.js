log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;

exports.Location = Location;
function Location()
{
    //Members
    this.latitude  = 0; //4 bytes
    this.longitude = 0; //4 bytes
    this.elevation = 0; //4 bytes
    this.accuracy  = 0; //2 bytes
    this.deltaElevation = 0; //2 bytes

    //Getters/Setters
    this.getLatitude = function() {
        return this.latitude;
    }

    this.setLatitude = function(latitude) {
        this.latitude = latitude;
    }

    this.getLongitude = function() {
        return this.longitude;
    }

    this.setLongitude = function(longitude) {
        this.longitude = longitude;
    }
    
    this.getElevation = function() {
        return this.elevation;
    }

    this.setElevation = function(elevation) {
        this.elevation = elevation;
    }
    
    this.getAccuracy = function() {
        return this.accuracy;
    }

    this.setAccuracy = function(accuracy) {
        this.accuracy = accuracy;
    }
    
    this.getDeltaElevation = function() {
        return this.deltaElevation;
    }

    this.setDeltaElevation = function(deltaElevation) {
        this.deltaElevation = deltaElevation;
    }

    //Byte Processing
    this.getLength = function ()
    {
        return Location.MIN_LENGTH;
    }

    this.encode = function()
    {
        var buf = new Buffer(this.getLength());
        var tmp = null;

        tmp = ByteUtilities.uint32ToBytes_LE(this.latitude);
        buf[ 0] = tmp[0];
        buf[ 1] = tmp[1];
        buf[ 2] = tmp[2];
        buf[ 3] = tmp[3];

        tmp = ByteUtilities.uint32ToBytes_LE(this.longitude);
        buf[ 4] = tmp[0];
        buf[ 5] = tmp[1];
        buf[ 6] = tmp[2];
        buf[ 7] = tmp[3];

        tmp = ByteUtilities.uint32ToBytes_LE(this.elevation);
        buf[ 8] = tmp[0];
        buf[ 9] = tmp[1];
        buf[10] = tmp[2];
        buf[11] = tmp[3];

        tmp = ByteUtilities.uint16ToBytes_LE(this.accuracy);
        buf[12] = tmp[0];
        buf[13] = tmp[1];

        tmp = ByteUtilities.uint16ToBytes_LE(this.deltaElevation);
        buf[14] = tmp[0];
        buf[15] = tmp[1];

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
        else if(buf.length >= Location.MIN_LENGTH)
        {
            this.latitude = ByteUtilities.bytesToUint32_LE(buf,offset);
            this.longitude = ByteUtilities.bytesToUint32_LE(buf,offset + 4);
            this.elevation = ByteUtilities.bytesToUint32_LE(buf,offset + 8);
            this.accuracy = ByteUtilities.bytesToUint16_LE(buf,offset + 12);
            this.deltaElevation = ByteUtilities.bytesToUint16_LE(buf,offset + 14);

            success = true;
        }

        return success;
    }
}

//Constants
Location.MIN_LENGTH = 16;
