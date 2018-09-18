#ifndef CRYSTAL_H_
#define CRYSTAL_H_

#include "glossy.h"


//#define CRYSTAL_INTER_PHASE_GAP (RTIMER_SECOND / 250) // 4 ms
#define CRYSTAL_INTER_PHASE_GAP (RTIMER_SECOND / 500) // 2 ms
//#define CRYSTAL_INTER_PHASE_GAP (RTIMER_SECOND / 625) // 1.6 ms

//#define CRYSTAL_SHORT_GUARD           5
#define CRYSTAL_SHORT_GUARD           5
#define CRYSTAL_SHORT_GUARD_NOSYNC    5

#define CRYSTAL_SINK_END_GUARD  8  // end guard for T slot at sink (give a bit more time to receive packets)

//#define CRYSTAL_LONG_GUARD       (16*(CRYSTAL_LONGSKIP+1))
//#define CRYSTAL_LONG_GUARD       (16+CRYSTAL_LONGSKIP)
//#define CRYSTAL_LONG_GUARD       16   // ~488 us
#define CRYSTAL_LONG_GUARD        5    // ~150 us


// The receivers capture the reference at SFD while the sender uses STXON as
// the reference (which happens 22 symbols earlier).
// We need to compensate it on the non-sink devices.
//
// In fact the delay consists of the following:
// 1. 12 symbols of the "turnaround time" used for calibration
// 2. preamble 8 symbols
// 3. SFD 2 symbols
// 22 symbols in total
//
// TODO 1: check that the parameters are left default in Glossy. DONE: yes, they are (MDMCTRL0, TXCTRL in cc2420.c)
// TODO 2: check that Cooja models them correctly. DONE: no, Cooja sets the calibration to 12+2=14 symbols

// Therefore, for Cooja we have 14+8+2 = 24 symbols, or 24*16 = 384 us, or 384/1000000*32768 = 12.6 ticks
// for real cc2420, 12+8+2 = 22 symbols, or 22*16 = 352 us, or 352/1000000*32768 = 11.5 ticks
// TODO: there is also a program delay between firing the timer and sending the STXON. How long is it?

#if COOJA
#define CRYSTAL_REF_SHIFT 13   // 397us
#else
#define CRYSTAL_REF_SHIFT 10   // 305us IPSN'18 submission
//#define CRYSTAL_REF_SHIFT 12   // 366us // this seems to be the correct setting but 10 works slightly better
#endif

/**
 * Guard-time when clock skew is not yet estimated
 */
//#define CRYSTAL_INIT_GUARD  (RTIMER_SECOND / 50)     //  20 ms, IPSN'18
#define CRYSTAL_INIT_GUARD  (RTIMER_SECOND / 100)    //  10 ms

/**
 * Duration during bootstrapping at receivers.
 */
#define CRYSTAL_SCAN_SLOT_DURATION    (RTIMER_SECOND / 20) //  50 ms

typedef uint8_t crystal_addr_t; // IPSN'18
//typedef uint16_t crystal_addr_t;

//typedef uint8_t crystal_epoch_t; // NOT SUPPORTED !
typedef uint16_t crystal_epoch_t; // IPSN'18

#define CRYSTAL_TYPE_SYNC 0x01
#define CRYSTAL_TYPE_DATA 0x02
#define CRYSTAL_TYPE_ACK  0x03

#define CRYSTAL_ACK_AWAKE(ack) ((ack).cmd == 0x11)
#define CRYSTAL_ACK_SLEEP(ack) ((ack).cmd == 0x22)

#define CRYSTAL_SET_ACK_AWAKE(ack) ((ack).cmd = 0x11)
#define CRYSTAL_SET_ACK_SLEEP(ack) ((ack).cmd = 0x22)

#define CRYSTAL_ACK_CMD_CORRECT(ack) (CRYSTAL_ACK_AWAKE(ack) || CRYSTAL_ACK_SLEEP(ack))


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

#endif /* CRYSTAL_H_ */
