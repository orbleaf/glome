#ifndef APDU_TAB_H
#include "../defs.h"
#include "token.h"

typedef struct array {
	uint16 length;
	uchar value[4096];
} array;

typedef union semrec 
{
	uchar string[4096];
	array bytes;
	yytoken token;
} YYSTYPE;

/*#define	BYTES 		TOKEN_BYTES
#define RESETCARD 	TOKEN_RESETCARD
#define RST 		TOKEN_RST		
#define INP 			TOKEN_IN			
#define INPUT 		TOKEN_INPUT		
#define OUTPUT 		TOKEN_OUTPUT		
#define VERSION 	TOKEN_VERSION	
#define SQOP 		TOKEN_SQOP		
#define SQEND 		TOKEN_SQEND		
#define BACKSLASH 	TOKEN_BACKSLASH 	
#define RESPONSE 	TOKEN_RESPONSE 	
#define EOL 		TOKEN_EOL*/			

extern YYSTYPE yylval;
//extern int line;
extern int errors;

#define APDU_TAB_H
#endif
