log = require('util').log
var Buffer = require('buffer').Buffer;
var ByteUtilities = require('./ByteUtils').ByteUtils;
var MyHaloMsgBase = require('./MyHaloMsgBase').MyHaloMsgBase;
var MyHaloMsgType = require('./MyHaloMsgBase').MyHaloMsgEnums.MyHaloMsgType;
var Location = require('./Location').Location;
var SignalStrength = require('./SignalStrength').SignalStrength;
var BatteryInfo = require('./BatteryInfo').BatteryInfo;

exports.CriticalAlertMsg = CriticalAlertMsg;
exports.CriticalAlertEnums = CriticalAlertEnums;

function CriticalAlertEnums() {}
CriticalAlertEnums.CriticalAlertType = {
    UNKNOWN : -1,
    FALL : 0,
    PANIC : 1,
    OPERATOR_ACK : 2
}

CriticalAlertEnums.CriticalAlertType.getName = function(type)
{
    var typeName = 'UNKNOWN';

    log('crittype: ' + type);
    if(type == CriticalAlertEnums.CriticalAlertType.FALL)
    {
        typeName = 'FALL';
    }
    else if(type == CriticalAlertEnums.CriticalAlertType.PANIC)
    {
        typeName = 'PANIC';
    }
    else if(type == CriticalAlertEnums.CriticalAlertType.OPERATOR_ACK)
    {
        typeName = 'OPERATOR_ACK';
    }

    return typeName;
}

