log = require('util').log
dgram = require('dgram')
var ByteUtilities = require('./ByteUtils').ByteUtils;
var MyHaloUdpFrame = require('./MyHaloUdpFrame').MyHaloUdpFrame;
var MyHaloUdpEnums = require('./MyHaloUdpFrame').MyHaloUdpEnums;
var SessionData = require('./SessionData').SessionData;
var TransmissionEntry = require('./TransmissionEntry').TransmissionEntry;

exports.MyHaloUdp = MyHaloUdp;

function MyHaloUdp() {}

//Session Data structure
MyHaloUdp.sessions = new Array(); //Holds session data
MyHaloUdp.transmissionItems = []; //Holds items pending to be sent

MyHaloUdp.preSendProcess = function(address,port,buf)
{
     //Handles sending pkts
    var session = MyHaloUdp.getSession(address,port);

    if(session != null)
    {
        //Modify the frame with a new session flag if a new session pending
        if(session.newTxSessionPending())
        {
            var frame = new MyHaloUdpFrame();
            frame.decode(buf,0);
            var header = frame.getHeader();

            //TODO: Figure out if we want data available managed here instead
            //of inside the MyHaloUdpFrame structure
            header.setStatusBits(MyHaloUdpEnums.MyHaloUdpStatusBits.NEW_SESSION);
            buf = frame.encode(header);
        }
    }

    return buf;
}

MyHaloUdp.send = function(sock,address,port,buf)
{
  var binaryRepString = ByteUtilities.toBinaryString(buf);

  sock.send(buf, 0, buf.length, port, address);
  log('sent message to '+ address +' port: '+ port);
  log('data len: '+ buf.length + " data: "+ binaryRepString);
}

MyHaloUdp.sendData = function(sock,address,port,data){
  var frame = new MyHaloUdpFrame();
  var session = MyHaloUdp.getSession(address,port);
  var buf = null;

  if(session != null)
  {
      frame.setPayload(data);

      var header = frame.getHeader();

      //Generate header needed to send the acknowledgment
      header.setVersion(MyHaloUdpFrame.CURRENT_VERSION);
      header.setSeqNum(session.getNextTxSeqNum());

      buf = frame.encode(header);

      MyHaloUdp.enqueueTransmission(new TransmissionEntry(address,port,header.getSeqNum(),buf,sock));
  }
}

MyHaloUdp.sendAck = function(sock,address,port,seqNum){
  var frame  = new MyHaloUdpFrame();
  var header = frame.getHeader();
  var session = MyHaloUdp.getSession(address, port);

  if(session != null)
  {
      //Set Session acknowledgment if applicable
      if(session.newRxSessionPending())
      {
          header.setStatusBits(MyHaloUdpEnums.MyHaloUdpStatusBits.SESSION_RESTARTED);
          session.newRxSessionComplete();
      }

      //Generate header needed to send the acknowledgment
      header.setVersion(MyHaloUdpFrame.CURRENT_VERSION);
      header.setSeqNum(session.getNextTxSeqNum());
      header.setStatusBits(MyHaloUdpEnums.MyHaloUdpStatusBits.MSG_RECEIVED_ACK);
      header.setAckSeqNum(seqNum);
      var buf = frame.encode(header);

      log('sending ack...'+'('+buf.length+' bytes) '+ByteUtilities.toBinaryString(buf));
      sock.send(buf, 0, buf.length, port, address);
      log('sent ack to ' + address + ':' + port);
  }
}

MyHaloUdp.receiveData = function(sock,address,port,bytes,newMsgCallback){
  var frame = new MyHaloUdpFrame();

  if(frame.decode(bytes,0))
  {
    var payload = frame.getPayload();
    var header  = frame.getHeader();
    var status  = header.getStatus();
    var seqNum  = header.getSeqNum();
    var ackSeqNum  = header.getAckSeqNum();
    var session = MyHaloUdp.getSession(address, port);

    //Add a session if it is not found
    if(session == null)
    {
        session = new SessionData(address,port);
        log('adding new session - address: '+session.address + ' port: ' + session.port);
        MyHaloUdp.addSession(session);
    }
    else
    {
        session.resetTicksInactive();
    }

    if(status & MyHaloUdpEnums.MyHaloUdpStatusBits.MSG_RECEIVED_ACK)
    {
      //Process Acknowledgements here
      log('ackSeqNum: '+ackSeqNum);
      MyHaloUdp.acknowledgementCheck(address,port,ackSeqNum);

      // Handles confirmation of new tx session
      if (status & MyHaloUdpEnums.MyHaloUdpStatusBits.SESSION_RESTARTED) {
        // Does new session processing
        session.newTxSessionComplete();
      }
    }
    else if(status & MyHaloUdpEnums.MyHaloUdpStatusBits.DATA_AVAILABLE)
    {
      //Process Data Here
      var newPacket = false;

      if (status & MyHaloUdpEnums.MyHaloUdpStatusBits.NEW_SESSION) {
          // Handles when the remote end notifies of starting a new
          // session
          session.requestedNewRxSession();
      }
      newPacket = session.addRxSequenceNumber(seqNum);

      //Send acknowledgement
      MyHaloUdp.sendAck(sock,address,port,seqNum);

      if(newPacket)
      {
          //Notify of a new message
          newMsgCallback(sock,address,port,payload);
      }
      else
      {
          log('Duplicate packet detected!');
      }
    }

  }
  else
  {
    log('Frame failed decoding!');
  }
}

