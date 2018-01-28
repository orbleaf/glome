#ifndef _DEFS_DEFINED
//compiler warning disable
#pragma warning(disable : 4996)
#pragma warning(disable : 4761)
#pragma warning(disable : 4047)
#pragma warning(disable : 4715)
#pragma warning(disable : 4244)
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

//operating system debug configuration
#define _DMA_DEBUG 		0
#define _MIDGARD_DEBUG 	0
#define _IOMAN_DEBUG 	0
//heap analysis for debugging
#if _DMA_DEBUG
#define	HEAP_CALC	 	0	 /* Should be made 0 to turn OFF debugging */
#endif

#define FS_TYPE						'A'
#define FS_UNFORMATTED 				3
#define FS_DESCRIPTION				"Asgard File System - 2010 Agus Purwanto"	//size=40
#define FS_DESCRIPTION_SIZE			40
#define FILE_NOT_FOUND				0xff03
#define INSUFFICIENT_MEMORY			0xff02
#define FILE_CANNOT_BE_CREATED		0xff04
#define FILE_CRC_ERROR				0xff01
#define FS_NO_AVAILABLE_SPACE		0xffff
#define SUCCESS 					1
#define FAIL						0x80
#define CLUSTER_SIZE 				32
#define CRC16_SIZE					2
#define NEXT_POINTER_SIZE			2
#define DATA_SIZE					(CLUSTER_SIZE - (CRC16_SIZE + NEXT_POINTER_SIZE))
#define CRC_SIZE					(DATA_SIZE + CRC16_SIZE)
#define QUEUE_SIZE 					8	//for BFS dan DFS
#define TRUE						1
#define FALSE						0

#define IOMAN_ATR					'\x3B','\x9C','\x94','\x00','\x68','\x86','\x8D','\x0C','\x86','\x98','\x02','\x44','\xC1','\x00','\x05','\x00'
#define IOMAN_ATR_SIZE				16

#define ACCESS_READ					

#define ALLOCATION_TABLE_OFFSET 	4
#define ALLOCATION_TABLE_SIZE 		508
#define ALLOCATION_DATA_OFFSET 		(ALLOCATION_TABLE_OFFSET + ALLOCATION_TABLE_SIZE)

#define EF_NULL						0xFF
#define EF_TRANSPARENT 				0x00
#define EF_LINIER					0x01
#define EF_CYCLIC					0x03
#define EF_EXECUTE					0x10
#define EF_WORKING					0x20
#define EF_INTERNAL					0x40

#define T_RFU						0x00
#define T_MF						0x01
#define T_DF						0x02
#define T_EF						0x04
#define T_CHV						0x08

#define DF_RESPONSE_SIZE			0x17
#define EF_RESPONSE_SIZE			0x0F

#define APDU_SUCCESS				0x9000
#define APDU_MEMORY_PROBLEM			0x9240
#define APDU_NO_EF_SELECTED			0x9400
#define APDU_FILE_NOT_FOUND			0x9404
#define APDU_OUT_OF_RANGE			0x9402
#define APDU_FILE_INCONSISTENT		0x9408
#define APDU_NO_CHV_INIT			0x9802
#define APDU_ACCESS_DENIED			0x9804
#define APDU_INCONTRADICTION_W_CHV	0x9808
#define APDU_INVALID_STATUS			0x9810
#define APDU_CHV_LAST_ATTEMPT		0x9840
#define APDU_MAX_VALUE_REACHED		0x9850
#define APDU_SUCCESS_RESPONSE		0x9F00
#define APDU_COMMAND_INVALID		0x6981
#define APDU_SECURITY_STATE_ERROR	0x6982
#define APDU_AUTH_BLOCKED			0x6983
#define APDU_INCONSISTENT_PARAMETER 0x6A87
#define APDU_DATA_NOT_FOUND			0x6a88
#define APDU_WRONG_PARAMETER		0x6B00
#define APDU_INSTRUCTION_INVALID	0x6D00
#define APDU_CLASS_INVALID			0x6E00
#define APDU_FATAL_ERROR			0x6F00			//no further description
#define APDU_CRC_ERROR				0x6581			//memory failure, eeprom write error
#define APDU_WRONG_LENGTH			0x6700	
#define APDU_FUNCTION_INVALID		0x6A81
#define APDU_NO_AVAILABLE_SPACE		APDU_MAX_VALUE_REACHED		

