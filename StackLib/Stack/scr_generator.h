#ifndef _SCR_GENERATOR__H
#include "..\defs.h"
#include "sym_table.h"

void sc_init(uchar * filename);
pk_method * sc_install_menu(uchar * menuname, pk_method * methrec);
pk_method * sc_install_event(uchar eventid, pk_method * methrec);
void sc_install_codes(uchar * codes, uint32 csize);
void sc_install_header(uchar * data, uint32 dsize);
void sc_flush(void);
void sc_cleanup(void);

#define _SCR_GENERATOR__H
#endif