exports.ByteUtils = ByteUtils;

log = require('util').log

function ByteUtils() {}

ByteUtils.uint16ToBytes_LE = function(shortVal) {
    var byteArray = new Array();

    byteArray.push((shortVal) & 0xff);
    byteArray.push((shortVal >> 8) &0xff);
    
    return byteArray;
}

ByteUtils.bytesToUint16_LE = function(buf,offset) {
    var shortVal;

    shortVal = buf[offset];
    shortVal |= ((buf[offset + 1] << 8) & 0xffff);

    return shortVal;
}

ByteUtils.uint32ToBytes_LE = function(intVal) {
    var byteArray = new Array();

    byteArray.push((intVal) & 0xff);
    byteArray.push((intVal >> 8) &0xff);
    byteArray.push((intVal >> 16) &0xff);
    byteArray.push((intVal >> 24) &0xff);

    return byteArray;
}

ByteUtils.bytesToUint32_LE = function(buf,offset) {
    var intVal;

    intVal  =   buf[offset];
    intVal |= ( buf[offset + 1] << 8);
    intVal |= ( buf[offset + 2] << 16);
    intVal |= ((buf[offset + 3] << 24) & 0xffffffff);

    return intVal;
}

ByteUtils.toBinaryString = function(buf) {
  var binaryRepString = '';

  for(var i = 0; i < buf.length; i++)
  {
      if(buf[i] < 16) {
         binaryRepString += '0';
      }
      binaryRepString += buf[i].toString(16) + ' ';
  }

  return binaryRepString;
}
