#ifndef CRYSTAL_H_
#define CRYSTAL_H_

#include "glossy.h"
#include "stdbool.h"

typedef uint8_t crystal_addr_t; // IPSN'18
//typedef uint16_t crystal_addr_t;

//typedef uint8_t crystal_epoch_t; // NOT SUPPORTED !
typedef uint16_t crystal_epoch_t; // IPSN'18


typedef struct {
  uint32_t period;
  uint8_t is_sink;
  uint8_t ntx_S;
  uint16_t w_S;
  uint8_t plds_S;
  uint8_t ntx_T;
  uint16_t w_T;
  uint8_t plds_T;
  uint8_t ntx_A;
  uint16_t w_A;
  uint8_t plds_A;
  uint8_t r;
  uint8_t y;
  uint8_t z;
  uint8_t x;
  uint8_t xa;
  uint16_t ch_whitelist;
  uint8_t enc_enable;
  uint8_t scan_duration;
} crystal_config_t;

/* == Crystal application interface (callbacks) ==============================*/

void app_crystal_start_done(bool success);
uint8_t* app_pre_S();
void app_post_S(int received, uint8_t* payload);
uint8_t* app_pre_T();
uint8_t* app_between_TA(int received, uint8_t* payload);
void app_post_A(int received, uint8_t* payload);
void app_epoch_end();
void app_ping();
void app_print_logs();

void crystal_init();

crystal_config_t crystal_get_config();
bool crystal_start(crystal_config_t* conf);

typedef struct {
  crystal_epoch_t epoch;
  uint16_t n_ta;
  uint16_t n_missed_s;
  uint8_t hops;
} crystal_info_t;

extern crystal_info_t crystal_info;

#define PRINT_CRYSTAL_CONFIG(conf) do {\
 printf("Crystal config. Node ID %x\n", node_id);\
 printf("Period: %lu\n", (conf).period);\
 printf("Sink: %u\n", (conf).is_sink);\
 printf("S: %u %u %u\n", (conf).ntx_S, (conf).w_S, (conf).plds_S);\
 printf("T: %u %u %u\n", (conf).ntx_T, (conf).w_T, (conf).plds_T);\
 printf("A: %u %u %u\n", (conf).ntx_A, (conf).w_A, (conf).plds_A);\
 printf("Term: %u %u %u %u %u\n", (conf).r, (conf).y, (conf).z, (conf).x, (conf).xa);\
 printf("Ch: %x, Enc: %u, Scan: %u\n", (conf).ch_whitelist, (conf).enc_enable, (conf).scan_duration);\
} while (0)

#endif /* CRYSTAL_H_ */
