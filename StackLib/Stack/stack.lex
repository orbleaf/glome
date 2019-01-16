%{
/*=========================================================================
C-libraries and Token definitions
=========================================================================*/
#include <string.h> /* for strdup */
/*#include <stdlib.h> */ /* for atoi */
#include "token.h"
#include "stack_tab.h" /* for token definitions and yylval */
#include "lex_proto.h"
#include "sym_table.h"
#include "asm_streamer.h"

#define YY_INPUT(buf,result,max_size) { \
	int c = '*', n, d; \
	for ( n = 0; n < max_size && \
		     (c = pp_getc( p_config )) != EOF && c != '\n'; ++n ) \
		buf[n] = (char)(c); \
	if ( c == '\n' ) { \
        while((d = pp_getc( p_config )) == '\r' || d == '\n') { if( d=='\n' ) p_config->line++; }\
        if( d == EOF) {\
            pp_close( p_config);\
		    p_config = lp_pop_config();\
        } else {\
            pp_seek( p_config, -1, SEEK_CUR);\
		    buf[n++] = (char) c; \
        }\
	} \
	if ( c == EOF && pp_error(  p_config ) ) {\
		/*printf( "input in flex scanner failed\n" );*/ \
	}\
	if(p_config->show_assembly) {\
		as_info(_RECAST(uchar *, buf));	\
	}\
	if ( c == EOF) {\
		pp_close( p_config);\
		p_config = lp_pop_config();\
	}\
	result = n; \
	/*buf[n]=0;\
	printf("%s\n", buf);*/\
	if(result<=0) result=YY_NULL; \
}


%}
/*=========================================================================
TOKEN Definitions
=========================================================================*/
%x BLOCK_COMMENT_SECTION
%x LINE_COMMENT_SECTION
%x PREPROCESSOR_SECTION
%x ASSEMBLER_SECTION
LETTER ([ !#-ÿ]|\\.|[^\\"])*
ALPHABET [_A-Za-z0-9]*
HEX [A-Fa-f0-9]*
NUMBER (([0-9]*)(\.[0-9]*)?)|(\.?[0-9]*)
/*NUMBER [0-9]*  */
/*=========================================================================
REGULAR EXPRESSIONS defining the tokens for the Simple language
=========================================================================*/
%%     
<ASSEMBLER_SECTION>{
	"#"			{ BEGIN(PREPROCESSOR_SECTION); }
	";"			{ BEGIN(LINE_COMMENT_SECTION); } 
	[ \t\r]+ 	{ /* eat up whitespace */ } 
	\n 			{ p_config->line++; }
	const
	new
	dup
	store
	push
	pop
	add
	sub
	mul
	div
	creq
	crne
	crlt
	crgt
	crlteq
	crgteq
	jfalse
	jmp
	.			{ /* eat up whitespace */ }
}

<BLOCK_COMMENT_SECTION>{
     "*/"      { BEGIN(INITIAL); }
     [^*\n]+   { /* eat comment in chunks */ }
     "*"       { /* eat the lone star */ }
     \n        { p_config->line++; }
}

<LINE_COMMENT_SECTION>{
     \n      { p_config->line++; BEGIN(INITIAL); }
     [^*\n]+   // eat comment in chunks
     "*"       // eat the lone star
}

<PREPROCESSOR_SECTION>{
	\n			{ p_config->line++; lp_flush_token(); if(_lex_state != LEX_STATE_INVALID) BEGIN(INITIAL); }
	\/\/		{ lp_flush_token(); if(_lex_state != LEX_STATE_INVALID) BEGIN(LINE_COMMENT_SECTION); }
	[ \t\r]+ 	{ /* eat up whitespace */ }
	include		{ /*lp_preprocessor_state(LEX_STATE_INCLUDE);*/ }
	pragma		{ lp_preprocessor_state(LEX_STATE_PRAGMA); 	}
	define		{ lp_preprocessor_state(LEX_STATE_DEFINE);  }
	ifdef		{ lp_preprocessor_state(LEX_STATE_IFDEF); 	}
	if			{ lp_preprocessor_state(LEX_STATE_IF); 		}
	else		{ lp_preprocessor_state(LEX_STATE_ELSE); 	}
	endif		{ lp_preprocessor_state(LEX_STATE_ENDIF); 	}
	asm			{ BEGIN(ASSEMBLER_SECTION); }
	endasm		{ BEGIN(INITIAL); }
	\"{LETTER}\" { 
		uchar buffer[4096];
		uint16 length;
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, yytext + 1, strlen(yytext) - 2);
		length = lp_unescape(buffer, buffer);
		lp_process_token(buffer, length);
	}
	\<{LETTER}\> { 
		uchar buffer[4096];
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, yytext + 1, strlen(yytext) - 2);
		lp_process_token(buffer, strlen((const char *)buffer));
	}
	{NUMBER} {
		lp_process_token(_RECAST(uchar *, yytext), strlen(_RECAST(const char *, yytext)));
	}
	0x{HEX} { 				
		char buffer[1024];	
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%i", lp_hex2integer(yytext + 2));
		lp_process_token((uchar *)buffer, strlen((const char *)buffer));
	}
	{ALPHABET} {
		lp_process_token(_RECAST(uchar *, yytext), strlen(yytext));
	}
	"=="		{ lp_push_opcode(LEX_OP_EQ); }
	"!"			{ lp_push_opcode(LEX_OP_NEG); }
	"<"			{ lp_push_opcode(LEX_OP_LT); }
	">"			{ lp_push_opcode(LEX_OP_GT); }
	"<="		{ lp_push_opcode(LEX_OP_LTEQ); }
	">="		{ lp_push_opcode(LEX_OP_GTEQ); }
	"||"		{ lp_push_opcode(LEX_OP_OR); }
	"&&"		{ lp_push_opcode(LEX_OP_AND); }
	"("			{ /* ignore */ }
	")"			{ /* ignore */ }
	.			{ /* eat up whitespace */ }
}

