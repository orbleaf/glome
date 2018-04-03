#include "stdafx.h"


#pragma unmanaged
#include "lex_proto.h"
#include "sem_proto.h"
#include "sym_table.h"
#include "token.h"
#include "pkg_encoder.h"
#include "pkg_linker.h"
#include "il_streamer.h"
#include "il1_optimizer.h"
#include "il2_optimizer.h"
#include "asm_streamer.h"
#include "scr_generator.h"
#include ".\pkg_encoder.c"
#include ".\pkg_linker.c"
#include ".\scr_generator.c"		//APDU generator

struct lk_context {
	uint16 offset;
	uint16 size;
	uchar * buffer;
};

uint16 lk_read(uchar * buf, uint16 size, lk_context * ctx) {
	if(ctx->offset >= ctx->size) return 0;
	if(ctx->offset + size > ctx->size) size = ctx->size - ctx->offset;
	memcpy(buf, ctx->buffer + ctx->offset, size);
	ctx->offset += size;
	return size;
}

uint16 lk_decode_class_header_buffer(lk_context * file, uint16 offset, uint16 length, void * codebase) ;
uint16 lk_decode_class_seq_buffer(lk_context * file, uint16 offset, void * codebase) ;
uint16 lk_decode_header_seq_buffer(lk_context * file, uint16 offset, void * codebase) ;
void * lk_decode_data_seq_buffer(lk_context * file, uint16 offset);


pk_object * lk_decode_buffer(uint16 length, uchar * buffer) {
	uchar t;
	uint16 l = 0;
	pk_object * pkg = NULL;
	void  * codebase = NULL;
	uint16 offset = 0, sz = -1;
	struct lk_context file = { 0, length, buffer };
	//FILE * file = fopen(_RECAST(const char *, path), "rb");
	//if(file == NULL) return pkg;
	//fseek(file, offset, SEEK_SET);
	//offset += fread(&t, 1, 1, file);
	_pk_iroot = NULL;
	offset += lk_read(&t, 1, &file);
	switch(t & 0x1f) {
		case ASN_TAG_SEQ:			//valid class sequence
			//offset += fread(&t, 1, 1, file);		//l
			offset += lk_read(&t, 1, &file);
 			if(t & 0x80) {
				//offset += fread(&l, 1, 2, file);
				offset += lk_read(_RECAST(uchar *, &l), 2, &file);
				l = end_swap16(l);
			} else {
				l = t;
			}
			break;
		default: break;
	}
	offset += l;
	//printf("%d\n", offset);
	if(offset != 0) {
		codebase = lk_decode_data_seq_buffer(&file, offset);
	}
	//printf("code base : %d\n", codebase);
	offset = 0;
	offset = lk_decode_header_seq_buffer(&file, offset, codebase);
	return (pk_object * )codebase;
}

void * lk_decode_data_seq_buffer(lk_context * file, uint16 offset) {
	uchar t;
	uint16 l = 0;
	uint16 e_offset = 0;
	uchar * codebase = NULL;
	//fseek(file, offset, SEEK_SET);
	file->offset = offset;
	//offset += fread(&t, 1, 1, file);
	offset += lk_read(&t, 1, file);
	switch(t & 0x1f) {
		case ASN_TAG_BMPSTRING:			//valid class sequence
			//offset += fread(&t, 1, 1, file);		//l
			offset += lk_read(&t, 1, file);
 			if(t & 0x80) {
				//offset += fread(&l, 1, 2, file);
				offset += lk_read(_RECAST(uchar *, &l), 2, file);
				l = end_swap16(l);
			} else {
				l = t;
			}
			codebase = (uchar *)malloc(l);
			//fread(codebase, 1, l, file);
			lk_read(codebase, l, file);
			//printf("size %d\n", l);
			//offset += csz;
			break;
		default:
			offset = 0;
			break;
	}
	return codebase;
}

uint16 lk_decode_header_seq_buffer(lk_context * file, uint16 offset, void * codebase) {
	uchar t;
	uint16 l = 0;
	uint16 e_offset = 0;
	//fseek(file, offset, SEEK_SET);
	file->offset = offset;
	//offset += fread(&t, 1, 1, file);
	offset += lk_read(&t, 1, file);
	switch(t & 0x1f) {
		case ASN_TAG_SEQ:			//valid class sequence
			//offset += fread(&t, 1, 1, file);		//l
			offset += lk_read(&t, 1, file);
 			if(t & 0x80) {
				//offset += fread(&l, 1, 2, file);
				offset += lk_read(_RECAST(uchar *, &l), 2, file);
				l = end_swap16(l);
			} else {
				l = t;
			}
			e_offset = offset + l;
			while(offset < e_offset) {
				offset = lk_decode_class_seq_buffer(file, offset, codebase);
			}
			//offset += csz;
			break;
		default:
			
			offset = 0;
			break;
	}
	return offset;
}

