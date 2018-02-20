/*
// GPIO 17
P6SEL &= ~(BIT0);
P6DIR |= BIT0;
// GPIO 4
P6SEL &= ~(BIT1);
P6DIR |= BIT1;
// GPIO 18
P6SEL &= ~(BIT2);
P6DIR |= BIT2;
// GPIO 27
P6SEL &= ~(BIT3);
P6DIR |= BIT3;
// GPIO 25
P6SEL &= ~(BIT6);
P6DIR |= BIT6;
// GPIO 22
P6SEL &= ~(BIT7);
P6DIR |= BIT7;
// GPIO 23
P2SEL &= ~(BIT3);
P2DIR |= BIT3;
// GPIO 24
P2SEL &= ~(BIT6);
P2DIR |= BIT6;
*/



#if COOJA

uint8_t cooja_bit_17,
        cooja_bit_4,
        cooja_bit_18,
        cooja_bit_27,
        cooja_bit_25,
        cooja_bit_22,
        cooja_bit_23,
        cooja_bit_24;

#define GPIO_MKOUT(p) do {} while(0)
#define GPIO_MKIN(p) do {} while(0)
#define GPIO_GET(p) (cooja_bit_##p != 0) 
#define GPIO_SET(p, x) do {cooja_bit_##p = ((x) != 0);} while(0)

#define INP_TUPLE(p)  &cooja_bit_##p, 1
#define OUTP_TUPLE(p) &cooja_bit_##p, 1

#else // not COOJA

#define SEL_17 P6SEL
#define SEL_4  P6SEL
#define SEL_18 P6SEL
#define SEL_27 P6SEL
#define SEL_25 P6SEL
#define SEL_22 P6SEL
#define SEL_23 P2SEL
#define SEL_24 P2SEL

#define DIR_17 P6DIR
#define DIR_4  P6DIR
#define DIR_18 P6DIR
#define DIR_27 P6DIR
#define DIR_25 P6DIR
#define DIR_22 P6DIR
#define DIR_23 P2DIR
#define DIR_24 P2DIR

#define OUT_17 P6OUT
#define OUT_4  P6OUT
#define OUT_18 P6OUT
#define OUT_27 P6OUT
#define OUT_25 P6OUT
#define OUT_22 P6OUT
#define OUT_23 P2OUT
#define OUT_24 P2OUT

#define IN_17 P6IN
#define IN_4  P6IN
#define IN_18 P6IN
#define IN_27 P6IN
#define IN_25 P6IN
#define IN_22 P6IN
#define IN_23 P2IN
#define IN_24 P2IN

#define BIT_17 BIT0
#define BIT_4  BIT1
#define BIT_18 BIT2
#define BIT_27 BIT3
#define BIT_25 BIT6
#define BIT_22 BIT7
#define BIT_23 BIT3
#define BIT_24 BIT6

#define INP_TUPLE(p) &IN_##p, BIT_##p
#define OUTP_TUPLE(p) &OUT_##p, BIT_##p 



#define GPIO_MKOUT(p) do { SEL_##p &= ~(BIT_##p); DIR_##p |= (BIT_##p); } while(0)
#define GPIO_MKIN(p) do { SEL_##p &= ~(BIT_##p); DIR_##p &= ~(BIT_##p); } while(0)
#define GPIO_GET(p) ((IN_##p & BIT_##p) != 0) 
#define GPIO_SET(p, x) do {if (x) {OUT_##p |= (BIT_##p);} else {OUT_##p &= ~(BIT_##p);}} while(0)

#endif // COOJA

static inline void gpio_set(volatile uint8_t *p, uint8_t bit, uint8_t v) {
  if (v) {*p |= bit;} else {*p &= ~bit;}
}

static inline uint8_t gpio_get(const volatile uint8_t *p, uint8_t bit) {
  return (*p & bit) != 0;
}
