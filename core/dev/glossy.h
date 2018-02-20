/*
 * Copyright (c) 2011, ETH Zurich.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author: Federico Ferrari <ferrari@tik.ee.ethz.ch>
 *
 */

/**
 * \file
 *         Glossy core, header file.
 * \author
 *         Federico Ferrari <ferrari@tik.ee.ethz.ch>
 */

#ifndef GLOSSY_H_
#define GLOSSY_H_

#include "contiki.h"
#include "dev/watchdog.h"
#include "dev/cc2420_const.h"
#include "dev/leds.h"
#include "dev/spi.h"
#include <stdio.h>
#include <legacymsp430.h>
#include <stdlib.h>

/**
 * If not zero, nodes print additional debug information (disabled by default).
 */
#define GLOSSY_DEBUG 0
/**
 * Size of the window used to average estimations of slot lengths.
 */
#define GLOSSY_SYNC_WINDOW            64
/**
 * Initiator timeout, in number of Glossy slots.
 * When the timeout expires, if the initiator has not received any packet
 * after its first transmission it transmits again.
 */
#define GLOSSY_INITIATOR_TIMEOUT      3

/**
 * Ratio between the frequencies of the DCO and the low-frequency clocks
 */
#if COOJA
#define CLOCK_PHI                     (4194304uL / RTIMER_SECOND)
#else
#define CLOCK_PHI                     (F_CPU / RTIMER_SECOND)
#endif /* COOJA */

#define GLOSSY_HEADER                 0xa0
#define GLOSSY_HEADER_MASK            0xf0
#define GLOSSY_APP_HEADER_MASK        0x07
#define GLOSSY_HEADER_SYNC_BIT        0x08
#define GLOSSY_HEADER_LEN             sizeof(uint8_t)
#define GLOSSY_RELAY_CNT_LEN          sizeof(uint8_t)
#define GLOSSY_IS_ON()                (get_state() != GLOSSY_STATE_OFF)
#define FOOTER_LEN                    2
#define FOOTER1_CRC_OK                0x80
#define FOOTER1_CORRELATION           0x7f

#define GLOSSY_LEN_FIELD              packet[0]
#define GLOSSY_HEADER_FIELD           packet[1]
#define GLOSSY_DATA_FIELD             packet[2]
#define GLOSSY_RELAY_CNT_FIELD        packet[packet_len_tmp - FOOTER_LEN]
#define GLOSSY_RSSI_FIELD             packet[packet_len_tmp - 1]
#define GLOSSY_CRC_FIELD              packet[packet_len_tmp]

enum {
  GLOSSY_INITIATOR = 1, GLOSSY_RECEIVER = 0
};

enum {
  GLOSSY_SYNC = 1, GLOSSY_NO_SYNC = 0
};
/**
 * List of possible Glossy states.
 */
enum glossy_state {
  GLOSSY_STATE_OFF,          /**< Glossy is not executing */
  GLOSSY_STATE_WAITING,      /**< Glossy is waiting for a packet being flooded */
  GLOSSY_STATE_RECEIVING,    /**< Glossy is receiving a packet */
  GLOSSY_STATE_RECEIVED,     /**< Glossy has just finished receiving a packet */
  GLOSSY_STATE_TRANSMITTING, /**< Glossy is transmitting a packet */
  GLOSSY_STATE_TRANSMITTED,  /**< Glossy has just finished transmitting a packet */
  GLOSSY_STATE_ABORTED       /**< Glossy has just aborted a packet reception */
};
#if GLOSSY_DEBUG
unsigned int high_T_irq, rx_timeout, bad_length, bad_header, bad_crc;
#endif /* GLOSSY_DEBUG */


/* Glossy persistent state. All fields should be initialized with zeroes*/
struct glossy {
  rtimer_clock_t T_slot_h;
  unsigned long T_slot_h_sum;
  uint8_t win_cnt;
};

// Upper bound of the time needed for a Glossy initiator 
// for preparations before it sends STXON
#define GLOSSY_PRE_TIME 16 // IPSN'18 setting


