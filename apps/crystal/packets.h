#ifndef PACKETS_H
#define PACKETS_H
#include "crystal.h"
#include "app.h"

typedef struct {
  crystal_addr_t src;
  crystal_epoch_t epoch;
} 
__attribute__((packed))
crystal_sync_hdr_t;

typedef struct {
} 
__attribute__((packed))
crystal_data_hdr_t;

typedef struct {
  crystal_epoch_t epoch;
  uint8_t n_ta;
  uint8_t cmd;
}
__attribute__((packed))
crystal_ack_hdr_t;

#endif //PACKETS_H
