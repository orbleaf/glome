#ifndef CR_APIS__H
#define CR_APIS__H
#include "../../defs.h"
//added pandora key generator, fixed (2016.04.01)
//fixed block size variable during crDoCrypt when processing >64 bit block cipher such as AES (2016.05.11)

#define CR_MODE_DES		0		//default DES  
#define CR_MODE_DES2		1		//default DES
#define CR_MODE_DES3		2		//default DES
#define CR_MODE_AES		3		//default DES
#define CR_MODE_CBC		0x40		//default ECB
#define CR_MODE_ENCRYPT	0x80		//default decrypt
#define CR_MODE_DECRYPT	0
#define CR_MODE_ECB		0

#define CR_MAC_PAD0			0x00
#define CR_MAC_PAD1			0x00
#define CR_MAC_PAD2			0x01
#define CR_MAC_PAD3			0x02

#define CR_MAC_ALGO1		(1 << 2)
#define CR_MAC_ALGO2		(2 << 2)
#define CR_MAC_ALGO3		(3 << 2)
#define CR_MAC_ALGO4		(4 << 2)
#define CR_MAC_ALGO5		(5 << 2)
#define CR_MAC_ALGO6		(6 << 2)

#define CR_ICV_SIZE			16

#define CR_MAC_SKIP_G		0x80	 	//skip G block
#define CR_MAC_CUSTOM_IV	0x40		//use custom IV

struct cr_context {
  	void * handle;
	WORD (* read)(void * handle, WORD offset, BYTE * buffer, BYTE len) ;
	WORD (* write)(void * handle, WORD offset, BYTE * buffer, BYTE len) ;
	BYTE icv[16];		//initialization value
	BYTE mode; 			//see defined
	BYTE * key;			//if any
};			

struct cr_codec_entry {	
	uint8 bsize;
	void (* entry)(BYTE *, BYTE *, BYTE *) _REENTRANT_ ;	  	/*< entry point to trigger function */
};

typedef struct cr_context cr_context;
typedef struct cr_context * cr_context_p;
typedef struct cr_codec_entry cr_codec_entry;

void cr_randomize(BYTE * buffer, WORD len);
void cr_xor(BYTE * mask, BYTE * buffer, WORD len);

void cr_init_context(cr_context_p ctx, void * handle) _REENTRANT_ ;
void cr_init_crypt(cr_context_p ctx, BYTE mode, BYTE * src) _REENTRANT_ ;
void cr_init_crc(cr_context_p ctx, BYTE * src) _REENTRANT_ ;
void cr_init_mac(cr_context_p ctx, BYTE mode, BYTE * src) _REENTRANT_ ;
void cr_set_key(cr_context_p ctx, BYTE * key) _REENTRANT_ ;
void cr_set_iv(cr_context_p ctx, BYTE * iv) _REENTRANT_ ;
//implementasi algoritma enkripsi triple DES
void cr_des_encode(BYTE *dst, BYTE *key, BYTE *src );
void cr_des_decode(BYTE *dst, BYTE *key, BYTE *src );
void cr_tdes2_encode(BYTE *dst, BYTE *key, BYTE *src );
void cr_tdes2_decode(BYTE *dst, BYTE *key, BYTE *src );
void cr_tdes3_encode(BYTE *dst, BYTE *key, BYTE *src );
void cr_tdes3_decode(BYTE *dst, BYTE *key, BYTE *src );
void cr_aes_encode(BYTE* dst, BYTE* key, BYTE * src) _REENTRANT_ ;
void cr_aes_decode(BYTE* dst, BYTE* key, BYTE * src) _REENTRANT_;

uint16 cr_do_crypt(cr_context_p ctx, WORD offset, WORD length) _REENTRANT_ ; 
uint32 cr_calc_crc(cr_context_p ctx, uint16 offset, uint16 length, uint8 * result) _REENTRANT_ ;
BYTE cr_calc_mac(cr_context_p ctx, WORD offset, WORD length, BYTE * result) _REENTRANT_ ;
uint8 cr_calc_sha1(cr_context_p ctx, uint16 offset, uint16 length, uint8 * result) _REENTRANT_ ;
uint8 cr_calc_md5(cr_context_p ctx, uint16 offset, uint16 length, uint8 * result) _REENTRANT_;
uint8 cr_calc_sha256(cr_context_p ctx, uint16 offset, uint16 length, uint8 * result) _REENTRANT_;
uint8 cr_calc_lrc(cr_context_p ctx, uint16 offset, uint16 length, uint8 * result) _REENTRANT_ ;

//pandora APIs (key generator)
uint16 cr_gen_lrc(char * key, uint8 length) _REENTRANT_ ;
uint16 cr_finalize_key(char * key, uint8 length, uint8 seed) _REENTRANT_ ;
uint8 cr_calculate_key(char * key, uint8 length, uint16 lrc) _REENTRANT_ ;
uint8 cr_generate_key(char * random, char * key, uint8 length) _REENTRANT_;
void cr_init_pandora() _REENTRANT_ ;

#endif