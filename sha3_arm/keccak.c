#include "keccakH.h"

/* //////////////////////////////////////////////////////////////////////////////////////////////// */
/* Із байтів у слова (смуги; змінення послідовності бітів)                                          */
void fromBytesToWords(uint8_t * msgBlock)
{
  uint8_t count;
  uint32_t i;
  uint32_t low, high;
  uint32_t temp1, temp2;
  
  count = 0;
  
  for (i = 0; i < RATE_IN_LANES; i++)
  {
    low = 0;
    low |= (uint32_t)msgBlock[count++];
    low |= (uint32_t)msgBlock[count++] << 8;
    low |= (uint32_t)msgBlock[count++] << 16;
    low |= (uint32_t)msgBlock[count++] << 24;
    
    high = 0;
    high |= (uint32_t)msgBlock[count++];
    high |= (uint32_t)msgBlock[count++] << 8;
    high |= (uint32_t)msgBlock[count++] << 16;
    high |= (uint32_t)msgBlock[count++] << 24;
    
    // Low, parity
    temp1 = low & 0x55555555;
    temp2 = temp1;
    temp1 = (temp1 >> 1) | temp2;
    
    temp1 &= 0x33333333;
    temp2 = temp1;
    temp1 = (temp1 >> 2) | temp2;
    
    temp1 &= 0x0F0F0F0F;
    temp2 = temp1;
    temp1 = (temp1 >> 4) | temp2;
    
    temp1 &= 0x00FF00FF;
    temp2 = temp1;
    state[2*i+0] ^= (temp1 >> 8) | (temp2 & 0x000000FF);
    
    // High, parity
    temp1 = high & 0x55555555;
    temp2 = temp1;
    temp1 = (temp1 >> 1) | temp2;
    
    temp1 &= 0x33333333;
    temp2 = temp1;
    temp1 = (temp1 >> 2) | temp2;
    
    temp1 &= 0x0F0F0F0F;
    temp2 = temp1;
    temp1 = (temp1 >> 4) | temp2;
    
    temp1 &= 0x00FF00FF;
    temp2 = temp1;
    state[2*i+0] ^= ( (temp1 >> 8) | (temp2 & 0x000000FF) ) << 16;
    
    // Low, odd
    temp1 = low & 0xAAAAAAAA;
    temp2 = temp1;
    temp1 = (temp1 << 1) | temp2;
    
    temp1 &= 0xCCCCCCCC;
    temp2 = temp1;
    temp1 = (temp1 << 2) | temp2;
    
    temp1 &= 0xF0F0F0F0;
    temp2 = temp1;
    temp1 = (temp1 << 4) | temp2;
    
    temp1 &= 0xFF00FF00;
    temp2 = temp1;
    state[2*i+1] ^= ( (temp1 << 8) | (temp2 & 0xFF000000) ) >> 16;
    
    // High, odd
    temp1 = high & 0xAAAAAAAA;
    temp2 = temp1;
    temp1 = (temp1 << 1) | temp2;
    
    temp1 &= 0xCCCCCCCC;
    temp2 = temp1;
    temp1 = (temp1 << 2) | temp2;
    
    temp1 &= 0xF0F0F0F0;
    temp2 = temp1;
    temp1 = (temp1 << 4) | temp2;
    
    temp1 &= 0xFF00FF00;
    temp2 = temp1;
    state[2*i+1] ^= (temp1 << 8) | (temp2 & 0xFF000000);
  }
}
/* //////////////////////////////////////////////////////////////////////////////////////////////// */



