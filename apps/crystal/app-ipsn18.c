#include "sndtbl.c"

static uint16_t app_have_packet;
static uint16_t app_seqn;
static uint16_t app_log_seqn;
static uint16_t app_n_packets;

static app_t_payload t_payload;
static app_a_payload a_payload;

#define PACKETS_PER_EPOCH 1

#if CRYSTAL_LOGGING
struct pkt_record {
  uint16_t seqn;
  uint16_t acked;
  
} app_packets[PACKETS_PER_EPOCH];
#endif //CRYSTAL_LOGGING


static void app_init() {
}

uint8_t* app_pre_S() {
  app_have_packet = 0;
  log_send_seqn = 0;
  app_n_packets = 0;
  return NULL;
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

void app_post_S(int received, uint8_t* payload) {
  if (conf.is_sink)
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

uint8_t* app_pre_T() {
  log_send_acked = 0;
  if (app_have_packet) {
    t_payload.seqn = app_seqn;
    t_payload.src  = node_id;
    log_send_seqn  = app_seqn;  // for logging
    return (uint8_t*)&t_payload;
  }
  return NULL;
}

uint8_t* app_between_TA(int received, uint8_t* payload) {
  if (received) {
    t_payload = *(app_t_payload*)payload;

    log_recv_src  = t_payload.src;
    log_recv_seqn = t_payload.seqn;
  }
  if (received && conf.is_sink) {
      // fill in the ack payload
      a_payload.seqn = log_recv_seqn;
      a_payload.src  = log_recv_src;
  }
  else {
    a_payload.seqn = NO_SEQN;
    a_payload.src  = NO_NODE;
  }
  return (uint8_t*)&a_payload;
}

void app_post_A(int received, uint8_t* payload) {
  // non-sink: if acked us, stop sending data
  log_send_acked = 0;
  if (app_have_packet && received) {
    a_payload = *(app_a_payload*)payload;
    
    if ((a_payload.src == node_id) && (a_payload.seqn == app_seqn)) {
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


void app_epoch_end() {}
void app_ping() {}

#if CRYSTAL_LOGGING
void app_print_logs() {
  if (!conf.is_sink) {
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
