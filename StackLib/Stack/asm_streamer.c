#include "../defs.h"
#include "../config.h"
#include "asm_streamer.h"
#include <stdio.h>
#include <stdarg.h>

//#ifdef STANDALONE_COMPILER
FILE * _asmfile = NULL;
//#if !IS_CODEDEBUG
//uint32 _current_offset = 0;
//#else 
extern uint32 _current_offset;			//already defined by il_streamer
//#endif
as_record * as_head = NULL;
as_record * as_tail = NULL;
//#endif

void as_push_record(uchar * msg) {
	as_record * rec = (as_record *)malloc(sizeof(as_record));
	rec->next = NULL;
	memset(rec->buffer, 0, 256);
	if(as_tail == NULL) {
		as_tail = rec;
		as_head = rec;
	} else {
		as_tail->next = rec;
		as_tail = rec;
	}
	strncpy(_RECAST(char *, rec->buffer), _RECAST(const char*, msg), 255);
}

as_record * as_get_enumerator() {
	return as_head;
}

as_record * as_get_next_record(as_record * rec) {
	return rec->next;
}

void as_init(uchar * inpath) {
//#ifdef STANDALONE_COMPILER
	uchar asmpath[512];
	uchar index = 0;
	as_record * temp;
	if(as_head != as_tail) {
		temp = as_head;
		as_head = as_head->next;
		free(temp);
	}
	_current_offset = 0;
	as_head = NULL;
	as_tail = NULL;
	if(inpath == NULL) return;
	strcpy(_RECAST(char *, asmpath), _RECAST(const char *, inpath));
	index = (uint32)strchr(_RECAST(const char *, asmpath), (int)'.') - (uint32)asmpath;
	asmpath[index] = 0;
	sprintf(_RECAST(char *,asmpath), "%s%s", asmpath, ".lst");
	_asmfile = fopen(_RECAST(const char *,asmpath), "w");
	if(_asmfile == NULL) return;
	fprintf(_asmfile, ";===========================================================\n");
	fprintf(_asmfile, ";6S Assembly Generator v%d.%d\n", IS_MAJOR_VERSION, IS_MINOR_VERSION);
	fprintf(_asmfile, ";Agus Purwanto Copyleft 2014\n");
	fprintf(_asmfile, ";===========================================================\n");
//#endif
}

void as_print(uint32 offset, uchar * str, ...) {
//#if STANDALONE_COMPILER
	uchar buffer[4096];
	if(offset >= _current_offset) {
		if(_asmfile != NULL) {
			fprintf(_asmfile, "%04x:%s\n", offset, str);
		}
		sprintf((char *)buffer, "%04x:%s\n", offset, str);
		as_push_record(_RECAST(uchar *, buffer));
		_current_offset = offset;
	}
//#endif
}

void as_info(uchar * str) {
//#if STANDALONE_COMPILER
	uchar i=0;
	uchar * iterator = str;
	uchar buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	if(_asmfile != NULL) {
		fputc(';', _asmfile);
		while(*iterator != NULL) {
			if(*iterator == '\n') break;
			fputc(*iterator, _asmfile);
			iterator++;
		}
		fputc('\r', _asmfile);
		fputc('\n', _asmfile);
	}
	iterator = str;
	buffer[i++] = ';';
	while(*iterator != NULL && i < 253) {
		if(*iterator == '\n') break;
		//fputc(*iterator, _asmfile);
		buffer[i++] = *iterator;
		iterator++;
	}
	as_push_record(_RECAST(uchar *, buffer));
//#endif
}

void as_reset(uchar * message) {
//#if STANDALONE_COMPILER
	if(_asmfile != NULL) {
		_current_offset = 0;
		fprintf(_asmfile, ";===========================================================\n");
		fprintf(_asmfile, ";%s\n", message);
		fprintf(_asmfile, ";===========================================================\n");
	}
//#endif
}

void as_flush(void) {
//#ifdef STANDALONE_COMPILER
	if(_asmfile != NULL) fclose(_asmfile);
//#endif
}
