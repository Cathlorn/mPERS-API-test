TARGET_SERVER=ec2-204-236-192-230.compute-1.amazonaws.com
TARGET_PORT=80

#Checking out of sequence numbering works
./halo_udp_dvt -d outseq -h$TARGET_SERVER -p$TARGET_PORT -t1 -a3

#Test Server can send Dynamic Vitals
./halo_udp_dvt -h$TARGET_SERVER -p$TARGET_PORT -t1 -a3

#Test Server can send Panic Msgs
./halo_udp_dvt -h$TARGET_SERVER -p$TARGET_PORT -t2 -a3

#Test Server can send Fall Msgs
./halo_udp_dvt -h$TARGET_SERVER -p$TARGET_PORT -t3 -a3

#Test Server can send Operator Ack Msgs
./halo_udp_dvt -h$TARGET_SERVER -p$TARGET_PORT -t4 -a3

##Test Server will work after receiving an Invalid Msg
#./halo_udp_dvt -h$TARGET_SERVER -p$TARGET_PORT -t5 -a3
#./halo_udp_dvt -h$TARGET_SERVER -p$TARGET_PORT -t1 -a3


#Test Server will fail with Bad CRC
./halo_udp_dvt -dbadcrc -h$TARGET_SERVER -p$TARGET_PORT -t2 -a1 2>&1 1 >/dev/null
PROG_RESULT=$?
#echo "PROG_RESULT: $PROG_RESULT"
if (( $PROG_RESULT )); then
  echo "BadCrcTest: PASSED"
else
  echo "BadCrcTest: FAILED"
fi

#Test Server will fail with No Acknowledgement
#./halo_udp_dvt -dnoack -h$TARGET_SERVER -p$TARGET_PORT -t2 -a1 2>&1 1 >/dev/null
#PROG_RESULT=$?
#echo "PROG_RESULT: $PROG_RESULT"
#if (( $PROG_RESULT )); then
#  echo "NoAckTest: PASSED"
#else
#  echo "NoAckTest: FAILED"
#fi

#Test Server will succeed in deliverying even with 50% success
./halo_udp_dvt -dcommfail=50 -h$TARGET_SERVER -p$TARGET_PORT -t2 -a5


#Tests supported by DVT:
#   1.     Send Dynamic Vitals Msg
#   2.     Send Panic Msg
#   3.     Send Fall Msg
#   4.     Send Operator Acknowledgement Msg
#   5.     Send Invalid Msg
#   6.     Listen for Msgs
