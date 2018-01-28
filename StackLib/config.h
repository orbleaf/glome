#ifndef _CONFIG__H

/* Midgard Configuration */
#define MIDGARD_ACTIVATED			1				//use midgard
#define MIDGARD_HEAP_SIZE			0x100			//total heap size
#define MIDGARD_LOW_OVERHEAD		1				//4 bytes header
#define MIDGARD_HEAP_ALLOC			0				//use internal heap alloc
#define MIDGARD_32BIT_MODE			0				//32/16 bit mode

/* VM Configuration */
#define VM_CODEDEBUG				0
#define VM_GC_DEBUG					0

/* IL Streamer  Configuration */
#define IS_MAJOR_VERSION			1
#define IS_MINOR_VERSION			4
#define IS_CODEDEBUG				1
#define IS_REL_JUMP_ADDRESS			1

//#define STANDALONE_COMPILER			1

//#define SP_MAX_VARS					4
//#define IL_MAX_CODESIZE				0x200
#define STACK_CONVERTER_APIS 		1
#define STACK_CRYPTO_APIS 			1
#define STACK_BIT_APIS				0
#define STACK_VAR_APIS				0
#define _CONFIG__H
#endif
