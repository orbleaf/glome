
#ifdef WIN32
//#include "stdafx.h"
#endif
#include "../../defs.h"  
#include "../inc/cr_apis.h"
#include <stdlib.h>
#include <string.h>

CONST cr_codec_entry g_fnRegisteredDecoder[] = {
	{ 8, cr_des_decode},
	{ 8, cr_tdes2_decode } ,
	{ 8, cr_tdes3_decode },
	{ 16, cr_aes_decode },
	{ 0, NULL } 
};

CONST cr_codec_entry g_fnRegisteredEncoder[] = {
	{ 8, cr_des_encode },
	{ 8, cr_tdes2_encode },
	{ 8, cr_tdes3_encode },
	{ 16, cr_aes_encode },
	{ 0, NULL }
};

uint16 cr_do_crypt(cr_context_p ctx, WORD offset, WORD length) _REENTRANT_ {
	#define CR_FILE_BUFFER_SIZE 		32
	WORD i, j;
	BYTE k;
	BYTE cbc_buffer[16];	
	BYTE cbc_buffer2[16];
	BYTE buffer[CR_FILE_BUFFER_SIZE];
	BYTE mode = ctx->mode;
	BYTE * key = ctx->key;
	memset(cbc_buffer, 0, sizeof(cbc_buffer)); 			//clear initial value
	for(j=0;j<length;j+=CR_FILE_BUFFER_SIZE) {
		ctx->read(ctx->handle, offset + j, buffer, CR_FILE_BUFFER_SIZE);
		for(k=0;k<CR_FILE_BUFFER_SIZE&&(j+k)<length;k+=g_fnRegisteredEncoder[mode & 0x3F].bsize) {
			//DES/3DES encrypt/decrypt
			if(mode & CR_MODE_ENCRYPT) {
			//START ENCRYPTION
				if(mode & CR_MODE_CBC) {		//use CBC
					for(i=0;i<g_fnRegisteredEncoder[mode & 0x3F].bsize;i++) {
						buffer[k+i] ^= cbc_buffer[i];
					}
				}
				if(g_fnRegisteredEncoder[mode & 0x3F].bsize == 0) return 0;
				if(g_fnRegisteredEncoder[mode & 0x3F].entry != NULL)		 
					g_fnRegisteredEncoder[mode & 0x3F].entry(buffer + k, key, buffer + k);
				if(mode & CR_MODE_CBC) {		//use CBC
					memcpy(cbc_buffer, buffer + k, g_fnRegisteredEncoder[mode & 0x3F].bsize);
				}
			//END ENCRYPTION
			} else {
			//START DECRYPTION
				if(mode & CR_MODE_CBC) {		//use CBC
					memcpy(cbc_buffer2, buffer + k, g_fnRegisteredDecoder[mode & 0x3F].bsize);
				}	
				if(g_fnRegisteredDecoder[mode & 0x3F].bsize == 0) return 0;
				if(g_fnRegisteredDecoder[mode & 0x3F].entry != NULL)
					g_fnRegisteredDecoder[mode & 0x3F].entry(buffer + k, key, buffer + k);
				if(mode & CR_MODE_CBC) {		//use CBC 
					for(i=0;i<g_fnRegisteredDecoder[mode & 0x3F].bsize;i++) {
						buffer[k+i] ^= cbc_buffer[i];
						cbc_buffer[i] = cbc_buffer2[i];
					}
				}
			//END DECRYPTION
			}
		}
		//re-write to handle
		ctx->write(ctx->handle, offset + j, buffer, k);
	}
	return ((j - CR_FILE_BUFFER_SIZE) + k);
}