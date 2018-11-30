#define  CHMAP_nohop        1
#define  CHMAP_nomap        2
#define  CHMAP_split        3
#define  CHMAP_splitbetter  4
#define  CHMAP_comb13       5
#define  CHMAP_comb10       6
#define  CHMAP_comb07       7

#define BSTRAP_nohop 1
#define BSTRAP_hop3  3

#if CRYSTAL_CHHOP_MAPPING == CHMAP_nohop
int channel_array[16] = {[0 ... 15] = RF_CHANNEL}; // no channel hopping
#elif CRYSTAL_CHHOP_MAPPING == CHMAP_nomap 
int channel_array[16] = {11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26}; // no mapping
#elif CRYSTAL_CHHOP_MAPPING == CHMAP_split 
int channel_array[16] = {20,20,20,18,24,20,20,20,18,26,26,26,26,26,26,26}; // Default channel-mapping ("split")
#elif CRYSTAL_CHHOP_MAPPING == CHMAP_splitbetter
int channel_array[16] = {20,20,20,24,24,20,20,20,24,26,26,26,26,26,26,26}; // Default with slightly better channels ("split", used in SenSys17)
#elif CRYSTAL_CHHOP_MAPPING == CHMAP_comb13
int channel_array[16] = {26,26,26,24,24,26,26,26,24,26,26,26,26,26,26,26}; // combined with 13 jammed
#elif CRYSTAL_CHHOP_MAPPING == CHMAP_comb10
int channel_array[16] = {26,26,26,24,24,24,24,24,24,26,26,26,26,26,26,26}; // combined with 10 jammed
#elif CRYSTAL_CHHOP_MAPPING == CHMAP_comb07
int channel_array[16] = {24,24,24,24,24,24,24,24,24,26,26,26,26,26,26,26}; // combined with 7 jammed
#elif CRYSTAL_CHHOP_MAPPING == CHMAP_tmp32
int channel_array[] = {110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235, 240, 245, 250, 255, 260};
#else
#error "Unsupported channel mapping name"
#endif

//int channel_array[16] = {11,12,13,14,15,16,17,18,19,26,26,26,26,26,26,26}; // 9 "natural" + 7 mapped

static inline int get_num_channels() {
  return (sizeof(channel_array)/sizeof(channel_array[0]));
}

// Warning: make sure the hopping sequence holds its properties
// when the epoch (or epoch+ta) wrap around the maximum.
static inline int get_channel_epoch(crystal_epoch_t epoch) {
  return channel_array[(epoch*7) % get_num_channels()];
}

static inline int get_channel_epoch_ta(crystal_epoch_t epoch, uint8_t ta) {
  //return channel_array[((epoch + 3 + ta)*7) % get_num_channels()];
  return channel_array[((7*epoch + ta + 1)*7)% get_num_channels()];
}

enum scan_rx {SCAN_RX_NOTHING, SCAN_RX_S, SCAN_RX_A};

#if CRYSTAL_BSTRAP_CHHOPPING == BSTRAP_nohop
static inline int get_channel_node_bootstrap(){
  return RF_CHANNEL;
}
#elif CRYSTAL_BSTRAP_CHHOPPING == BSTRAP_hop3

enum scan_state {SCAN_ST_CHASE_S, SCAN_ST_SEARCH} scan_state;

static uint32_t        scan_last_hop_ago;
static rtimer_clock_t  scan_last_check_time;
static crystal_epoch_t scan_last_epoch;
static uint8_t         scan_epoch_misses;
static uint8_t         scan_last_ch_index;

#define SCAN_MAX_S_MISSES 5
#define SCAN_START_CHASING_S 0

static inline int get_channel_node_bootstrap(enum scan_rx rx){
  static int first_time = 1;

  if (first_time) {// first time 
    first_time = 0;
    scan_last_check_time = RTIMER_NOW();
#if SCAN_START_CHASING_S
    // root starts with epoch 1, so if we all start roughly at the same time, we'll use the same channel.
    scan_last_epoch = 1;
    scan_state = SCAN_ST_CHASE_S;
    // we might be late for the first S, try it but change to the next one in half-epoch time
    scan_last_hop_ago = conf.period / 2;
    return get_channel_epoch(scan_last_epoch);
#else // SCAN_START_CHASING_S
    scan_last_ch_index = 0;
    scan_state = SCAN_ST_SEARCH;
    scan_last_hop_ago = 0;
    return channel_array[scan_last_ch_index];
#endif // SCAN_START_CHASING_S
  }

  scan_last_hop_ago += RTIMER_NOW() - scan_last_check_time;
  scan_last_check_time = RTIMER_NOW();

  if (rx == SCAN_RX_S) { // caught an S, try to catch the next one
    scan_state = SCAN_ST_CHASE_S;
    scan_epoch_misses = 0;
    scan_last_hop_ago = conf.period / 2;
    scan_last_epoch = epoch; // the current one but will change in half an epoch
    return get_channel_epoch(epoch);
  }
  if (rx == SCAN_RX_A) { // caught an A
    scan_state = SCAN_ST_CHASE_S;
    scan_epoch_misses = 0;
    if (n_ta < CRYSTAL_MAX_TAS - 1) {
      // enough time to catch the next S
      scan_last_hop_ago = 0;
      scan_last_epoch = epoch + 1; // the next one
      return get_channel_epoch(scan_last_epoch);
    }
    else { // might be late for the next S, try it but change to +2 in half-epoch time
      scan_last_hop_ago = conf.period / 2;
      scan_last_epoch = epoch + 1; // the next one
      return get_channel_epoch(epoch);
    }
  }
  else { // not received anything
    if (scan_state == SCAN_ST_CHASE_S) {
      if (scan_epoch_misses > SCAN_MAX_S_MISSES) {
        scan_state = SCAN_ST_SEARCH;
        scan_last_ch_index = 0;
        scan_last_hop_ago = 0;
        return channel_array[scan_last_ch_index];
      }
      // is it time to change the epoch?
      if (scan_last_hop_ago > conf.period) {
        scan_epoch_misses ++;
        scan_last_hop_ago = scan_last_hop_ago % (uint16_t)conf.period;
        scan_last_epoch ++;
      }
      return get_channel_epoch(scan_last_epoch);
    }
    else { // just searching
      // is it time to change the channel?
      if (scan_last_hop_ago > conf.period*1.3) {
        scan_last_hop_ago = 0;
        scan_last_ch_index = (scan_last_ch_index + 1) % 16;
      }
      return channel_array[scan_last_ch_index];
    }
  }
}
#else
#error "Unsupported bootstrap channel hopping setting"
#endif // CRYSTAL_BSTRAP_CHHOPPING