#define ACC_ALW						0
#define ACC_CHV1					1
#define ACC_CHV2					2
#define ACC_RFU						3
#define ACC_ADM1					4
#define ACC_ADM2					5
#define ACC_ADM3					6
#define ACC_ADM4					7
#define ACC_ADM5					8
#define ACC_ADM6					9
#define ACC_NVR						15

#define CHV_ALWAYS					0x00
#define CHV_ENABLE					1
#define CHV_DISABLE					2
#define CHV_BLOCK					4
#define CHV_UNBLOCK					8
#define CHV_NEVER					0x10
#define CHV_VERIFIED				0x80
#define CHV_PUK_FAILED				0x20
#define CHV_PIN_FAILED				0x40

#define STAT_INVALID				0x00
#define STAT_VALID					0x01

#define CHV_KEY						"gungnir"		//spear of odin
#define CHV_KEY_LENGTH				7				//des key, max = 7

#define FID_MF						0x3F00	//MASTER_FILE
#define FID_ICCID					0x2FE2
#define FID_ICC						0x0002
#define FID_GSM						0x7F20	//DF_GSM
#define FID_IMSI					0x6F07
#define FID_KC						0x6F20
#define FID_LP						0x6F05
#define FID_PLMNSE					0x6F30
#define FID_HPLMN					0x6F31
#define FID_ACMMAX					0x6F37
#define FID_SST						0x6F38
#define FID_ACM						0x6F39
#define FID_PUCT					0x6F41
#define FID_CBMI					0x6F45
#define FID_SUME					0x6F54
#define FID_BCCH					0x6F74
#define FID_ACC						0x6F78
#define FID_FPLMN					0x6F7B
#define FID_LOCI					0x6F7E
#define FID_AD						0x6FAD
#define FID_PHASE					0x6FAE
#define FID_TELECOM					0x7F10	//DF_TELECOM
#define FID_ADN						0x6F3A
#define FID_FDN						0x6F3B
#define FID_LND						0x6F44
#define FID_EXT1					0x6F4A
#define FID_EXT2					0x6F4B
#define FID_CDMA					0x7F25	//DF_CDMA

#define N_CLA						0		//class
#define N_INS						1		//instruction
#define N_P1						2		//parameter 1
#define N_P2						3		//parameter 2
#define N_P3						4		//parameter 3
#define N_DATA						5		//data offset

#define MAX_APDU_LENGTH				0xFF
#define MAX_DATA_LENGTH				(MAX_APDU_LENGTH - 5)
#define INCREASE_RES_LENGTH			6

#define FID_KI						0x2FE1
#define FID_ATR						0x0000

#ifndef NULL
#define NULL						0
#endif

#define OS_DEBUG_ENTRY(x)				//entry
#define OS_DEBUG_EXIT()					//exit

#define _REENTRANT_
#define MACH_LITTLE_ENDIAN
#ifdef MACH_LITTLE_ENDIAN
#define end_swap32(x)		( ((x&0xff000000) >> 24) | ((x&0x00ff0000) >> 8) | ((x&0x0000ff00) << 8) | ((x & 0xff) << 24) )
#define end_swap16(x)		( ((x&0xff00) >> 8) | ((x&0x00ff) << 8) )

#else
#define end_swap32(x)		(x)
#define end_swap16(x)		(x)	

#endif

#define CONST 		const
//#define BYTE		unsigned char
//#define WORD		unsigned 
//#define DWORD		unsigned int
typedef unsigned char BYTE;
typedef unsigned short WORD;

#define vm_memcpy	memcpy
#define vm_memcmp	memcmp
#define vm_memset	memset
#define vm_strlen	strlen
#define mmAllocMemP	malloc
#define mmFreeMem	free

//type definition
typedef long int32;
typedef long eint32;
typedef unsigned long uint32;
typedef unsigned long euint32;
typedef unsigned long ulong;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned short euint16;
typedef unsigned short uint;
typedef unsigned char uchar;
typedef char eint8;
typedef unsigned char euint8;
typedef char int8;
typedef unsigned char uint8;
typedef const unsigned char uint8_t;	//taruh di code program
typedef const unsigned int uint16_t;	//taruh di code program
typedef const unsigned long uint32_t;	//taruh di code program
typedef unsigned long u_ptr;			//typedef cast untuk pointer, diganti menyesuaikan target hardware
//@dir unsigned long * @dir zpage_ptr_to_zero_page;
#ifdef __cplusplus_cli
#define _RECAST(T, x) reinterpret_cast<T>(x)
#else
#define _RECAST(T, x) ((T)x)
#endif
#define _DEFS_DEFINED
#endif
