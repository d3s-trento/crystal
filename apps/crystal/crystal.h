#ifndef CRYSTAL_H_
#define CRYSTAL_H_

#include "glossy.h"

#define CRYSTAL_PERIOD     (CRYSTAL_CONF_PERIOD*RTIMER_SECOND)

//#define CRYSTAL_SINK_MAX_EMPTY_TS 2  // sink: max allowed number of empty Ts in a row
//#define CRYSTAL_MAX_SILENT_TAS 2    // node: max allowed number of empty TA pairs in a row
//#define CRYSTAL_MAX_SILENT_TAS CRYSTAL_SINK_MAX_EMPTY_TS
//#define CRYSTAL_MAX_MISSING_ACKS 4    // node: max allowed number of missing acks in a row
//#define CRYSTAL_SINK_MAX_NOISY_TS 4    // sink: max allowed number of noisy empty Ts in a row

#define CRYSTAL_MAX_NOISY_AS CRYSTAL_SINK_MAX_NOISY_TS // node: max allowed number of noisy empty As

//#define CRYSTAL_PAYLOAD_LENGTH 0      // the length of the application data payload

//#define CCA_THRESHOLD -32
//#define CCA_COUNTER_THRESHOLD 80


//#define CRYSTAL_SYNC_ACKS 1

/**
 * Duration of each Glossy phase.
 */
#define DUR_S ((uint32_t)RTIMER_SECOND*DUR_S_MS/1000)
#define DUR_T ((uint32_t)RTIMER_SECOND*DUR_T_MS/1000)
#define DUR_A ((uint32_t)RTIMER_SECOND*DUR_A_MS/1000)


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


/**
 * Length of data structure.
 */
#define CRYSTAL_SYNC_LEN sizeof(crystal_sync_struct)
#define CRYSTAL_DATA_LEN sizeof(crystal_data_struct)
#define CRYSTAL_ACK_LEN sizeof(crystal_ack_struct)

#define CRYSTAL_TYPE_SYNC 0x01
#define CRYSTAL_TYPE_DATA 0x02
#define CRYSTAL_TYPE_ACK  0x03

#define CRYSTAL_ACK_AWAKE(ack) ((ack)->cmd == 0x11)
#define CRYSTAL_ACK_SLEEP(ack) ((ack)->cmd == 0x22)

#define CRYSTAL_SET_ACK_AWAKE(ack) ((ack)->cmd = 0x11)
#define CRYSTAL_SET_ACK_SLEEP(ack) ((ack)->cmd = 0x22)

#define CRYSTAL_ACK_CMD_CORRECT(ack) (CRYSTAL_ACK_AWAKE(ack) || CRYSTAL_ACK_SLEEP(ack))




#endif /* CRYSTAL_H_ */
