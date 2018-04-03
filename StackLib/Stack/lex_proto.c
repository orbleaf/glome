#include "lex_proto.h"
#include "il_streamer.h"
#include "sym_table.h"
#include "sem_proto.h"
#include <stdio.h>

int lp_getdec( char hex )
{
        switch(hex ) { 
        case 'F': 
        case 'f': return( 15);
        case 'E':
        case 'e': return( 14);
        case 'D':
        case 'd': return( 13);
        case 'C':
        case 'c': return( 12);
        case 'B':
        case 'b': return( 11);
        case 'A':
        case 'a': return( 10);
        case '9': return( 9);
        case '8': return( 8);
        case '7': return( 7);
        case '6': return( 6);
        case '5': return( 5);
        case '4': return( 4);
        case '3': return( 3);
        case '2': return( 2);
        case '1': return( 1);
        default: return( 0);
        }
}

uint32 lp_hex2integer(char * strin) {
	int ret = 0;
	int i = strlen(strin);
	for(i=0;i < strlen(strin); i++)
	{
		//if(strin[i] != '\0') {		//check if end of string
		ret = ret*16;
		ret = ret + lp_getdec(strin[i]);
		//}
	}
	return ret;
}

int lp_hex2dec(char *strin)
{
	int ret = 0;
	int i;
	for(i = 0; i<2; i++)
	{
		if(strin[i] != '\0') {		//check if end of string
			ret = ret*16;
			ret = ret + lp_getdec(strin[i]);
		}
	}
	return ret;
}

int lp_hex2bytes(char * strin, char * p) {
	int length = strlen(strin) / 2;
	int i;
	for(i=0;i<length;i++) {
		*(p + i) = (char)lp_hex2dec(strin + (i * 2));
		//printf("%x", *(p + i));
	}
	//*strOut = p;
	//*sizeout = length;
	return length;
}

char lp_tolower(char c) {
	if(c>=0x61 && c<0x7A)
		c-= 0x20;
	return c;
}

void lp_push_constant(yytoken * token) {					//push constant value to constant pool, used when a constant identified by lex
	is_push_constant(token->length, _RECAST(uchar *,token->bytes)) ;
}

void lp_escape(uchar * s, uchar * t) {
    int i, j;
    i = j = 0;
    
    while ( t[i] ) {
        
        /*  Translate the special character, if we have one  */
        
        switch( t[i] ) {
        case '\n':
            s[j++] = '\\';
            s[j] = 'n';
            break;
            
        case '\t':
            s[j++] = '\\';
            s[j] = 't';
            break;
            
        case '\a':
            s[j++] = '\\';
            s[j] = 'a';
            break;
            
        case '\b':
            s[j++] = '\\';
            s[j] = 'b';
            break;
            
        case '\f':
            s[j++] = '\\';
            s[j] = 'f';
            break;
            
        case '\r':
            s[j++] = '\\';
            s[j] = 'r';
            break;
            
        case '\v':
            s[j++] = '\\';
            s[j] = 'v';
            break;
            
        case '\\':
            s[j++] = '\\';
            s[j] = '\\';
            break;
            
        case '\"':
            s[j++] = '\\';
            s[j] = '\"';
            break;
            
        default:
            
            /*  This is not a special character, so just copy it  */
            
            s[j] = t[i];
            break;
        }
        ++i;
        ++j;
    }
    s[j] = t[i];    /*  Don't forget the null character  */
}

