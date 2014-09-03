#include "GOST_34112012_int_32b.h"

struct GOST_34112012_content
{
	uint32_t h[WORDS];
	uint32_t N[WORDS];
	uint32_t sum[WORDS];
	uint32_t m[WORDS];
	uint32_t msg_len;
};

void GOST_Start(uint8_t * msg, uint32_t msg_len, uint8_t * out)
{
	GOST_34_11_2012_content GOST_state;

	GOST_state.msg_len = msg_len;

	init_gost_state(&GOST_state);
	comp_hash(&GOST_state, msg, out);
}

void init_gost_state(GOST_34_11_2012_content * GOST_state)
{
	memset(GOST_state->h, 0, WORDS_IN_BYTES);
	memset(GOST_state->N, 0, WORDS_IN_BYTES);
	memset(GOST_state->sum, 0, WORDS_IN_BYTES);
	memset(GOST_state->m, 0, WORDS_IN_BYTES);
}

void copy(uint8_t * dest, uint8_t * src)
{
  uint8_t i, j;
  
  for (i = 0; i < WORDS; i++)
  {
    for (j = 0; j < 4; j++)
    {
      dest[i*4 + j] = src[i*4 + 3 - j];
    }
  }
}

void copy_same(uint8_t * dest)
{
  uint8_t i, j, temp;
  
  for (i = 0; i < WORDS; i++)
  {
    for (j = 0; j < 2; j++)
    {
      temp = dest[i*4 + j];
      dest[i*4 + j] = dest[i*4 + 3 - j];
      dest[i*4 + 3 - j] = temp;
    }
  }
}

void comp_hash(GOST_34_11_2012_content * GOST_state, uint8_t * msg, uint8_t * out)
{
	while (GOST_state->msg_len >= 64)
	{
                copy((uint8_t *)&GOST_state->m, msg+8);
		
		g_N(GOST_state->N, GOST_state->h, GOST_state->m);

		add_N_mod512(GOST_state->N, 512);

		add_sum_mod512(GOST_state->sum, GOST_state->m);

		GOST_state->msg_len -= 64;
	}

	memcpy( (uint8_t *)(GOST_state->m) + (64 - GOST_state->msg_len), msg, GOST_state->msg_len);
	memset(GOST_state->m, 0, 64 - GOST_state->msg_len - 1);
	((uint8_t *)GOST_state->m)[64 - GOST_state->msg_len - 1] = 0x01;
        copy_same((uint8_t *)GOST_state->m);
        
	g_N(GOST_state->N, GOST_state->h, GOST_state->m);
	add_N_mod512(GOST_state->N, GOST_state->msg_len*8);
	add_sum_mod512(GOST_state->sum, GOST_state->m);

	memset(GOST_state->m, 0, WORDS_IN_BYTES);
	g_N(GOST_state->m, GOST_state->h, GOST_state->N);
	g_N(GOST_state->m, GOST_state->h, GOST_state->sum);
        
	memcpy(out, GOST_state->h, WORDS_IN_BYTES);
        copy_same(out);
}

void g_N(uint32_t * N, uint32_t * h, uint32_t * m)
{
	uint8_t i;
	uint32_t K[WORDS];

	for (i = 0; i < WORDS; i++)
	{
		K[i] = h[i] ^ N[i];
	}

	LPS((uint8_t *)&K);

	E(K, m);

	for (i = 0; i < WORDS; i++)
	{
		h[i] ^= K[i] ^ m[i];
	}
}

void E(uint32_t * K, uint32_t * m)
{
	uint8_t i, kkk, j;
	uint32_t K_internal[WORDS];
        memset(K_internal, 0, WORDS_IN_BYTES);
	memcpy((uint8_t *)&K_internal, (uint8_t *)K, WORDS_IN_BYTES);

	for (i = 0; i < WORDS; i++)
	{
		K[i] ^= m[i];
	}

	for (kkk = 0; kkk < 12; kkk++)
	{
		LPS((uint8_t *)K);
                
		// Key(K_internal, kkk);
                for (j = 0; j < WORDS; j++)
                {
                  K_internal[j] ^= C[kkk][j];
                }

                LPS((uint8_t *)&K_internal);
                // /////////////////////
                
		for (i = 0; i < WORDS; i++)
		{
			K[i] ^= K_internal[i];
		}
	}
}

void LPS(uint8_t * state)
{
        uint32_t acum[WORDS];
        uint64_t temp;
        int32_t i, j, m, k;
        
        for (i = 3, m = 0; i >= 0; i--, m += 2)
        {
            temp = get_A(Pi[state[i]], 0);
            acum[m] = (temp & 0xFFFFFFFF00000000) >> 32;
            acum[m+1] = temp & 0x00000000FFFFFFFF;
            for (j = i + 8, k = 1; j < 60; j +=8, k++)
            {
                temp = get_A(Pi[state[j]], k);
                acum[m] ^= (temp & 0xFFFFFFFF00000000) >> 32;
                acum[m+1] ^= temp & 0x00000000FFFFFFFF;
            }
        }

        for (i = 7, m = 8; i >= 4; i--, m += 2)
        {
            temp = get_A(Pi[state[i]], 0);
            acum[m] = (temp & 0xFFFFFFFF00000000) >> 32;
            acum[m+1] = temp & 0x00000000FFFFFFFF;
            for (j = i + 8, k = 1; j < 64; j +=8, k++)
            {
                temp = get_A(Pi[state[j]], k);
                acum[m] ^= (temp & 0xFFFFFFFF00000000) >> 32;
                acum[m+1] ^= temp & 0x00000000FFFFFFFF;
            }
        }
        
        memcpy((uint32_t *)state, acum, WORDS_IN_BYTES);
}

void add_N_mod512(uint32_t * dest, uint32_t n)
{
	int8_t i;
	uint64_t temp;

	temp = (uint64_t)dest[15] + (uint64_t)n;
	dest[15] = temp & 0x00000000FFFFFFFF;
	temp >>= 32;

	for (i = 14; i >= 0; i--)
	{
		temp += (uint64_t)dest[i];
		dest[i] = temp & 0x00000000FFFFFFFF;
		temp >>= 32;
	}
}

void add_sum_mod512(uint32_t * dest, uint32_t * m)
{
	int8_t i;
	uint64_t temp;

	temp = 0;
	for (i = 15; i >= 0; i--)
	{
		temp += (uint64_t)dest[i] + (uint64_t)m[i];
		dest[i] = temp & 0x00000000FFFFFFFF;
		temp >>= 32;
	}
}

uint64_t get_A(uint8_t value, uint8_t position)
{
    uint32_t v[2];
    uint8_t k;
    uint8_t mask;

    v[0] = 0;
    v[1] = 0;

    mask = 0x80;
    for (k = 0; k < 8; k++)
    {
        if ( (value & mask) != 0)
        {
            v[0] ^= A[position*16 + k*2];
            v[1] ^= A[position*16 + k*2 + 1];
        }

        mask >>= 1;
    }

    return ((uint64_t)v[0] << 32) | v[1];
}