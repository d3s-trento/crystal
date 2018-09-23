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

/* Crystal application interface (callbacks) */
uint8_t* app_pre_S();
void app_post_S(int received, uint8_t* payload);
uint8_t* app_pre_T();
uint8_t* app_between_TA(int received, uint8_t* payload);
void app_post_A(int received, uint8_t* payload);
void app_epoch_end();
void app_ping();
void app_print_logs();

bool crystal_start(crystal_config_t* conf);
crystal_config_t crystal_get_config();

typedef struct {
  crystal_epoch_t epoch;
  //uint16_t n_ta;
} crystal_info_t;

extern crystal_info_t crystal_info;

#endif /* CRYSTAL_H_ */
