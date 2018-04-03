#ifndef _LEX_PROTO__H
#include "../defs.h"
#include "token.h"
#include <stdio.h>

#define LEX_STATE_NONE				0
#define LEX_STATE_INCLUDE			3
#define LEX_STATE_PRAGMA			5
#define LEX_STATE_DEFINE			7
#define LEX_STATE_DEFINE_CONST		9
#define LEX_STATE_IF				13
#define LEX_STATE_IF_CONST			14
#define LEX_STATE_ELSE				15
#define LEX_STATE_ENDIF				16
#define LEX_STATE_IFDEF				17
#define LEX_STATE_IFDEF_CONST			18
#define LEX_STATE_INVALID			255

#define LEX_OP_NEG					0x80
#define LEX_OP_EQ					0x50
#define LEX_OP_GT					0x60
#define LEX_OP_LT					0x40
#define LEX_OP_GTEQ					0x66
#define LEX_OP_LTEQ					0x44
#define LEX_OP_OR					0xAA
#define LEX_OP_AND					0xDD

typedef struct pp_const {
	struct pp_const * next;
	uchar name[256];
	uchar value[256];
} pp_const;

typedef struct pp_config {
	uchar filename[256];
	FILE * file;
	uchar * codebuffer;
	int32 length;
	int32 index;
	uchar mode;
	int32 line;
	uchar show_assembly;
} pp_config;

int pp_getc(pp_config * pfig);
void pp_close(pp_config * pfig);
void pp_seek(pp_config * pfig, long offset, int origin);
int pp_error(pp_config * pfig);

extern pp_config * p_config;
extern uchar _lex_state;

void lp_init();

int lp_getdec( char hex );
int lp_hex2dec(char *strin);
char lp_tolower(char c);
uint32 lp_hex2integer(char * strin);
int lp_hex2bytes(char * strin, char * p);
void lp_push_constant(yytoken * token) ;
void lp_escape(uchar * s, uchar * t);
uint16 lp_unescape(uchar * s, uchar * t);


//preprocessor function
pp_config * lp_push_config(pp_config * config);
pp_config * lp_pop_config();
pp_config * lp_peek_config();
uchar lp_push_opcode(uchar opcode);
uchar lp_pop_opcode();
pp_const * lp_push_operand(pp_const * operand);
pp_const * lp_pop_operand();

pp_const * lp_select(uchar * name);
pp_const * lp_insert(uchar * name, uchar * value);
pp_const * lp_update(uchar * name, uchar * value, uint8 length);
void lp_delete(uchar * name);
void lp_clear_globals(void);

void lp_preprocessor_state(uchar state);
void lp_process_token(uchar * token, uint16 length);

void yyinit(void);

#define _LEX_PROTO__H
#endif
