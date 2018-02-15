#include "sndtbl.c"

static uint16_t app_have_packet;
static uint16_t app_seqn;
static uint16_t app_log_seqn;
static uint16_t app_n_packets;

#define PACKETS_PER_EPOCH 1

#if CRYSTAL_LOGGING
struct pkt_record {
  uint16_t seqn;
  uint16_t acked;
  
} app_packets[PACKETS_PER_EPOCH];
#endif //CRYSTAL_LOGGING


static void app_init() {
}

static inline void app_pre_S() {
  app_have_packet = 0;
  log_send_seqn = 0;
  app_n_packets = 0;
}

static inline void app_new_packet() {
  app_seqn ++;
#if CRYSTAL_LOGGING
  app_packets[app_n_packets].seqn = app_seqn;
  app_packets[app_n_packets].acked = 0;
#endif //CRYSTAL_LOGGING
  app_n_packets ++;
}

static inline void app_mark_acked() {
#if CRYSTAL_LOGGING
  app_packets[app_n_packets-1].acked = 1;
#endif //CRYSTAL_LOGGING
}

static inline void app_post_S(int received) {
  if (IS_SINK())
    return;
#if CONCURRENT_TXS > 0
  int i;
  int cur_idx;
  if (epoch >= START_EPOCH) {
    cur_idx = ((epoch - START_EPOCH) % NUM_ACTIVE_EPOCHS) * CONCURRENT_TXS;
    for (i=0; i<CONCURRENT_TXS; i++) {
      if (node_id == sndtbl[cur_idx + i]) {
        app_have_packet = 1;
        app_new_packet();
        break;
      }
    }
  }
#endif // CONCURRENT_TXS
}

static inline int app_pre_T() {
  log_send_acked = 0;
  if (app_have_packet) {
    crystal_data->app.seqn = app_seqn;
    crystal_data->app.src = node_id;
    log_send_seqn = app_seqn;  // for logging
    return 1;
  }
  return 0;
}

static inline void app_between_TA(int received) {
  if (received) {
    log_recv_src = crystal_data->app.src;
    log_recv_seqn = crystal_data->app.seqn;
    if (IS_SINK()) {
      // fill in the ack payload
      crystal_ack->app.seqn=log_recv_seqn;
      crystal_ack->app.src=log_recv_src;
    }
  }
  else {
    crystal_ack->app.seqn=NO_SEQN;
    crystal_ack->app.src=NO_NODE;
  }
}

static inline void app_post_A(int received) {
  // non-sink: if acked us, stop sending data
  log_send_acked = 0;
  if (app_have_packet && received) {
    if ((crystal_ack->app.src == node_id) && (crystal_ack->app.seqn == app_seqn)) {
      log_send_acked = 1;
      app_mark_acked();
      if (app_n_packets < PACKETS_PER_EPOCH) {
        app_new_packet();
      }
      else {
        app_have_packet = 0;
      }
    }
  }
}


static inline void app_epoch_end() {}
static inline void app_ping() {}

#if CRYSTAL_LOGGING
void app_print_logs() {
  if (!IS_SINK()) {
    int i;
    for (i=0; i<app_n_packets; i++) {
      printf("A %u:%u %u %u\n", epoch, 
          app_packets[i].seqn, 
          app_packets[i].acked, 
          app_log_seqn);
      app_log_seqn ++;
    }
  }
}
#endif //CRYSTAL_LOGGING
