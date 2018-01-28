
#ifdef WIN32
//#include "stdafx.h"
#endif
#include "../../defs.h" 
#include "../inc/cr_apis.h"
#include <string.h>
#include <stdlib.h>

BYTE cr_calc_mac(cr_context_p ctx, WORD offset, WORD length, BYTE * result) _REENTRANT_ {
	BYTE i;
	BYTE plen = 0;
	BYTE k3[8];			//k"
	BYTE k;
	BYTE buffer[8];
	BYTE dbuffer[8];
	WORD end = length + offset;				//calculate end offset, case offset != 0 (fixed:2015.06.20)
	BYTE mode = ctx->mode;
	BYTE * key = ctx->key;
	//static BYTE result[4];
	//WORD blen;				//length in bits
	if((length & 0x07) != 0) plen = 8 - (length & 0x07);
	if(mode & CR_MAC_CUSTOM_IV) {
		memcpy(buffer, ctx->icv, 8);
	} else {
		memset(buffer, 0, 8);						   //precondition iv
	}
	//padding, initializing
	for(i=0;i<8;i++) {
		k = key[i + 8];		  						//load k'
		k3[i] = (k & 0x0F)	| ((~k) & 0xF0);		//calculate k"
	}
	if(mode & 0x02) {		//use padding method 3;
		buffer[5] = (length >> 13);
		buffer[6] = (length >> 5);
		buffer[7] = (length << 3);
	} else {				//read first block
		ctx->read(ctx->handle, offset, dbuffer, 8);
		if(length < 8) {	//case length too small, skip iteration
			goto post_processing;
		}
		memcpy(buffer, dbuffer, 8);
		offset += 8;
	}
	//initial tranformation 1	
	//crEncodeDES(buffer, key);
	cr_des_encode(buffer, key, buffer );
	//pre-processing (I Block)
	pre_processing:
	switch(mode >> 2) {
		case 0x01:
		case 0x02:		
		case 0x03:	   	//no initial processing
			break;	
		case 0x04: 
			//initial tranformation 2	 
			//crEncodeDES(buffer, k3);			//use initialized k"
			cr_des_encode(buffer, k3, buffer );
			break;
	}
	//start step operation
	while((offset + 8) < end) {		 
		ctx->read(ctx->handle, offset, dbuffer, 8);	
		for(i=0;i<8;i++) buffer[i] ^= dbuffer[i];		//XOR operation (CBC)
		//crEncodeDES(buffer, key); 
		cr_des_encode(buffer, key, buffer );
		offset += 8;
	}
	ctx->read(ctx->handle, offset, dbuffer, 8);
	//post-processing (last block, add padding)
	post_processing:
	if(plen != 0) {				//padding operation either padding method 1 or 2
		dbuffer[8-plen] = (mode & 0x01)?0x80:0x00;
		plen--;
		if(plen != 0) memset(dbuffer + (8 -plen), 0, plen);  //fixed:2015.06.19
	}
	for(i=0;i<8;i++) buffer[i] ^= dbuffer[i];		//XOR operation (CBC)
	//start G block process	(output transformation)
	if((mode & CR_MAC_SKIP_G) == 0) {
		//crEncodeDES(buffer, key);
		cr_des_encode(buffer, key, buffer );
		switch(mode >> 2) {
			case 0x01:
				break;
			case 0x02:	
				//crEncodeDES(buffer, k3);
				cr_des_encode(buffer, k3, buffer );
				break;	
			case 0x03:	   	
				//crDecodeDES(buffer, key + 8);
				cr_des_decode(buffer, key + 8, buffer );
				//crEncodeDES(buffer, key);
				cr_des_encode(buffer, key, buffer );
				break;	
			case 0x04: 	
				//crEncodeDES(buffer, key + 8);			//use initialized k"
				cr_des_encode(buffer, key + 8, buffer );
				break;
		}
	}
	//no truncation yet!
	memcpy(result, buffer, 8);
	return 8;
}