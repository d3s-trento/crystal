#define NO_NODE 0
#define NO_SEQN 65535

typedef uint16_t app_state_t; // global system state

typedef struct {
  app_state_t pin_state;
} 
__attribute__((packed))
app_s_payload;

typedef struct {
  uint8_t pin_id_and_state;
} 
__attribute__((packed))
app_t_payload;

typedef struct {
  app_state_t pin_state;
}
__attribute__((packed))
app_a_payload;
