#include "../defs.h"
#include "../config.h"
#include "pkg_linker.h"
#include <stdarg.h>
#include <stdio.h>

FILE * _scfile = NULL;
uint16 _sch_length = 0;
uchar _sch_buffer[1024];
uint16 _scd_length = 0;
uchar _scd_buffer[64512];
uint16 _schex_length = 0;
uchar _schex_buffer[65536];

void sc_init(uchar * filename) {
	uchar scrpath[512];
	uchar index = 0;
	if(filename == NULL) return;
	strcpy(_RECAST(char *,scrpath), _RECAST(const char *, filename));
	index = (uint32)strchr(_RECAST(const char *,scrpath), '.') - (uint32)scrpath;
	scrpath[index] = 0;
	sprintf(_RECAST(char *,scrpath), "%s%s", scrpath, ".apdu");
	_scfile = fopen(_RECAST(const char *,scrpath), "w");
}

pk_method * sc_install_menu(uchar * menuname, pk_method * methrec) {
	return methrec;
}
 
pk_method * sc_install_event(uchar eventid, pk_method * methrec) {
	return methrec;
}

void sc_install_codes(uchar * codes, uint32 csize) {
	//printf("install codes size : %d bytes\n", csize);
	_scd_length = csize;
	memcpy(_scd_buffer, codes, csize);
}

void sc_install_header(uchar * data, uint32 dsize) {
	_sch_length = dsize;
	memcpy(_sch_buffer, data, dsize);
}

void sc_flush(void) {
	uint16 i, j;
	uint16 s2x;
	uchar hbuf[512];
	printf("generate apdu script\n");
	_schex_length = _sch_length + _scd_length;
	memcpy(_schex_buffer, _sch_buffer, _sch_length);
	memcpy(_schex_buffer + _sch_length, _scd_buffer, _scd_length); 
	if(_scfile == NULL) return;
	fprintf(_scfile, "A0A4000002%04X\r\n", 0x3F00); 
	fprintf(_scfile, "A0A4000002%04X\r\n", 0x2000);
	fprintf(_scfile, "A0A4000002%04X\r\n", 0x6F01);
	i = 0;
	while(i < _schex_length) {
		//convert to hexstring
		s2x = ((i+128) < _schex_length)?128:(_schex_length-i); 
		for(j=0;j<s2x;j++) {
			sprintf(_RECAST(char *,hbuf + (j*2)), "%02X", (unsigned char)_schex_buffer[i + j]);
		}
		hbuf[j*2] = 0;
		fprintf(_scfile, "A0D6%04x%02X%s\r\n", i, s2x, hbuf);
		i+=s2x;
	}
	fflush(_scfile);
}

void sc_cleanup(void) {
}