#include "il_streamer.h"
#ifndef _ASM_STREAMER__H

typedef struct as_record {
	struct as_record * next;
	uchar buffer[256];
} as_record;

void as_init(uchar * inpath);
void as_print(uint32 offset, uchar * str, ...);
void as_info(uchar * str);
void as_reset(uchar * message);
void as_vardump(uint32 offset, uchar * varname);
void as_flush(void);

as_record * as_get_enumerator() ;
as_record * as_get_next_record(as_record * rec) ;
#define _ASM_STREAMER__H
#endif
