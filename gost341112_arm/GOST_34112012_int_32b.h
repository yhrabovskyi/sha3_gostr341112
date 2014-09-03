#include <stdint.h>
#include <string.h>

#define WORDS 16
#define WORDS_IN_BYTES 64

typedef struct GOST_34112012_content GOST_34_11_2012_content;

extern const uint32_t A[128];
extern const uint32_t C[12][16];
extern const uint8_t tau[64];
extern const uint8_t Pi[256];

void copy(uint8_t * dest, uint8_t * src);

void init_gost_state(GOST_34_11_2012_content * GOST_state);
void comp_hash(GOST_34_11_2012_content * GOST_state, uint8_t * msg, uint8_t * out);
void add_N_mod512(uint32_t * dest, uint32_t n);
void add_sum_mod512(uint32_t * dest, uint32_t * m);
void g_N(uint32_t * N, uint32_t * h, uint32_t * m);
void E(uint32_t * K, uint32_t * m);
void LPS(uint8_t * state);
uint64_t get_A(uint8_t value, uint8_t position);