MyHaloUdp.acknowledgementCheck = function(address,port,seqNum){
   MyHaloUdp.transmissionComplete(address,port,seqNum);
}

MyHaloUdp.getSession = function(address,port){
    var selectedSession = null;

    for (var i = 0; i < MyHaloUdp.sessions.length; i++)
    {
        if((MyHaloUdp.sessions[i].address == address) && (MyHaloUdp.sessions[i].port == port))
        {
            selectedSession = MyHaloUdp.sessions[i];
            break;
        }
    }

    return selectedSession;
}

MyHaloUdp.addSession = function(session){
    var success = false;

    MyHaloUdp.sessions.push(session);
    success = true;

    return success;
}

MyHaloUdp.updateSessionTicks = function(){
    //Optimized (Needs to be tested) - Should work as long as the array
    //is only altered starting with the index selected for removal
    var currentIndex = 0;

    while(currentIndex < MyHaloUdp.sessions.length)
    {
        //Increment each session
        MyHaloUdp.sessions[currentIndex].incrementTicksInactive();

        //Remove old sessions
        if(MyHaloUdp.sessions[currentIndex].getTicksInactive() >= MyHaloUdp.MAX_TICKS_INACTIVE)
        {
            var removedArry=MyHaloUdp.sessions.splice(currentIndex, 1); //Remove this item
            //Do NOT increment the index as this is where the array altering starts
            log('removing session - address: '+removedArry[0].address + ' port: ' + removedArry[0].port);
        }
        else
        {
            currentIndex++;
        }
    }
}

MyHaloUdp.removeAnOldEntry = function(){
    var oldEntryFound = false;

    for (var i = 0; i < MyHaloUdp.sessions.length; i++)
    {
        if(MyHaloUdp.sessions[i].getTicksInactive() >= MyHaloUdp.MAX_TICKS_INACTIVE)
        {
            MyHaloUdp.sessions.splice(i, 1); //Remove this item
            oldEntryFound = true;
            break; //this loop has to be stopped because the indexing has changed due to the removal
        }
    }
    
    return oldEntryFound;
}

MyHaloUdp.HzTick = function() {
    MyHaloUdp.updateSessionTicks();
    MyHaloUdp.processTransmissionQueue();
}

MyHaloUdp.enqueueTransmission = function(transmissionEntry) {
    MyHaloUdp.transmissionItems.push(transmissionEntry);
}

MyHaloUdp.transmissionComplete = function(address,port,seqNum) {
    for(var i = 0; i < MyHaloUdp.transmissionItems.length; i++)
    {
        var transmissionEntry = MyHaloUdp.transmissionItems[i];

        if(transmissionEntry.pktMatch(address,port,seqNum))
        {
            MyHaloUdp.transmissionItems.splice(i, 1); //Remove this item
            log('transmission successful');
            log('removing pkt - address: '+transmissionEntry.getDstAddress()
            + ' port: ' + transmissionEntry.getDstPort());
        }
    }
}

MyHaloUdp.processTransmissionQueue = function() {
    var currentIndex = 0;
    var itemsTransmitted = 0;

    while((currentIndex < MyHaloUdp.transmissionItems.length)&&(itemsTransmitted < MyHaloUdp.MAX_BURST))
    {
        var transmissionEntry = null;

        transmissionEntry = MyHaloUdp.transmissionItems[currentIndex];
        transmissionEntry.incrementTimesTransmitted();

        log('sending pkt - address: '+transmissionEntry.getDstAddress() +
        ' port: ' + transmissionEntry.getDstPort());

        //Remove items already retransmitted too much
        if(transmissionEntry.getTimesTransmitted() >= MyHaloUdp.MAX_XMIT)
        {
            MyHaloUdp.transmissionItems.splice(currentIndex, 1); //Remove this item
            log('transmission failed. Dropping');
            //Do NOT increment the index as this is where the array altering starts
            log('removing pkt - address: '+transmissionEntry.getDstAddress()
            + ' port: ' + transmissionEntry.getDstPort());
        }
        else
        {
            //Do any last minute processing
            transmissionEntry.setData(MyHaloUdp.preSendProcess(transmissionEntry.getDstAddress(), 
            transmissionEntry.getDstPort(), transmissionEntry.getData()));

            //Actual transmit the data
            MyHaloUdp.send(transmissionEntry.getSocket(),
            transmissionEntry.getDstAddress(),
            transmissionEntry.getDstPort(),transmissionEntry.getData());
            currentIndex++;
            itemsTransmitted++;
        }
    }
}

//Constants
MyHaloUdp.MAX_TICKS_INACTIVE = 180; //3min timeout
MyHaloUdp.MAX_XMIT = 5; //Maximum amount of entries to allow in the queue
MyHaloUdp.MAX_BURST = 3; //Maximum amount of entries to allow in the queue