uint16 lp_unescape(uchar * s, uchar * t) {
    int i, j;
    i = j = 0;
    
    while ( t[i] ) {
        switch ( t[i] ) {
        case '\\':
            
            /*  We've found an escape sequence, so translate it  */
            
            switch( t[++i] ) {
            case 'n':
                s[j] = '\n';
                break;
                
            case 't':
                s[j] = '\t';
                break;
                
            case 'a':
                s[j] = '\a';
                break;
                
            case 'b':
                s[j] = '\b';
                break;
                
            case 'f':
                s[j] = '\f';
                break;
                
            case 'r':
                s[j] = '\r';
                break;
                
            case 'v':
                s[j] = '\v';
                break;
                
            case '\\':
                s[j] = '\\';
                break;
                
            case '\"':
                s[j] = '\"';
                break;

			case 'x':			//hexadecimal
				s[j] = lp_getdec(t[++i]);
				s[j] *= 16;
				s[j] += lp_getdec(t[++i]);
				break;

			case '0':			//octal
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				s[j] = lp_getdec(t[++i]);
				s[j] *= 8;
				s[j] += lp_getdec(t[++i]);
				s[j] *= 8;
				s[j] += lp_getdec(t[++i]);

				break;
                
            default:
                
                /*  We don't translate this escape
                    sequence, so just copy it verbatim  */
                
                s[j++] = '\\';
                s[j] = t[i];
            }
            break;
            
        default:
            
            /*  Not an escape sequence, so just copy the character  */
            
            s[j] = t[i];
        }
        ++i;
        ++j;
    }
    s[j] = t[i];    /*  Don't forget the null character  */
	return j;
}

pp_config * p_config = NULL;			//global parser config
pp_config * _lp_conf_stack[20];
int16 _lp_conf_index = 0;
uchar _lp_opc_stack[20];
int16 _lp_opc_index = 0;
pp_const * _lp_opr_stack[20];
int16 _lp_opr_index = 0;

pp_config * lp_push_config(pp_config * config) {
	_lp_conf_stack[_lp_conf_index++] = config;
	return config;
}

pp_config * lp_pop_config() {
	pp_config * config = _lp_conf_stack[_lp_conf_index];
	if((_lp_conf_index - 1) < 0) return _lp_conf_stack[_lp_conf_index];
	free(config);									//delete current config
	return _lp_conf_stack[--_lp_conf_index];		//pop previous config
}

pp_config * lp_peek_config() {
	return _lp_conf_stack[_lp_conf_index - 1];
}

uchar lp_push_opcode(uchar opcode) {
	_lp_opc_stack[_lp_opc_index++] = opcode;
	return opcode;
}

uchar lp_pop_opcode() {
	if((_lp_opc_index - 1) < 0) return NULL;
	return _lp_opc_stack[--_lp_opc_index];		//pop previous config
}

pp_const * lp_push_operand(pp_const * operand) {
	_lp_opr_stack[_lp_opr_index++] = operand;
	return operand;
}

pp_const * lp_pop_operand() {
	if((_lp_opr_index - 1) < 0) return NULL;
	return _lp_opr_stack[--_lp_opr_index];		//pop previous config
}

uchar _lex_state = LEX_STATE_NONE;
pp_const * _pphead = NULL;

static pp_const * lp_get_prev_record(pp_const * rel) {
	pp_const * iterator = _pphead;
	if(iterator == NULL) return NULL;
	while(iterator->next != rel) {
		iterator = iterator->next;
	}
	return iterator;
}

static pp_const * lp_add_record(pp_const * newrec) {
	pp_const * prevrec = NULL;
	prevrec = lp_get_prev_record(NULL);
	if(prevrec == NULL) {
		_pphead = newrec;
	} else {
		prevrec->next = newrec;
	}
	return newrec;
}

pp_const * lp_select(uchar * name) {
	pp_const * iterator = _pphead;
	if(iterator == NULL) return NULL;
	while(iterator != NULL) {							//check for iterator
		if(strcmp(_RECAST(const char *, name), _RECAST(const char *, iterator->name)) == 0) {		//compare name
			return iterator;
		}
		iterator = iterator->next;
	}
	return NULL;
}

pp_const * lp_insert(uchar * name, uchar * value) {
	//allocate memory
	pp_const * iterator;
	pp_const * rec = (pp_const *)malloc(sizeof(pp_const));	
	memset(rec, 0, sizeof(pp_const));
	//initialize value
	memcpy(_RECAST(char *, rec->name), _RECAST(const char *,name), strlen(_RECAST(const char *,name)) + 1); 
	memcpy(_RECAST(char *, rec->value), _RECAST(const char *,value), strlen(_RECAST(const char *,value)) + 1);
	rec->next = NULL;
	//printf("INSERT NEW SYMBOL : %s\n", rec->name);
	//fflush(0);
	rec = lp_add_record(rec);
	return rec;
}

