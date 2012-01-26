#CC=arm-elf-gcc
CC=$1

#Defaults to 'gcc' if no compiler is given
if [ "$CC" = "" ]; then
CC=gcc
fi

$CC -lpthread -o halo_udp_test client_mode.c \
client_test_impl.c \
client_tests.c \
DynamicVitalsMsg.c \
HaloMsgHelperFunctions.c \
main.c \
server_mode.c \
server_tests.c \
tests.c \
UdpLib/halo_udp_tx_mgmt.c \
UdpLib/halo_udp_stats.c \
UdpLib/tx_mgmt_buffer.c \
UdpLib/udp_lib_posix_socket_impl.c \
UdpLib/halo_udp_comm.c \
UdpLib/tx_buffer_static.c \
UdpLib/crc16.c
