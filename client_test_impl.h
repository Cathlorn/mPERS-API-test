#ifndef CLIENT_TEST_IMPL_H
#define CLIENT_TEST_IMPL_H

//Function Prototypes
void sendDynamicVitalsPkt(void);
void sendPanicMsgPkt(void);
void sendFallMsgPkt(void);
void sendBurst(void);

void start_new_halo_udp_session(void);
void show_halo_udp_sessions(void);
void halo_udp_loopback_enable(void);
void halo_udp_loopback_disable(void);
void halo_udp_bad_crc_enable(void);
void halo_udp_bad_crc_disable(void);
void halo_udp_never_ack_enable(void);
void halo_udp_never_ack_disable(void);
void halo_udp_never_tx_drop_enable(void);
void halo_udp_never_tx_drop_disable(void);
void halo_udp_print_dbg_comm_status(void);
void halo_udp_duplicate_tx_enable(void);
void halo_udp_duplicate_tx_disable(void);
void halo_udp_out_of_seq_tx_enable(void);
void halo_udp_out_of_seq_tx_disable(void);

#endif //CLIENT_TEST_IMPL_H