function CriticalAlertMsg()
{
    //Members
    this.myHaloMsgBase = new MyHaloMsgBase();
    this.location = new Location();
    this.signalStrength = new SignalStrength();
    this.batteryInfo = new BatteryInfo();
    this.nadTimestamp = 0;  //(replaces gw timestamp;nad=Network Access Device)
    this.criticalAlertType = CriticalAlertEnums.CriticalAlertType.UNKNOWN;
    this.retryNumber = 0;   //(Up to 6 tries per upload attempt)

    //Getters/Setters
    this.getMyHaloMsgBase = function() {
        return this.myHaloMsgBase;
    }

    this.setMyHaloMsgBase = function(myHaloMsgBase) {
        this.myHaloMsgBase = myHaloMsgBase;
    }

    this.getLocation = function() {
        return this.location;
    }

    this.setLocation = function(location) {
        this.location = location;
    }
    
    this.getSignalStrength = function() {
        return this.signalStrength;
    }

    this.setSignalStrength = function(signalStrength) {
        this.signalStrength = signalStrength;
    }

    this.getBatteryInfo = function() {
        return this.batteryInfo;
    }

    this.setBatteryInfo = function(batteryInfo) {
        this.batteryInfo = batteryInfo;
    }

    this.getNadTimestamp = function() {
        return this.nadTimestamp;
    }

    this.setNadTimestamp = function(nadTimestamp) {
        this.nadTimestamp = nadTimestamp;
    }

    this.getCriticalAlertType = function() {
        return this.criticalAlertType;
    }

    this.setCriticalAlertType = function(criticalAlertType) {
        this.criticalAlertType = criticalAlertType;
    }

    this.getRetryNumber = function() {
        return this.retryNumber;
    }

    this.setRetryNumber = function(retryNumber) {
        this.retryNumber = retryNumber;
    }

    //Byte Processing
    this.getLength = function ()
    {
        var payloadLength = this.getPayloadLength();

        return this.myHaloMsgBase.getLength() + payloadLength;
    }

    this.getPayloadLength = function ()
    {
        return this.location.getLength() + this.signalStrength.getLength()
               + this.batteryInfo.getLength()
               + CriticalAlertMsg.XTRA_PARAM_LENGTH;
    }

    this.encode = function()
    {
        var buf = new Buffer(this.getLength());
        var offset = 0;
        var tmp = null;

        //MyHaloMsgBase processing

        //Updates information common to all msgs.
        this.myHaloMsgBase.update();

        //Makes sure msg type is correct
        if(this.myHaloMsgBase.getCommandType() != MyHaloMsgType.CRITICAL_ALERT)
        {
            this.myHaloMsgBase.setCommandType(MyHaloMsgType.CRITICAL_ALERT);
        }

        this.myHaloMsgBase.setFormatVersion(1);

        var baseMsg = this.myHaloMsgBase.encode();
        for(var i = 0; i < baseMsg.length; i++)
        {
            buf[offset++] = baseMsg[i];
        }

        //Location
        tmp = this.location.encode();
        for(var i = 0; i < tmp.length; i++)
        {
            buf[offset++] = tmp[i];
        }

        //Signal Strength
        tmp = this.signalStrength.encode();
        for(var i = 0; i < tmp.length; i++)
        {
            buf[offset++] = tmp[i];
        }

        //Battery Info
        tmp = this.batteryInfo.encode();
        for(var i = 0; i < tmp.length; i++)
        {
            buf[offset++] = tmp[i];
        }

        tmp = ByteUtilities.uint32ToBytes_LE(this.nadTimestamp);
        buf[offset++] = tmp[0];
        buf[offset++] = tmp[1];
        buf[offset++] = tmp[2];
        buf[offset++] = tmp[3];

        buf[offset++] = this.criticalAlertType;
        buf[offset++] = this.retryNumber;

        //Reserved Bytes
        buf[offset++] = 0;
        buf[offset++] = 0;

        return buf;
    }

    this.decode = function(buf,offset)
    {
        var success = false;

        if(buf == null)
        {
            //Do Nothing
        }
        else if(buf.length >= CriticalAlertMsg.MIN_LENGTH)
        {
            var msgOffset;

            msgOffset = offset
            if(this.myHaloMsgBase.decode(buf, msgOffset))
            {
                  msgOffset += this.myHaloMsgBase.getLength();
                  success |= this.import(this.myHaloMsgBase, buf, msgOffset);
            }
        }

        return success;
    }
    
    //Import takes in the unique params and gets provided the msg base
    //This prevents redundant decoding of myHalo Udp msgs, yet keeps all info
    //populated
    this.import = function(msgBase,buf,offset)
    {
        var success = false;

        if(buf == null)
        {
            //Do Nothing
        }
        else if(buf.length >= CriticalAlertMsg.MIN_LENGTH)
        {
            var msgOffset;

            msgOffset = offset
            if(msgBase != null)
            {
                  this.myHaloMsgBase = msgBase;
                  if(this.location.decode(buf, msgOffset))
                  {
                      msgOffset += this.location.getLength();
                      if(this.signalStrength.decode(buf, msgOffset))
                      {
                          msgOffset += this.signalStrength.getLength();
                          if(this.batteryInfo.decode(buf, msgOffset))
                          {
                              msgOffset += this.batteryInfo.getLength();
                              this.nadTimestamp = ByteUtilities
                                                  .bytesToUint32_LE(buf,
                                                                    msgOffset);
                              this.criticalAlertType = buf[msgOffset + 4]
                              this.retryNumber = buf[msgOffset + 5];

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

//Additional sizing params
CriticalAlertMsg.NAD_TIMESTAMP_SIZE = 4;
CriticalAlertMsg.CRITICAL_ALERT_TYPE_SIZE = 1;
CriticalAlertMsg.RETRY_NUMBER_SIZE = 1;
CriticalAlertMsg.PAD_SIZE = 2;

CriticalAlertMsg.XTRA_PARAM_LENGTH = CriticalAlertMsg.NAD_TIMESTAMP_SIZE
                                   + CriticalAlertMsg.CRITICAL_ALERT_TYPE_SIZE
                                   + CriticalAlertMsg.RETRY_NUMBER_SIZE
                                   + CriticalAlertMsg.PAD_SIZE;

CriticalAlertMsg.MIN_LENGTH = MyHaloMsgBase.MIN_LENGTH 
                            + Location.MIN_LENGTH + SignalStrength.MIN_LENGTH
                            + BatteryInfo.MIN_LENGTH
                            + CriticalAlertMsg.XTRA_PARAM_LENGTH;
