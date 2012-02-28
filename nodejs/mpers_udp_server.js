log = require('util').log
dgram = require('dgram')
var ByteUtilities = require('./ByteUtils').ByteUtils;
var MyHaloMsgBase = require('./MyHaloMsgBase').MyHaloMsgBase;
var MyHaloMsgEnums = require('./MyHaloMsgBase').MyHaloMsgEnums;
var CriticalAlertMsg = require('./CriticalAlertMsg').CriticalAlertMsg;
var CriticalAlertEnums = require('./CriticalAlertMsg').CriticalAlertEnums;
var MyHaloUdp = require('./MyHaloUdp').MyHaloUdp;
var mysql = require('./felixge-node-mysql/lib/mysql');

//MySQL DB interface setup
var mySqlClient = mysql.createClient({
  host: 'dev1.cgbkgqhir9k9.us-east-1.rds.amazonaws.com',
  user: 'halords',
  password: '8Zpku25w',
  database: 'halodev',
});

socket = dgram.createSocket("udp4");
socket.addListener('message', function (msg, rinfo) {
  var binaryRepString = ByteUtilities.toBinaryString(msg);

  log('got message from '+ rinfo.address +' port: '+ rinfo.port);
  log('data len: '+ rinfo.size + " data: "+ binaryRepString);

  //will callback newMsg if for handling of new data msgs
  MyHaloUdp.receiveData(socket,rinfo.address,rinfo.port, msg, newMsg);
});

socket.bind(80);

function sendMsg(sock,address,port){
  var critMsg = new CriticalAlertMsg();

  critMsg.setCriticalAlertType(CriticalAlertEnums.CriticalAlertType.FALL);
  MyHaloUdp.sendData(sock, address, port, critMsg.encode());
}

function getDbTimestampString(date) {
  return date.getUTCFullYear() + '-' + (date.getUTCMonth() + 1) + '-' +
  date.getUTCDate() + ' ' + date.getUTCHours() + ':' + date.getUTCMinutes() +
  ':' + date.getUTCSeconds() + '+00';
}

//Callback for new received myHalo UDP msgs
function newMsg(sock,address,port, msg){
  var myHaloMsgBase = new MyHaloMsgBase();

  if(myHaloMsgBase.decode(msg, 0))
  {
    var commandType = myHaloMsgBase.getCommandType();
    var payloadOffset = myHaloMsgBase.getLength();

    if( commandType == MyHaloMsgEnums.MyHaloMsgType.CRITICAL_ALERT)
    {
      var critMsg = new CriticalAlertMsg();

      if(critMsg.import(myHaloMsgBase, msg, payloadOffset))
      {
        //log('Crit Msg Rcvd!');
        //sendMsg(sock, address, port);
        var alertTimestamp = new Date(critMsg.getMyHaloMsgBase().getTime()*1000);
        var nadTimestamp = new Date(critMsg.getNadTimestamp()*1000);

        //log('orig alertTime: ' + alertTime);
        log('alerttime[utc]: ' + alertTimestamp.toUTCString());
        var queryString =           'insert into critical_alerts (deviceId,alertTime,' +
          'nadTime,criticalAlertType,retryNumber,battPercentage,location)' +
          ' values (' +
          critMsg.getMyHaloMsgBase().getDeviceId() + ',' +
          '\'' + getDbTimestampString(alertTimestamp) + '\',\'' + getDbTimestampString(nadTimestamp) + '\',' +
          '\'' + CriticalAlertEnums.CriticalAlertType.getName(critMsg.getCriticalAlertType())+'\','+
          critMsg.getRetryNumber() + ',' +
          critMsg.getBatteryInfo().getBattPercentage() + ',' +
          '\'' + critMsg.getLocation().getLatitude() + ',' + critMsg.getLocation().getLongitude() + '\')';

        log('queryString: ' + queryString);

        //Write to DB the Critical Alert
        mySqlClient.query(
          queryString,
          function selectCb(err, results, fields) {
            if (err) {
              throw err;
            }

            //console.log(results);
            //mySqlClient.end();
          }
        );
      }
    }
  }
}

//Calls tick Management from a Timer
setInterval(function() {
MyHaloUdp.HzTick(socket);
}, 1000);
