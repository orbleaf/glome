
#ifdef WIN32
//#include "stdafx.h"
#endif
#include "../../defs.h"  
#include "../inc/cr_apis.h"
#include <stdlib.h>
#include <string.h>

static uint32 lGenCRC32(uint32 lOldCRC, uchar ByteVal) _REENTRANT_ {
	uint32 TabVal;
	uchar j;
	TabVal = ((lOldCRC) ^ ByteVal) & 0xFF;
	for(j=8; j>0; j--) {
		if(TabVal & 1) {
		 	TabVal = (TabVal >> 1) ^ 0xEDB88320;
		} else {
			TabVal >>= 1;
		}
	}
	return TabVal ^ ((lOldCRC >> 8) & 0x00FFFFFF);
}

uint32 cr_calc_crc(cr_context_p ctx, uint16 offset, uint16 length, uint8 * result) _REENTRANT_ {
	uint32 crc32;
	uint16 ii;
	uchar jj;
	uchar buffer[16];
	crc32 = 0xFFFFFFFF;		//pre-conditioning
	for(ii = 0; ii < length; ii+=16) {
		ctx->read(ctx->handle, offset+ii, buffer, 16);
		for(jj = 0 ;jj < 16 && (ii+jj) < length; jj++) {
			crc32 = lGenCRC32(crc32, buffer[jj]);
		}
	}
	crc32 = ~crc32;				//post-conditioning
	crc32 = end_swap32(crc32);
	memcpy(result, &crc32, 4);
	return crc32;
}

#if CRC32_PROCESS_FILE
uint32 FileCRC32(fs_handle * handle, uint16 length, uint16 offset) _REENTRANT_ {
	uint32 crc32;
	uint16 ii;
	uchar jj;
	uchar buffer[16];
	crc32 = 0xFFFFFFFF;		//pre-conditioning
	for(ii = 0; ii < length; ii+=16) {
		_readbin(handle, offset+ii, buffer, 16);
		for(jj = 0 ;jj < 16 && (ii+jj) < length; jj++) {
			crc32 = lGenCRC32(crc32, buffer[jj]);
		}
	}
	crc32 = ~crc32;				//post-conditioning
	return crc32;
}
#endif

 