#CC=arm-elf-gcc
CC=$1

#Defaults to 'gcc' if no compiler is given
if [ "$CC" = "" ]; then
  CC=gcc
fi

$CC -I . -lpthread -o halo_udp_dvt mpers/DynamicVitalsMsg.c \
mpers/HaloMsgHelperFunctions.c \
General/genericStaticArrayBuffer.c \
UdpLib/halo_udp_tx_mgmt.c \
UdpLib/tx_mgmt_buffer.c \
UdpLib/halo_udp_stats.c \
UdpLib/halo_udp_comm.c \
UdpLib/tx_buffer_static.c \
UdpLib/crc16.c \
dvt_app_linux/client_mode.c \
dvt_app_linux/client_test_impl.c \
dvt_app_linux/client_tests.c \
dvt_app_linux/main.c \
dvt_app_linux/server_mode.c \
dvt_app_linux/server_tests.c \
dvt_app_linux/str_utils.c \
Test/tests.c \
UdpLibImpl/udp_lib_posix_socket_impl.c

