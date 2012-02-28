log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;

exports.MyHaloMsgBase = MyHaloMsgBase;
exports.MyHaloMsgEnums = MyHaloMsgEnums;

function MyHaloMsgEnums() {}
MyHaloMsgEnums.MyHaloMsgType = {
    UNKNOWN : -1,
    ALL_DATA : 0,
    ALL_DATA_DYNAMIC : 1,
    VITALS : 2,
    LOCATION : 3,
    BATTERY : 4,
    DEBUG : 5,
    CRITICAL_ALERT : 6,
    RESOLVED_EVENT : 7
}

function MyHaloMsgBase()
{
    //Members
    this.commandType = 0; //2 bytes
    this.formatVersion = 0; // 1 byte
    this.deviceId = 0; //4 bytes
    this.time = 0; //4 bytes

    //Getters/Setters
    this.getCommandType = function() {
        return this.commandType;
    }

    this.setCommandType = function(commandType) {
        this.commandType = commandType;
    }

    this.getFormatVersion = function() {
        return this.formatVersion;
    }

    this.setFormatVersion = function(formatVersion) {
        this.formatVersion = formatVersion;
    }

    this.getDeviceId = function() {
        return this.deviceId;
    }

    this.setDeviceId = function(deviceId) {
        this.deviceId = deviceId;
    }

    this.getTime = function() {
        return this.time;
    }

    this.setTime = function(time) {
        this.time = time;
    }

    //Byte Processing
    this.getLength = function ()
    {
        return MyHaloMsgBase.MIN_LENGTH;
    }

    this.encode = function()
    {
        var buf = new Buffer(this.getLength());
        var tmp = null;

        buf[ 0] = this.formatVersion;
        buf[ 1] = 0;
        tmp = ByteUtilities.uint16ToBytes_LE(this.commandType);
        buf[ 2] = tmp[0];
        buf[ 3] = tmp[1];
        tmp = ByteUtilities.uint32ToBytes_LE(this.deviceId);
        buf[ 4] = tmp[0];
        buf[ 5] = tmp[1];
        buf[ 6] = tmp[2];
        buf[ 7] = tmp[3];
        tmp = ByteUtilities.uint32ToBytes_LE(this.time);
        buf[ 8] = tmp[0];
        buf[ 9] = tmp[1];
        buf[10] = tmp[2];
        buf[11] = tmp[3];

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
        else if(buf.length >= MyHaloMsgBase.MIN_LENGTH)
        {
            this.formatVersion = buf[offset];
            this.commandType = ByteUtilities.bytesToUint16_LE(buf,offset + 2);
            this.deviceId = ByteUtilities.bytesToUint32_LE(buf,offset + 4);
            this.time = ByteUtilities.bytesToUint32_LE(buf,offset + 8);

            success = true;
        }

        return success;
    }

    this.update = function()
    {
        var d = new Date();
        this.time = (d.getTime()/1000);
    }
}

//Constants
MyHaloMsgBase.MIN_LENGTH = 12;