PROCESS_NAME(glossy_process);

/* ----------------------- Application interface -------------------- */
/**
 * \defgroup glossy_interface Glossy API
 * @{
 * \file   glossy.h
 * \file   glossy.c
 */

/**
 * \defgroup glossy_main Interface related to flooding
 * @{
 */

/**
 * \brief            Start Glossy and stall all other application tasks.
 *
 * \param data_      A pointer to the flooding data.
 *
 *                   At the initiator, Glossy reads from the given memory
 *                   location data provided by the application.
 *
 *                   At a receiver, Glossy writes to the given memory
 *                   location data for the application.
 * \param data_len_  Length of the flooding data, in bytes.
 * \param initiator_ Not zero if the node is the initiator,
 *                   zero if it is a receiver.
 * \param sync_      Not zero if Glossy must provide time synchronization,
 *                   zero otherwise.
 * \param tx_max_    Maximum number of transmissions (N).
 * \param header_    Application-specific header (value between 0x0 and 0xf).
 * \param t_start_   Time instant at which Glossy must turn on the radio
 *                   (and send the packet if initiator)
 * \param t_stop_    Time instant at which Glossy must stop, in case it is
 *                   still running.
 * \param cb_        Callback function, called when Glossy terminates its
 *                   execution.
 * \param rtimer_    First argument of the callback function.
 * \param ptr_       Second argument of the callback function.
 */
void glossy_start(struct glossy *glossy_,
    uint8_t *data_, uint8_t data_len_, uint8_t initiator_, uint8_t channel_,
    uint8_t sync_, uint8_t tx_max_, uint8_t stop_on_sync_,
    uint8_t header_,
    rtimer_clock_t t_start_, rtimer_clock_t t_stop_, rtimer_callback_t cb_,
    struct rtimer *rtimer_, void *ptr_);

/**
 * \brief            Stop Glossy and resume all other application tasks.
 * \returns          Number of times the packet has been received during
 *                   last Glossy phase.
 *                   If it is zero, the packet was not successfully received.
 * \sa               get_rx_cnt
 */
uint8_t glossy_stop(void);

uint8_t get_data_len(void);
uint8_t get_app_header(void);

/**
 * \brief            Get the last received counter.
 * \returns          Number of times the packet has been received during
 *                   last Glossy phase.
 *                   If it is zero, the packet was not successfully received.
 */
uint8_t get_rx_cnt(void);
uint8_t get_tx_cnt(void);

/**
 * \brief            Get the current Glossy state.
 * \return           Current Glossy state, one of the possible values
 *                   of \link glossy_state \endlink.
 */
uint8_t get_state(void);

/**
 * \brief            Get low-frequency time of first packet reception
 *                   during the last Glossy phase.
 * \returns          Low-frequency time of first packet reception
 *                   during the last Glossy phase.
 */
rtimer_clock_t get_t_first_rx_l(void);

/** @} */

/**
 * \defgroup glossy_sync Interface related to time synchronization
 * @{
 */

/**
 * \brief            Get the last relay counter.
 * \returns          Value of the relay counter embedded in the first packet
 *                   received during the last Glossy phase.
 */
uint8_t get_relay_cnt(void);

/**
 * \brief            Get the local estimation of T_slot, in DCO clock ticks.
 * \returns          Local estimation of T_slot.
 */
rtimer_clock_t get_T_slot_h(void);

/**
 * \brief            Get low-frequency synchronization reference time.
 * \returns          Low-frequency reference time
 *                   (i.e., time at which the initiator started the flood).
 */
rtimer_clock_t get_t_ref_l(void);

/**
 * \brief            Provide information about current synchronization status.
 * \returns          Not zero if the synchronization reference time was
 *                   updated during the last Glossy phase, zero otherwise.
 */
uint8_t is_t_ref_l_updated(void);