pp_const * lp_update(uchar * name, uchar * value, uint8 length) {
	//allocate memory
	pp_const * rec = lp_select(name);
	if(rec != NULL) {
		memcpy(_RECAST(char *,rec->value), _RECAST(const char *,value), length);
	} else {
		rec = lp_insert(name, value);
	}
	return rec;
}

void lp_delete(uchar * name) {
	pp_const * prevrec = NULL;
	pp_const * candidate = lp_select(name);
	if(candidate == NULL) return;						//not found
	prevrec = lp_get_prev_record(candidate);
	if(prevrec == NULL) {
		_pphead = candidate->next;
	} else {
		prevrec->next = candidate->next;
	}
	free(candidate);
}

void lp_clear_globals(void) {
	pp_const * previtr = NULL;
	pp_const * iterator = _pphead;
	pp_const * candidate = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		candidate = iterator;
		iterator = iterator->next;
		_pphead = iterator;
		memset(candidate, 0, sizeof(pp_const));		//clear previous data
		free(candidate);
	}
	_pphead = NULL;
}

void lp_preprocessor_state(uchar state) {
	//if(_lex_state != 
	switch(state) {
		case LEX_STATE_ELSE: 
			if(_lex_state == LEX_STATE_INVALID) _lex_state = LEX_STATE_NONE;
			else _lex_state = LEX_STATE_INVALID;
			break;
		case LEX_STATE_ENDIF:
			_lex_state = LEX_STATE_NONE;
			break;
		default:
			if(state != LEX_STATE_INVALID) _lex_state = state;
			break;
	}
}

void lp_flush_token() {
	uint32 i,j;
	uchar opc;
	pp_const * op2;
	pp_const * op1;
	op1 = lp_pop_operand();
	if(_lex_state != LEX_STATE_IF) { return; }
	if(op1 == NULL) { _lex_state = LEX_STATE_INVALID; return; }
	i = atoi(_RECAST(const char *,op1->value));
	while((op2 = lp_pop_operand()) != NULL) {
		j = atoi(_RECAST(const char *,op2->value));
		pop_opcode_again:
		switch((opc = lp_pop_opcode())) {
			case LEX_OP_NEG: j = !j; goto pop_opcode_again;
			case LEX_OP_EQ: i = (i==j); break;
			case LEX_OP_GT: i = (i>j); break;
			case LEX_OP_LT: i = (i<j); break;
			case LEX_OP_GTEQ: i = (i>=j); break;
			case LEX_OP_LTEQ: i = (i<=j); break;
			case LEX_OP_OR: i = (i||j); break;
			case LEX_OP_AND: i = (i&&j); break;
			default: break;		
		}
	}
	if((opc = lp_pop_opcode()) && (opc == LEX_OP_NEG)) {
		i = !i;
	}
	if(!i) _lex_state = LEX_STATE_INVALID;
}

void lp_process_token(uchar * token, uint16 length) {
	static uchar const_name[256];
	uint32 ival;
	pp_const * pvar;
	switch(_lex_state) {
		case LEX_STATE_INCLUDE:
			lp_push_config(p_config);
			p_config = sp_open(token);
			_lex_state = LEX_STATE_NONE;
			break;
		case LEX_STATE_PRAGMA:
			
			break;
		case LEX_STATE_DEFINE:
			memcpy(const_name, token, strlen(_RECAST(const char *,token)) + 1);
			lp_update(const_name, _RECAST(uchar *,""), 0);
			_lex_state = LEX_STATE_DEFINE_CONST;
			break;
		case LEX_STATE_DEFINE_CONST:
			lp_update(const_name, token, length);
			_lex_state = LEX_STATE_NONE;
			break;
		case LEX_STATE_IFDEF:
			if(lp_select(token) == NULL) _lex_state = LEX_STATE_INVALID;
			break;
		case LEX_STATE_IF:
			pvar = lp_select(token);
			lp_push_operand(pvar);
			break;
		default: break;
	}
}

void lp_init() {
	_lp_conf_index = 0;
	_lp_opr_index = 0;
	_lp_opc_index = 0;
	//_lex_state = LEX_STATE_NONE;
	//_pphead = NULL;
	//lex state init
	yyinit();
}
