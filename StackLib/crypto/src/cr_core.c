
#include "../../defs.h" 
#include "../../config.h" 
#include "../inc/cr_apis.h"
#include <string.h>
#ifdef SMT32F2_RNG
#include "stm32f4xx_hal_rng.h"
#endif

static WORD cr_mem_read(void * handle, WORD offset, BYTE * buffer, BYTE len) _REENTRANT_ {
	memcpy(buffer, ((uint8 *)handle) + offset, len);
	return len;
}

static WORD cr_mem_write(void * handle, WORD offset, BYTE * buffer, BYTE len) _REENTRANT_ {
	memcpy(((uint8 *)handle) + offset, buffer, len);
	return len;
}

void cr_init_context(cr_context_p ctx, void * handle) _REENTRANT_ {
	ctx->handle = handle;
	ctx->read = cr_mem_read;
	ctx->write = cr_mem_write;
	ctx->mode = 0;
	ctx->key = NULL;
	memset(ctx->icv, 0, sizeof(ctx->icv));
}

void cr_init_crypt(cr_context_p ctx, BYTE mode, BYTE * src) _REENTRANT_ {
	cr_init_context(ctx, src);
	ctx->mode = mode;
}

void cr_init_crc(cr_context_p ctx, BYTE * src) _REENTRANT_ {
	cr_init_context(ctx, src);
}

void cr_init_mac(cr_context_p ctx, BYTE mode, BYTE * src) _REENTRANT_ {
	cr_init_context(ctx, src);
	ctx->mode = mode;
}

void cr_set_key(cr_context_p ctx, BYTE * key) _REENTRANT_ {
	ctx->key = key;
}

void cr_set_iv(cr_context_p ctx, BYTE * icv) _REENTRANT_ {
	memcpy(ctx->icv, icv, 8);
}

void cr_randomize(uint8 * buffer, uint16 len) {
	uint8 i=0;
	for(i=0;i<len;i++) {
		//buffer[0] = 0x01; //rand();
#ifdef SMT32F2_RNG
		while((RNG->SR & RNG_FLAG_DRDY) == 0);
		//while(RNG_GetFlagStatus(RNG_FLAG_DRDY) == 0);
		buffer[i] = RNG->DR;
		//buffer[i] = RNG_GetRandomNumber();
#endif
	}
}

uint8 cr_calc_lrc(cr_context_p ctx, uint16 offset, uint16 length, uint8 * result) _REENTRANT_ {
	uint8 b = 0;
	uint16 ii;
	uchar jj;
	uchar buffer[16];
	for(ii = 0; ii < length; ii+=16) {
		ctx->read(ctx->handle, offset+ii, buffer, 16);
		for(jj = 0 ;jj < 16 && (ii+jj) < length; jj++) {
			b ^= buffer[jj];
		}
	}
	result[0] = b;
	return 1;
}

void cr_xor(uint8 * mask, uint8 * buffer, uint16 len) {
	uint8 i=0;
	for(i=0;i<len;i++) buffer[i] ^= mask[i];
}