[ \t\r]+ 	{ /* eat up whitespace */ } 
\n 			{ p_config->line++; }
"\xef\xbb\xbf"	{ /* eat up byte order mark */ }

"/*"        { BEGIN(BLOCK_COMMENT_SECTION); }
"#"			{ BEGIN(PREPROCESSOR_SECTION); }

"+="		{ return S_ADDEQ; }
"-="		{ return S_SUBEQ; }
"++"		{ return S_ADDADD; }
"--"		{ return S_SUBSUB; }
"*="		{ return S_MULEQ; }
"/="		{ return S_DIVEQ; }
"%="		{ return S_MODEQ; }
"&="		{ return S_ANDEQ; }
"|="		{ return S_OREQ; }
"^="		{ return S_XOREQ; }

"=="		{ return T_EQ; }
"&&"		{ return T_AND; }	//type and
"||"		{ return T_OR; }	//type or
"!="		{ return T_NE; }	//type not equal
"{"			{ return L_CL; }
"}"			{ return R_CL; }
";"			{ return EOS; } 	//semicolon (end of statement)
":"			{ return EOC; } 	//colon (end of label)
"("			{ return L_BR; }	//left bracket
")"			{ return R_BR; } 	//right bracket
">"			{ return T_GT; }	//type greater than
"<"			{ return T_LT; }	//type less than
">="		{ return T_GTEQ; }	//type greater than
"<="		{ return T_LTEQ; }	//type less than
"="			{ return S_EQ; }	//type equal
"+"			{ return S_ADD; } 	
"-"			{ return S_SUB; } 	
"*"			{ return S_MUL; }
"/"			{ return S_DIV; }
"%"			{ return S_MOD; }
"&"			{ return S_AND; } 	
"|"			{ return S_OR; } 	
"^"			{ return S_XOR; }
"!"			{ return S_NOT; }
"[" 		{ return L_SB; }	//left square bracket
"]" 		{ return R_SB; } 	//right square bracket
"->"		{ return S_RNEXT; }
"."			{ return S_PDOT; }

