#include "crystal-test.h"
#include "crystal.h"
#include "node-id.h"
#include "ds2411.h"
#include "etimer.h"

static const uint8_t sndtbl[] = {SENDERS_TABLE};

static uint16_t app_have_packet;
static uint16_t app_seqn;
static uint16_t app_log_seqn;
static uint16_t n_pkt_sent;
static uint16_t n_pkt_recv;

static app_t_payload t_payload;
static app_a_payload a_payload;

static int is_sink;

#define PACKETS_PER_EPOCH 1
#define LOGGING 1

#if LOGGING
struct out_pkt_record {
  uint16_t seqn;
  uint16_t acked;
  
} out_packets[PACKETS_PER_EPOCH];

#define RECV_PACKET_NUM 100
struct in_pkt_record {
  uint16_t src;
  uint16_t seqn;
} in_packets[RECV_PACKET_NUM];

#endif //LOGGING

uint8_t* app_pre_S() {
  app_have_packet = 0;
  //log_send_seqn = 0;
  n_pkt_sent = 0;
  n_pkt_recv = 0;
  return NULL;
}

static inline void app_new_packet() {
  app_seqn ++;
#if LOGGING
  out_packets[n_pkt_sent].seqn = app_seqn;
  out_packets[n_pkt_sent].acked = 0;
#endif //LOGGING
  n_pkt_sent ++;
}

static inline void app_mark_acked() {
#if LOGGING
  out_packets[n_pkt_sent-1].acked = 1;
#endif //LOGGING
}

void app_post_S(int received, uint8_t* payload) {
  if (is_sink)
    return;
#if CONCURRENT_TXS > 0
  int i;
  int cur_idx;
  if (crystal_info.epoch >= START_EPOCH) {
    cur_idx = ((crystal_info.epoch - START_EPOCH) % NUM_ACTIVE_EPOCHS) * CONCURRENT_TXS;
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
  //log_send_acked = 0;
  if (app_have_packet) {
    t_payload.seqn = app_seqn;
    t_payload.src  = node_id;
    //log_send_seqn  = app_seqn;  // for logging
    return (uint8_t*)&t_payload;
  }
  return NULL;
}

uint8_t* app_between_TA(int received, uint8_t* payload) {
  if (received) {
    t_payload = *(app_t_payload*)payload;

    //log_recv_src  = t_payload.src;
    //log_recv_seqn = t_payload.seqn;
  }
  if (received && is_sink) {
      // fill in the ack payload
      a_payload.src  = t_payload.src;
      a_payload.seqn = t_payload.seqn;

#if LOGGING
      if (n_pkt_recv < RECV_PACKET_NUM) {
        in_packets[n_pkt_recv].src = t_payload.src;
        in_packets[n_pkt_recv].seqn = t_payload.seqn;
        n_pkt_recv ++;
      }
#endif
  }
  else {
    a_payload.seqn = NO_SEQN;
    a_payload.src  = NO_NODE;
  }
  return (uint8_t*)&a_payload;
}

void app_post_A(int received, uint8_t* payload) {
  // non-sink: if acked us, stop sending data
  //log_send_acked = 0;
  if (app_have_packet && received) {
    a_payload = *(app_a_payload*)payload;
    
    if ((a_payload.src == node_id) && (a_payload.seqn == app_seqn)) {
      //log_send_acked = 1;
      app_mark_acked();
      if (n_pkt_sent < PACKETS_PER_EPOCH) {
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

void app_print_logs() {
#if LOGGING
  int i;
  if (is_sink) {
    for (i=0; i<n_pkt_recv; i++) {
      printf("B %u:%u %u %u\n", crystal_info.epoch, 
          in_packets[i].src, 
          in_packets[i].seqn,
          app_log_seqn);
      app_log_seqn ++;
    }
  }
  else {
    for (i=0; i<n_pkt_sent; i++) {
      printf("A %u:%u %u %u\n", crystal_info.epoch, 
          out_packets[i].seqn, 
          out_packets[i].acked, 
          app_log_seqn);
      app_log_seqn ++;
    }
  }
#endif //LOGGING
}




PROCESS(crystal_test, "Crystal test");
AUTOSTART_PROCESSES(&crystal_test);
PROCESS_THREAD(crystal_test, ev, data) {
  PROCESS_BEGIN();

  static struct etimer et;

  etimer_set(&et, CLOCK_SECOND);
  PROCESS_YIELD_UNTIL(etimer_expired(&et));


  printf("I am alive! EUI-64: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", ds2411_id[0],ds2411_id[1],ds2411_id[2],ds2411_id[3],ds2411_id[4],ds2411_id[5],ds2411_id[6],ds2411_id[7]);

  is_sink = node_id == SINK_ID;

  crystal_config_t conf = crystal_get_config();
  conf.is_sink = is_sink;
  conf.plds_S = sizeof(app_s_payload);
  conf.plds_T = sizeof(app_t_payload);
  conf.plds_A = sizeof(app_a_payload);

  crystal_start(&conf);

  PROCESS_END();
}



PROCESS_THREAD(alive_print_process, ev, data) {
  PROCESS_BEGIN();
  PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
  PROCESS_END();
}
