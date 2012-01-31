#CC=arm-elf-gcc
CC=$1

#Defaults to 'gcc' if no compiler is given
if [ "$CC" = "" ]; then
  CC=gcc
fi

$CC -I . -lpthread -o halo_udp_unit_test mpers/DynamicVitalsMsg.c \
mpers/HaloMsgHelperFunctions.c \
UdpLib/halo_udp_tx_mgmt.c \
UdpLib/tx_mgmt_buffer.c \
UdpLib/halo_udp_stats.c \
UdpLib/halo_udp_comm.c \
UdpLib/tx_buffer_static.c \
UdpLib/crc16.c \
UdpLib/unit_test/halo_udp_tests.c \
UdpLib/unit_test/test_impl.c \
UdpLib/unit_test/test_impl2.c \
UdpLib/unit_test/test_mode.c \
UdpLib/unit_test/udp_lib_test_impl.c \
UdpLib/unit_test/main.c \
UdpLib/unit_test/tests.c \
UdpLib/unit_test/test_msg_queue.c

