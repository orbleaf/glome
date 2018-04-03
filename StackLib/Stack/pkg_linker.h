#ifndef _PKG_LINKER__H
#include "../defs.h"
#include <stdio.h>
#include "pkg_encoder.h"


void lk_set_root(pk_object * root);
pk_class * lk_select_class(uchar * name);
pk_method * lk_select_method(pk_class * parent, uchar * name);

void lk_import_directory(uchar * path);
pk_object * lk_decode_file(uchar * path);
pk_object * lk_decode_buffer(uint16 length, uchar * buffer);
uint16 lk_decode_header_seq(FILE * file, uint16 offset, void * codebase);
void * lk_decode_data_seq(FILE * file, uint16 offset) ;
uint16 lk_decode_class_seq(FILE * file, uint16 offset, void * codebase);
uint16 lk_decode_class_header(FILE * file, uint16 offset, uint16 length, void * codebase);
uint16 lk_decode_method_header(pk_object * parent, FILE * file, uint16 offset, uint16 length, void * codebase);
void lk_dump_classes();
void lk_clear_entries();
#define _PKG_LINKER__H
#endif
