#include <depcomp-pins.c>

static uint16_t app_seqn;
static uint16_t app_epoch;
static rtimer_clock_t app_epoch_start_time;

app_state_t root_global_view;

// How many TAs to wait before actuating to 0 for many-to-one scenarios
#define APP_WAIT_MTO_TAS 2
uint16_t mto_ta_cnt;


#define PIN_ID_118   0
#define PIN_ID_119   1
#define PIN_ID_206   2
#define PIN_ID_117   3
#define PIN_ID_207   4
#define PIN_ID_226   5
#define PIN_ID_213   6
#define PIN_ID_213_1 7
#define PIN_ID_219   8
#define PIN_ID_110   9
#define PIN_ID_201   10

struct srcpin {
  const volatile uint8_t *port;
  uint8_t pin;
};


#define N_ALL_SRC_PINS 11 // the length ot the array below

struct srcpin all_srcpins[] = {
  {INP_TUPLE(18)}, // PIN_ID_118  
  {INP_TUPLE(23)}, // PIN_ID_119  
  {INP_TUPLE(24)}, // PIN_ID_206  
  {INP_TUPLE(27)}, // PIN_ID_117  
  {INP_TUPLE(27)}, // PIN_ID_207  
  {INP_TUPLE(27)}, // PIN_ID_226  
  {INP_TUPLE(4)},  // PIN_ID_213  
  {INP_TUPLE(17)}, // PIN_ID_213_1
  {INP_TUPLE(25)}, // PIN_ID_219  
  {INP_TUPLE(25)}, // PIN_ID_110  
  {INP_TUPLE(22)}, // PIN_ID_201  
};

#define N_MAX_SRC_PINS    2 // max number of pins per source
#define N_MAX_INPUT_PINS  3 // max number of source pins per actuator
#define N_MAX_OUTPUT_PINS 2 // max number of actuated pins per node


struct my_srcpin {
  uint8_t pin_id;
  uint8_t state;    // local src pin state last time we checked
};

struct my_srcpin my_srcpins[N_MAX_SRC_PINS]; // input pins that the current node probes
uint8_t src_pins_cnt;

static void add_my_srcpin(uint8_t pin_id) {
  my_srcpins[src_pins_cnt].pin_id = pin_id;
  src_pins_cnt ++;
}

// pack the pin into a global state variable
static inline void pack_pin(app_state_t *global_state, uint8_t pin_id, app_state_t pin_state) {
  app_state_t mask = (app_state_t)1 << pin_id;
  *global_state &= ~mask;
  *global_state |= (pin_state & 1) << pin_id;
}

// pack the pins I am the source for to a global state variable
static inline void pack_my_pins(app_state_t *global_state) {
  int i;
  for (i=0; i<src_pins_cnt; i++) {
    pack_pin(global_state, my_srcpins[i].pin_id, my_srcpins[i].state);
  }
}

static inline uint8_t unpack_pin(app_state_t global_state, uint8_t pin_id) {
  return (global_state & ((app_state_t)1 << pin_id)) != 0;
}


#define IS_SOURCE() (src_pins_cnt != 0)

static uint8_t act_input_cnt;

struct act_output {
  volatile uint8_t *port;
  uint8_t pin;
  uint8_t n_inputs;
  uint8_t input_pin_ids[N_MAX_INPUT_PINS]; // source pin IDs for this output
};

struct act_output outputs[N_MAX_OUTPUT_PINS];
static uint8_t act_output_cnt;

// add an output pin
static uint8_t add_output_record(volatile uint8_t* port, uint8_t pin) {
  outputs[act_output_cnt].port = port;
  outputs[act_output_cnt].pin = pin;
  return act_output_cnt ++;
}

// add an input and register it with the output
static uint8_t add_input_record(uint8_t pin_id, uint8_t out_idx) {
  outputs[out_idx].input_pin_ids[outputs[out_idx].n_inputs] = pin_id;
  outputs[out_idx].n_inputs ++;
  return act_input_cnt ++;
}


