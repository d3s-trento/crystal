#ifndef PACKETS_H
#define PACKETS_H
#include "crystal.h"
#include "app.h"

typedef struct {
  crystal_addr_t src;
  crystal_epoch_t epoch;
  app_s_payload app;
} 
__attribute__((packed))
crystal_sync_struct;

typedef struct {
  app_t_payload app;
} 
__attribute__((packed))
crystal_data_struct;

typedef struct {
  crystal_epoch_t epoch;
  uint8_t n_ta;
  uint8_t cmd;
  app_a_payload app;
}
__attribute__((packed))
crystal_ack_struct;

#endif //PACKETS_H
