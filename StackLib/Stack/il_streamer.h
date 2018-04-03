#include "../defs.h"
#ifndef _IL_STREAMER__H

#define MAX_BUFFER_SIZE		65536

#define INS_NOP			0	//-> no operation 		(pseudo instruction)
#define INS_SCTX		8	//-> save context for variables (function call)
#define INS_RCTX		9	//-> restore context memory (function return)
#define INS_OBJCONST	16	//-> load constant to stack
#define INS_OBJNEW		17	//-> allocate new instance
#define INS_OBJDUP		18	//-> duplicate stack
#define INS_OBJDEL		19	//-> delete (relative to sp, remove current object from stack)
#define INS_OBJSZ		22	//-> size of object on stack
#define INS_OBJSUB		23	//-> explode object (relative to sp)

#define INS_OBJPUSH		25	//-> push variable to stack		
#define INS_OBJPOP		26	//-> pop variable from stack
#define INS_OBJSTORE		29	//-> store variable from stack

//stack operation
#define INS_ADD		32	//-> addition (relative to sp)
#define INS_SUB		33	//-> subtract (relative to sp)
#define INS_MUL		34	//-> multiplication
#define INS_DIV		35	//-> division
#define INS_MOD		36	//-> modulus

#define INS_END		60	//-> end function			(pseudo instruction)
#define INS_FUNC	61	//-> create function		(pseudo instruction)
#define INS_LBL		62	//-> create label			(pseudo instruction)
#define INS_CREQ		64	//-> jump if equal (relative to pc)
#define INS_CRNE			65	//-> jump not equal (relative to pc)
#define INS_CRGT			66	//-> jump greater than (relative to pc)
#define INS_CRLT			67	//-> jump less than (relative to pc)
#define INS_CRGTEQ			68	//-> jump greater than (relative to pc)
#define INS_CRLTEQ			69	//-> jump less than (relative to pc)
#define INS_JMP				70	//-> jump unconditional (relative to pc)
#define INS_JFALSE			71	//-> jump if false
#define INS_JTRUE			72	//-> jump if true
#define INS_SWITCH			74	//-> switch

#define INS_RET		126	//-> finished operation
#define INS_CALL		127
#define INS_SYSCALL		128
#define INS_SYSCALL0	128
#define INS_SYSCALL1	129
#define INS_SYSCALL2	130
#define INS_SYSCALL3	131
#define INS_SYSCALL4	132
#define INS_SYSCALL5	133
#define INS_SYSCALL6	134
#define INS_SYSCALL7	135
#define INS_SYSCALL8	136
#define INS_SYSCALL9	137
#define INS_SYSCALL10	138
#define INS_SYSCALL11	139
#define INS_SYSCALL12	140
#define INS_SYSCALL13	141
#define INS_SYSCALL14	142
#define INS_SYSCALL15	143

#define INS_EXTCALL		160
#define INS_EXTCALL0	160
#define INS_EXTCALL1	161
#define INS_EXTCALL2	162
#define INS_EXTCALL3	163
#define INS_EXTCALL4	164
#define INS_EXTCALL5	165
#define INS_EXTCALL6	166
#define INS_EXTCALL7	167
#define INS_EXTCALL8	168
#define INS_EXTCALL9	169
#define INS_EXTCALL10	170
#define INS_EXTCALL11	171
#define INS_EXTCALL12	172
#define INS_EXTCALL13	173
#define INS_EXTCALL14	174
#define INS_EXTCALL15	175

#define INS_F2O			96	//-> offset to object
#define INS_F2O_0		96	//-> offset to object
#define INS_F2O_1		97	//-> offset to object
#define INS_F2O_2		98	//-> offset to object
#define INS_F2O_3		99	//-> offset to object
#define INS_F2O_4		100	//-> offset to object
#define INS_F2O_5		101	//-> offset to object
#define INS_F2O_6		102	//-> offset to object
#define INS_F2O_7		103	//-> offset to object
#define INS_F2O_8		104	//-> offset to object
#define INS_F2O_9		105	//-> offset to object
#define INS_F2O_10		106	//-> offset to object
#define INS_F2O_11		107	//-> offset to object
#define INS_F2O_12		108	//-> offset to object
#define INS_F2O_13		109	//-> offset to object
#define INS_F2O_14		110	//-> offset to object
#define INS_F2O_15		111	//-> offset to object
//.db	-> data byte
//.dw	-> data word

//APIs:
//disptext	-> display text
//getinput	-> get input
//sendsm		-> send short message
//sendussd	-> send ussd
//getlocal	-> local info
//showlist 	-> select item
#define IS_OPTIMIZE_L0		0
#define IS_OPTIMIZE_L1		1
#define IS_OPTIMIZE_L2		2
#define IS_OPTIMIZE_L3		4

typedef struct optrec {
	struct optrec * next;			//internal use
	uchar ins;						//instruction
	uint16 index;					//
	uint16 offset;					//original offset
	uint16 opt_offset;				//optimized offset
} optrec;

typedef struct coderec {
	struct coderec * next;
	uint16 offset;					//original offset
	uint16 rdc;						//code reduction, opt_offset = original_offset - code_reduction
} coderec;

typedef struct lblrec {
	struct lblrec * next;
	uint32 offset;
	uint16 refcount;
	void * tag;
	uchar name[240];
} lblrec;

extern uchar _istream_code_buffer[MAX_BUFFER_SIZE];
extern uchar _istream_data_buffer[MAX_BUFFER_SIZE];

void is_init(uchar * inpath);
uint32 is_push_constant(uchar size, uchar * value) ;
uint32 is_gencode(uint32 offset, uchar opcode, ...);
void is_link_optimize(uchar mode);
void is_file_flush();

//label record (debug information)
uint16 is_push_lblrec(uint16 offset, uchar * name);
lblrec * is_get_lblrec(uint16 offset);
void is_clear_unused_lblrec(void);
void is_clear_lblrec(void);

#define _IL_STREAMER__H
#endif