static void app_init() {
  uint8_t out_idx;
  switch (node_id) {
    // inputs ----------------------------------------
    case 118: GPIO_MKIN(18); // 1
              add_my_srcpin(PIN_ID_118);
              break;
    case 119: GPIO_MKIN(23); // 2
              add_my_srcpin(PIN_ID_119);
              break;
    case 206: GPIO_MKIN(24); // 3
              add_my_srcpin(PIN_ID_206);
              break;
    case 117: GPIO_MKIN(27); // 4
              add_my_srcpin(PIN_ID_117);
              break;
    case 207: GPIO_MKIN(27); // 4
              add_my_srcpin(PIN_ID_207);
              break;
    case 226: GPIO_MKIN(27); // 4
              add_my_srcpin(PIN_ID_226);
              break;
    case 213: GPIO_MKIN(4);  // 5
              add_my_srcpin(PIN_ID_213);
              GPIO_MKIN(17); // 6
              add_my_srcpin(PIN_ID_213_1);
              break;
    case 219: GPIO_MKIN(25); // 7
              add_my_srcpin(PIN_ID_219);
              break;
    case 110: GPIO_MKIN(25); // 7
              add_my_srcpin(PIN_ID_110);
              break;
    case 201: GPIO_MKIN(22); // 8
              add_my_srcpin(PIN_ID_201);
              break;
    // outputs ---------------------------------------
    case 209: GPIO_MKOUT(18); GPIO_SET(18, 0); // 1
              out_idx = add_output_record(OUTP_TUPLE(18));
              add_input_record(PIN_ID_118, out_idx);

              GPIO_MKOUT(22); GPIO_SET(22, 0); // 8
              out_idx = add_output_record(OUTP_TUPLE(22));
              add_input_record(PIN_ID_201, out_idx);
              break;

    case 217: GPIO_MKOUT(23); GPIO_SET(23, 0); // 2
              out_idx = add_output_record(OUTP_TUPLE(23));
              add_input_record(PIN_ID_119, out_idx);

              break;
    case 224: GPIO_MKOUT(23); GPIO_SET(23, 0); // 2
              out_idx = add_output_record(OUTP_TUPLE(23));
              add_input_record(PIN_ID_119, out_idx);

              GPIO_MKOUT(22); GPIO_SET(22, 0); // 8
              out_idx = add_output_record(OUTP_TUPLE(22));
              add_input_record(PIN_ID_201, out_idx);
              break;
    case 210: GPIO_MKOUT(24); GPIO_SET(24, 0); // 3
              out_idx = add_output_record(OUTP_TUPLE(24));
              add_input_record(PIN_ID_206, out_idx);
              break;
    case 222: GPIO_MKOUT(27); GPIO_SET(27, 0); // 4
              out_idx = add_output_record(OUTP_TUPLE(27));
              add_input_record(PIN_ID_117, out_idx);
              add_input_record(PIN_ID_207, out_idx);
              add_input_record(PIN_ID_226, out_idx);
              break;
    case 225: GPIO_MKOUT(4); GPIO_SET(4, 0);   // 5
              out_idx = add_output_record(OUTP_TUPLE(4));
              add_input_record(PIN_ID_213, out_idx);

              GPIO_MKOUT(22); GPIO_SET(22, 0); // 8
              out_idx = add_output_record(OUTP_TUPLE(22));
              add_input_record(PIN_ID_201, out_idx);
              break;
    case 108:
    case 200: GPIO_MKOUT(17); GPIO_SET(17, 0); // 6
              out_idx = add_output_record(OUTP_TUPLE(17));
              add_input_record(PIN_ID_213_1, out_idx);
              break;
    case 220: GPIO_MKOUT(25); GPIO_SET(25, 0); // 7
              out_idx = add_output_record(OUTP_TUPLE(25));
              add_input_record(PIN_ID_219, out_idx);
              add_input_record(PIN_ID_110, out_idx);
              break;
    case 211: GPIO_MKOUT(22); GPIO_SET(22, 0); // 8
              out_idx = add_output_record(OUTP_TUPLE(22));
              add_input_record(PIN_ID_201, out_idx);
              break;
  }
}

