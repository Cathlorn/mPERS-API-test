#CC=arm-elf-gcc
CC=$1

#Defaults to 'gcc' if no compiler is given
if [ "$CC" = "" ]; then
CC=gcc
fi

$CC -lpthread -I . -o halo_udp_dbg_lnx debug_app_linux/client_mode.c \
debug_app_linux/client_test_impl.c \
debug_app_linux/client_tests.c \
debug_app_linux/main.c \
debug_app_linux/server_mode.c \
debug_app_linux/server_tests.c \
Test/tests.c \
UdpLibImpl/udp_lib_posix_socket_impl.c \
mpers/DynamicVitalsMsg.c \
mpers/HaloMsgHelperFunctions.c \
UdpLib/halo_udp_tx_mgmt.c \
UdpLib/halo_udp_stats.c \
UdpLib/tx_mgmt_buffer.c \
UdpLib/halo_udp_comm.c \
UdpLib/tx_buffer_static.c \
UdpLib/crc16.c
