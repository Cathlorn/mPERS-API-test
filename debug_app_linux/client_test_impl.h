#ifndef CLIENT_TEST_IMPL_H
#define CLIENT_TEST_IMPL_H

//Function Prototypes
int sendDynamicVitalsPkt(void *args);
int sendPanicMsgPkt(void *args);
int sendFallMsgPkt(void *args);
int sendBurst(void *args);

int start_new_halo_udp_session(void *args);
int show_halo_udp_sessions(void *args);
int halo_udp_loopback_enable(void *args);
int halo_udp_loopback_disable(void *args);
int halo_udp_bad_crc_enable(void *args);
int halo_udp_bad_crc_disable(void *args);
int halo_udp_never_ack_enable(void *args);
int halo_udp_never_ack_disable(void *args);
int halo_udp_never_tx_drop_enable(void *args);
int halo_udp_never_tx_drop_disable(void *args);
int halo_udp_print_dbg_comm_status(void *args);
int halo_udp_duplicate_tx_enable(void *args);
int halo_udp_duplicate_tx_disable(void *args);
int halo_udp_out_of_seq_tx_enable(void *args);
int halo_udp_out_of_seq_tx_disable(void *args);
int halo_udp_spotty_comm_control(void *args);
int halo_udp_print_stats(void *args);
int halo_udp_reset_stats(void *args);

#endif //CLIENT_TEST_IMPL_H