static inline void capture_state_change() {
  int i;
  for (i=0; i<src_pins_cnt; i++) {
    uint8_t pin_id = my_srcpins[i].pin_id;
    uint8_t new_state = gpio_get(all_srcpins[pin_id].port, all_srcpins[pin_id].pin);

    my_srcpins[i].state = new_state;
  }
}

/* Change the output pins in function of the 
 *  - root global pin view
 */
static void actuate() {
  int act_i, src_i;

  for (act_i=0; act_i<act_output_cnt; act_i++) {
    uint8_t n_inputs = outputs[act_i].n_inputs;
    volatile uint8_t* port = outputs[act_i].port;
    uint8_t pin = outputs[act_i].pin;
    if (n_inputs == 1) { // one-to-one logic
      gpio_set(port, pin, unpack_pin(root_global_view, outputs[act_i].input_pin_ids[0]));
    }
    else if (n_inputs > 1) { // many-to-one logic
      uint8_t decision = 0;
      for (src_i=0; src_i<n_inputs && !decision; src_i++) {
        decision = decision || unpack_pin(root_global_view, outputs[act_i].input_pin_ids[src_i]);
      }
      if (decision) {
        gpio_set(port, pin, decision);
        mto_ta_cnt = 0;
      }
      else {
        if (mto_ta_cnt >= APP_WAIT_MTO_TAS) {
          gpio_set(port, pin, 0);
          mto_ta_cnt = 0; 
        }
      }
    }
  }
}


static inline int fill_in_t_packet() {
  int i;
  for (i=0; i<src_pins_cnt; i++) {
    uint8_t pin_id = my_srcpins[i].pin_id;
    uint8_t my_pin_state = my_srcpins[i].state;
    if (((root_global_view >> pin_id) & 1) != my_pin_state) {
      // my pin is different from what the root knows
      app_seqn ++;

      crystal_data->app.pin_id_and_state = pin_id | (my_pin_state << 7);
      log_send_seqn = app_seqn;  // for logging only
      return 1;
    }
  }
  return 0; // no packet
}

static inline void app_ping() {
  capture_state_change();
}

static inline void app_pre_S() {
  app_epoch ++;
  app_epoch_start_time = RTIMER_NOW();
  if (IS_SINK()) {
    if (IS_SOURCE()) { // if the sink is also a source
      capture_state_change();
      pack_my_pins(&root_global_view);
    }
    crystal_sync->app.pin_state = root_global_view;
  }
}

static inline void app_post_S(int received) {
  if (!IS_SINK() && received) {
    root_global_view = crystal_sync->app.pin_state;
    actuate();
  }
}

static inline int app_pre_T() {
  // if local pin state is different from what was acked, send a packet
  if (!IS_SOURCE())
    return 0;

  if (!IS_SINK()) {
    capture_state_change();
    return fill_in_t_packet();
  }
  return 0;
}

static inline void app_between_TA(int received) {
  // if root and have received new state, ACK it.
  if (received) {
    log_recv_src = 0;

    if (IS_SINK()) {
      uint8_t pin_id = crystal_data->app.pin_id_and_state & 0x7f;
      uint8_t pin_state = crystal_data->app.pin_id_and_state >> 7;
      if (pin_id < N_ALL_SRC_PINS) { // sanity check
        pack_pin(&root_global_view, pin_id, pin_state);
      }
    }
  }

  if (IS_SINK()) {
    if (IS_SOURCE()) { // if the sink is also a source
      capture_state_change();
      pack_my_pins(&root_global_view);
    }
    // flood the current state
    crystal_ack->app.pin_state = root_global_view;
    actuate();
  }
}


static inline void app_post_A(int received) {
  if (received) {
    root_global_view = crystal_ack->app.pin_state;
  }
  actuate();
  mto_ta_cnt ++;
}

static inline void app_epoch_end() {
  actuate();
}

void app_print_logs() {}