/* //////////////////////////////////////////////////////////////////////////////////////////////// */
/* Зі слів (смуг) у байти (змінення послідовності бітів)                                            */
void fromWordsToBytes(uint8_t * msgBlock)
{
  uint8_t i;
  uint8_t count;
  uint32_t low, high, temp;
  
  count = 0;
  
  for(i = 0; i < CAPACITY_IN_LANES/2; i++)
  {
    low = state[2*i+0];
    high = state[2*i+1];
    
    temp = low;
    low = (low & 0x0000FFFF) | (high << 16);
    high = (temp >> 16) | (high & 0xFFFF0000);
    
    temp = (low ^ (low >> 8)) & 0x0000FF00;
    low = low ^ temp ^ (temp << 8);
    temp = (low ^ (low >> 4)) & 0x00F000F0;
    low = low ^ temp ^ (temp << 4);
    temp = (low ^ (low >> 2)) & 0x0C0C0C0C;
    low = low ^ temp ^ (temp << 2);
    temp = (low ^ (low >>  1)) & 0x22222222;
    low = low ^ temp ^ (temp << 1);
    
    temp = (high ^ (high >> 8)) & 0x0000FF00;
    high = high ^ temp ^ (temp << 8);
    temp = (high ^ (high >> 4)) & 0x00F000F0;
    high = high ^ temp ^ (temp << 4);
    temp = (high ^ (high >> 2)) & 0x0C0C0C0C;
    high = high ^ temp ^ (temp << 2);
    temp = (high ^ (high >> 1)) & 0x22222222;
    high = high ^ temp ^ (temp << 1);
    
    msgBlock[count++] = ( low & 0x000000FF );
    msgBlock[count++] = ( low & 0x0000FF00 ) >> 8;
    msgBlock[count++] = ( low & 0x00FF0000 ) >> 16;
    msgBlock[count++] = ( low & 0xFF000000 ) >> 24;
    
    msgBlock[count++] = ( high & 0x000000FF );
    msgBlock[count++] = ( high & 0x0000FF00 ) >> 8;
    msgBlock[count++] = ( high & 0x00FF0000 ) >> 16;
    msgBlock[count++] = ( high & 0xFF000000 ) >> 24;
  }
}
/* //////////////////////////////////////////////////////////////////////////////////////////////// */

/* //////////////////////////////////////////////////////////////////////////////////////////////// */
/* Доповнення останнього блоку                                                                      */
void padding(uint8_t tempLen)
{
	/* Якщо лишається рівно-більше два вільні байти								*/
	if ( (tempLen <= (RATE_IN_BYTES-2)) && (tempLen != 0) )
	{
		msgBlock[tempLen++] = 0x01;					/* Додаємо 0х01			*/
		for ( ; tempLen<(RATE_IN_BYTES-1); tempLen++)			        /* Додаємо потрібно к-сть нулів	*/
		{
			msgBlock[tempLen] = 0x00;
		}
		msgBlock[tempLen] = 0x80;					/* Додаємо останню 0х08		*/
	}
	/* В іншому випадку											*/
	else
	{
		/* Якщо залишається один вільний байт   							*/
		if (tempLen == (RATE_IN_BYTES-1))
		{
			msgBlock[tempLen] = 0x81;				/* Додаємо 0х01			*/
		}
		/* В іншому випадку (немає вільних байтів, наступний блок: 0х01,0х00,...,0х80)			*/
		else
		{
			tempLen = 0;
			msgBlock[tempLen++] = 0x01;				/* Додаємо 0х01			*/
			for ( ; tempLen<(RATE_IN_BYTES-1); tempLen++)		/* Додаємо потрібно к-сть нулів	*/
			{
				msgBlock[tempLen] = 0x00;
			}
			msgBlock[tempLen] = 0x80;				/* Додаємо останню 0х08 	*/
		}
	}
}
/* //////////////////////////////////////////////////////////////////////////////////////////////// */

/* //////////////////////////////////////////////////////////////////////////////////////////////// */
/* Ініціалізація стану                                                                              */
void stateInit (void)
{
	uint8_t i;

	for (i = 0; i < LANES*DIV; i++)
		state[i] = 0;
}
/* //////////////////////////////////////////////////////////////////////////////////////////////// */