/**
 * \brief            Set low-frequency synchronization reference time.
 * \param t          Updated reference time.
 *                   Useful to manually update the reference time if a
 *                   packet has not been received.
 */
void set_t_ref_l(rtimer_clock_t t);

/**
 * \brief            Set the current synchronization status.
 * \param updated    Not zero if a node has to be considered synchronized,
 *                   zero otherwise.
 */
void set_t_ref_l_updated(uint8_t updated);


/**
 * \brief            Return the radio on time for the previous Glossy phase
 */
unsigned long get_rtx_on();

/**
 * \brief            Returns True if a packet with bad CRC was received
 */
uint8_t is_corrupted();

/**
 * \brief            Returns the number of channel-busy CCA samples during the last phase
 */
uint16_t get_cca_busy_cnt();

int cc2420_rssi(void);
void cc2420_set_txpower(uint8_t power);

/** @} */

/** @} */

/**
 * \defgroup glossy_internal Glossy internal functions
 * @{
 * \file   glossy.h
 * \file   glossy.c
 */

/* ------------------------------ Timeouts -------------------------- */
/**
 * \defgroup glossy_timeouts Timeouts
 * @{
 */

inline void glossy_schedule_rx_timeout(void);
inline void glossy_stop_rx_timeout(void);
inline void glossy_schedule_initiator_timeout(void);
inline void glossy_stop_initiator_timeout(void);

/** @} */

/* ----------------------- Interrupt functions ---------------------- */
/**
 * \defgroup glossy_interrupts Interrupt functions
 * @{
 */

inline void glossy_begin_rx(void);
inline void glossy_end_rx(void);
inline void glossy_begin_tx(void);
inline void glossy_end_tx(void);

/** @} */

/**
 * \defgroup glossy_capture Timer capture of clock ticks
 * @{
 */

/* -------------------------- Clock Capture ------------------------- */
/**
 * \brief Capture next low-frequency clock tick and DCO clock value at that instant.
 * \param t_cap_h variable for storing value of DCO clock value
 * \param t_cap_l variable for storing value of low-frequency clock value
 */
#define CAPTURE_NEXT_CLOCK_TICK(t_cap_h, t_cap_l) do {\
    /* Enable capture mode for timers B6 and A2 (ACLK) */\
    TBCCTL6 = CCIS0 | CM_POS | CAP | SCS; \
    TACCTL2 = CCIS0 | CM_POS | CAP | SCS; \
    /* Wait until both timers capture the next clock tick */\
    while (!((TBCCTL6 & CCIFG) && (TACCTL2 & CCIFG))); \
    /* Store the capture timer values */\
    t_cap_h = TBCCR6; \
    t_cap_l = TACCR2; \
    /* Disable capture mode */\
    TBCCTL6 = 0; \
    TACCTL2 = 0; \
} while (0)

/** @} */

/* -------------------------------- SFD ----------------------------- */

/**
 * \defgroup glossy_sfd Management of SFD interrupts
 * @{
 */

/**
 * \brief Capture instants of SFD events on timer B1
 * \param edge Edge used for capture.
 *
 */
#define SFD_CAP_INIT(edge) do {\
  P4SEL |= BV(SFD);\
  TBCCTL1 = edge | CAP | SCS;\
} while (0)

/**
 * \brief Enable generation of interrupts due to SFD events
 */
#define ENABLE_SFD_INT()    do { TBCCTL1 |= CCIE; } while (0)

/**
 * \brief Disable generation of interrupts due to SFD events
 */
#define DISABLE_SFD_INT()    do { TBCCTL1 &= ~CCIE; } while (0)

/**
 * \brief Clear interrupt flag due to SFD events
 */
#define CLEAR_SFD_INT()      do { TBCCTL1 &= ~CCIFG; } while (0)

/**
 * \brief Check if generation of interrupts due to SFD events is enabled
 */
#define IS_ENABLED_SFD_INT()    !!(TBCCTL1 & CCIE)

/** @} */

#endif /* GLOSSY_H_ */

/** @} */