var 		{ return P_VAR; }
new 		{ return P_NEW; }
hex			{ return P_HEX; }
sizeof		{ return P_SIZEOF; }

goto		{ return P_GOTO; }
if 			{ return P_IF; }
else		{ return P_ELSE; }
while		{ return P_WHILE; }
function 	{ return P_FUNCTION; }
alias		{ return P_ALIAS; }
event       { return P_EVENT; }
switch		{ return P_SWITCH; }
case 		{ return P_CASE; } 
default		{ return P_DEFAULT; }
break		{ return P_BREAK; }
for			{ return P_FOR; }
do 			{ return P_DO; }
continue 	{ return P_CONTINUE; }
return 		{ return P_RETURN; }
proto		{ return P_PROTO; }
extern 		{ return P_EXTERN; }
syscall		{ return P_SYSCALL; }

class		{ return P_CLASS; }
interface 	{ return P_INTERFACE; }
package 	{ return P_PACKAGE; }
import	 	{ return P_IMPORT; }
public 		{ return P_PUBLIC; }
private 	{ return P_PRIVATE; }
protected	{ return P_PROTECTED; }

\"{LETTER}\" { 
	char buffer[4096];
	uint16 length;
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, yytext + 1, strlen(yytext) - 2);
	memset(yylval.string, 0, sizeof(yylval.string));
	length = lp_unescape(_RECAST(uchar *, yylval.bytes.value), _RECAST(uchar *, buffer));
	yylval.bytes.length = length;
	return CONSTANT; 
}

\/\/		{ BEGIN(LINE_COMMENT_SECTION); }

{NUMBER} { 			
	char buffer[256];	
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, yytext, strlen(yytext));
	memcpy((uchar *)(yylval.bytes.value), buffer, strlen(buffer) + 1);
	yylval.bytes.length = strlen(buffer);
	if(strstr(buffer, ".") != NULL) return NUMERIC;
	return CONSTANT; 
}

-{NUMBER} { 				
	char buffer[256];	
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, yytext, strlen(yytext));
	memcpy((uchar *)(yylval.bytes.value), buffer, strlen(buffer) + 1);
	yylval.bytes.length = strlen(buffer);
	if(strstr(buffer, ".") != NULL) return N_NUMERIC;
	return N_CONSTANT;
}

0x{HEX} { 				
	char buffer[1024];	
	memset(buffer, 0, 256);
	sprintf(buffer, "%i", lp_hex2integer(yytext + 2));
	memcpy((uchar *)(yylval.bytes.value), buffer, strlen(buffer) + 1);
	yylval.bytes.length = strlen(buffer);
	return CONSTANT; 
}

{ALPHABET} {	
	pp_const * pconst = NULL;
	symrec * var = NULL;
	char buffer[4096];
	uint32 offset = 0;
	pconst = lp_select(_RECAST(uchar *, yytext));			/* select from preprocessor constants */
	if(pconst == NULL) {
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, yytext, strlen(yytext));
		memcpy((uchar *)(yylval.string), buffer, strlen(buffer) + 1);
		return VARIABLE;
	} else {							/* preprocessor constant found */
		memset(buffer, 0, sizeof(buffer));
		memcpy(buffer, pconst->value, pconst->length);
		memcpy(_RECAST(uchar *, yylval.bytes.value), buffer, pconst->length);
		yylval.bytes.length = pconst->length;
		return CONSTANT;
	}	
}

. { return(yytext[0]); }
%%

int yywrap(void) { return -1; }

void yyinit(void) { 
	YY_FLUSH_BUFFER;
	BEGIN(INITIAL);	
}

/************************** End Scanner File *****************************/