uint16 lk_decode_class_seq_buffer(lk_context * file, uint16 offset, void * codebase) {
	uchar t;
	uint16 l = 0;
	uint16 csz = 0;
	//fseek(file, offset, SEEK_SET);
	file->offset = offset;
	//offset += fread(&t, 1, 1, file);
	offset += lk_read(&t, 1, file);
	switch(t & 0x1f) {
		case ASN_TAG_SEQ:			//valid class sequence
			//offset += fread(&t, 1, 1, file);		//l
			offset += lk_read(&t, 1, file);
 			if(t & 0x80) {
				//offset += fread(&l, 1, 2, file);
				offset += lk_read(_RECAST(uchar *, &l), 2, file);
				l = end_swap16(l);
			} else {
				l = t;
			}
			offset = lk_decode_class_header_buffer(file, offset, l, codebase);
			break;
		case ASN_TAG_OCTSTRING:			//valid menu entry 
		case ASN_TAG_INTEGER:			//valid event entry
			//offset += fread(&t, 1, 1, file);		//l
			offset += lk_read(&t, 1, file);
 			if(t & 0x80) {
				//offset += fread(&l, 1, 2, file);
				offset += lk_read(_RECAST(uchar *, &l), 2, file);
				l = end_swap16(l);
			} else {
				l = t;
			}
			offset = offset + l;
			break;
		default:
			offset = 0;
			break;
	}
	return offset;
}

uint16 lk_decode_class_header_buffer(lk_context * file, uint16 offset, uint16 length, void * codebase) {
	uchar t;
	uint16 l = 0;
	uint16 csz = 0;
	uchar buffer[256];
	uchar numargs = 0;
	uint16 m_offset = 0;
	pk_object * parent;
	pk_object * method_obj;
	uint16 e_offset = offset + length, d_offset = 0;
	memset(buffer, 0, sizeof(buffer));
	//fseek(file, offset, SEEK_SET);
	file->offset = offset;
	while(offset < e_offset) {
		//offset += fread(&t, 1, 1, file);
		offset += lk_read(&t, 1, file);
		switch(t & 0x1f) {
			case ASN_TAG_IA5STRING:			//valid class sequence
				//offset += fread(&t, 1, 1, file);		//l
				offset += lk_read(&t, 1, file);
	 			if(t & 0x80) {
					//offset += fread(&l, 1, 2, file);
					offset += lk_read(_RECAST(uchar *, &l), 2, file);
					l = end_swap16(l);
				} else {
					l = t;
				}
				//offset += fread(buffer, 1, l, file);
				offset += lk_read(buffer, l, file);
				parent = (pk_object *)pk_install_class(&_pk_iroot, buffer);
				parent->codebase = codebase;
				break;
			case ASN_TAG_OCTSTRING:		//method
				//offset += fread(&t, 1, 1, file);		//l
				offset += lk_read(&t, 1, file);
	 			if(t & 0x80) {
					//offset += fread(&l, 1, 2, file);
					offset += lk_read(_RECAST(uchar *, &l), 2, file);
					l = end_swap16(l);
				} else {
					l = t;
				}
				/*d_offset = offset + l;
				while(offset < d_offset) {
					offset = lk_decode_method_header(parent, file, offset, l, codebase);
				}*/

				if(l <= 3) return offset;
				//offset += fread(&m_offset, 1, 2, file);
				//offset += fread(&numargs, 1, 1, file);
				//offset += fread(buffer, 1, l - 3, file);
				offset += lk_read(_RECAST(uchar *, &m_offset), 2, file);
				offset += lk_read(&numargs, 1, file);
				offset += lk_read(buffer, l-3, file);
				buffer[l-3] = 0;
				//printf("method : %s\n", buffer);
				method_obj = (pk_object *)pk_register_method((pk_class *)parent, buffer, numargs, m_offset);
				method_obj->codebase = codebase;
				break;
			default:
				offset = 0;
				break;
		}
	}
	return offset;
}