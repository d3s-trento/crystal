#define NO_NODE 0
#define NO_SEQN 65535

typedef struct {
} 
__attribute__((packed))
app_s_payload;

typedef struct {
  crystal_addr_t src;
  uint16_t seqn;
  uint8_t payload[CRYSTAL_PAYLOAD_LENGTH];
} 
__attribute__((packed))
app_t_payload;

typedef struct {
  crystal_addr_t src;
  uint16_t seqn;
} 
__attribute__((packed))
app_a_payload;


#define APP_SPLD_LEN sizeof(app_s_payload)
#define APP_TPLD_LEN sizeof(app_t_payload)
#define APP_APLD_LEN sizeof(app_a_payload)
