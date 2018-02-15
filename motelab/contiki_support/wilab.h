
/* this file is only used to generate TestWilabContiki.java with nescc-mig */

enum {
  AM_WILAB_CONTIKI_PRINTF = 66,
};

#define PRINTF_MSG_LENGTH    50

nx_struct wilab_Contiki_printf {
  nx_int8_t buffer[PRINTF_MSG_LENGTH];
};

