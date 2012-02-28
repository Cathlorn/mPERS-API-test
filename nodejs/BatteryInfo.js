log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;

exports.BatteryInfo = BatteryInfo;
function BatteryInfo()
{
    //Members
    this.minRemaining = 0; //2 bytes
    this.battPercentage = 0; //1 byte
    this.deviceStatus = 0; //3 bits
    this.chargeComplete = 0; //1 bit
    this.charging  = 0; //1 bit
    this.plugged = 0; //1 bit
    this.battVoltage = 0; //4 bytes

    //Getters/Setters
    this.getMinRemaining = function() {
        return this.minRemaining;
    }

    this.setMinRemaining = function(minRemaining) {
        this.minRemaining = minRemaining;
    }

    this.getBattPercentage = function() {
        return this.battPercentage;
    }

    this.setBattPercentage = function(battPercentage) {
        this.battPercentage = battPercentage;
    }

    this.getDeviceStatus = function() {
        return this.deviceStatus;
    }

    this.setDeviceStatus = function(deviceStatus) {
        this.deviceStatus = deviceStatus;
    }

    this.getChargeComplete = function() {
        return this.chargeComplete;
    }

    this.setChargeComplete = function(chargeComplete) {
        this.chargeComplete = chargeComplete;
    }

    this.getCharging = function() {
        return this.charging;
    }

    this.setCharging = function(charging) {
        this.charging = charging;
    }
    
    this.getPlugged = function() {
        return this.plugged;
    }

    this.setPlugged = function(plugged) {
        this.plugged = plugged;
    }
    
    this.getBattVoltage = function() {
        return this.battVoltage;
    }

    this.setBattVoltage = function(battVoltage) {
        this.battVoltage = battVoltage;
    }

    //Byte Processing
    this.getLength = function ()
    {
        return BatteryInfo.MIN_LENGTH;
    }

    this.encode = function()
    {
        var buf = new Buffer(this.getLength());
        var tmp = null;

        tmp = ByteUtilities.uint16ToBytes_LE(this.minRemaining);
        buf[ 0] = tmp[0];
        buf[ 1] = tmp[1];

        buf[ 2] = this.battPercentage;

        buf[ 3]  = ((this.plugged ? 1 : 0) << 7) & 0xff;
        buf[ 3] |= ((this.charging ? 1 : 0) << 6)& 0xff;
        buf[ 3] |= ((this.chargeComplete ? 1 : 0) << 5)& 0xff;
        buf[ 3] |= ((this.deviceStatus & 0x7) << 2)& 0xff;

        tmp = ByteUtilities.uint32ToBytes_LE(this.battVoltage);
        buf[ 4] = tmp[0];
        buf[ 5] = tmp[1];
        buf[ 6] = tmp[2];
        buf[ 7] = tmp[3];

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
        else if(buf.length >= BatteryInfo.MIN_LENGTH)
        {
            this.minRemaining = ByteUtilities.bytesToUint16_LE(buf,offset);
            this.battPercentage = buf[offset + 2];
            var status  = buf[offset + 3];
            this.plugged = ((status >> 7) & 0x1);
            this.charging = ((status >> 6) & 0x1);
            this.chargeComplete = ((status >> 5) & 0x1);
            this.deviceStatus = ((status >> 2) & 0x7);
            this.battVoltage = ByteUtilities.bytesToUint32_LE(buf,offset + 4);

            success = true;
        }

        return success;
    }
}

//Constants
BatteryInfo.MIN_LENGTH = 8;
