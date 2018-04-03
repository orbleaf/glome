#include "../defs.h"
#include "../config.h"
#include "pkg_encoder.h"
#include "scr_generator.h"
#include <stdio.h>

pk_object * _pk_root = NULL;
uchar _pkg_buffer[65536];
FILE * _pkfile = NULL;

pk_object * pk_get_root() {
	return _pk_root;
}

static pk_class * pk_create_class(uchar * name) {
	pk_class * pkc = (pk_class *)malloc(sizeof(pk_class));
	((pk_object *)pkc)->tag = PK_TAG_CLASS;
	((pk_object *)pkc)->next = NULL;
	((pk_object *)pkc)->codebase = NULL;
	memset(pkc->name, 0, sizeof(pkc->name));
	strcpy(_RECAST(char *,pkc->name), _RECAST(const char *, name));
	pkc->properties = NULL;
	return pkc;
}

static pk_method * pk_create_method(uchar * name, uchar numargs, uint16 offset) {
	pk_method * pkm = (pk_method *)malloc(sizeof(pk_method));
	((pk_object *)pkm)->tag = PK_TAG_METHOD;
	((pk_object *)pkm)->next = NULL;
	((pk_object *)pkm)->codebase = NULL;
	memset(pkm->name, 0, sizeof(pkm->name));
	strcpy(_RECAST(char *,pkm->name), _RECAST(const char *, name));
	pkm->parent = NULL;
	pkm->numargs = numargs;
	pkm->offset = offset;
	pkm->event = 0xFF;
	memset(pkm->alias, 0, sizeof(pkm->alias));
	return pkm;
}

pk_class * pk_select_class(pk_object * root, uchar * name) {
	pk_object * iterator = (pk_object *)root;
	if(iterator == NULL) return (pk_class *)iterator;
	while(iterator != NULL) {
		if(strcmp(_RECAST(const char *,((pk_class *)iterator)->name), _RECAST(const char *, name)) == 0) {
			return (pk_class *)iterator;
		}
		iterator = (pk_object *)iterator->next;
	}
	return (pk_class *)iterator;
}

pk_class * pk_install_class(pk_object ** root, uchar * name) {
	pk_object * iterator = (pk_object *)*root;
	pk_class * pkc = pk_create_class(name);
	if(*root == NULL) *root = (pk_object *)pkc;
	else {
		while(iterator->next != NULL) {
			iterator = (pk_object *)iterator->next;
		}
		iterator->next = pkc;
	}
	return pkc;
}

pk_class * pk_register_class(uchar * name) {
	return pk_install_class(&_pk_root, name);
}

pk_method * pk_select_method(pk_class * parent, uchar * name) {
	pk_object * iterator;
	if(parent == NULL) return NULL;
	iterator = (pk_object *)parent->properties;
	if(iterator == NULL) return (pk_method *)iterator;
	while(iterator != NULL) {
		if(strcmp(_RECAST(const char *,((pk_method *)iterator)->name), _RECAST(const char *, name)) == 0) {
			return (pk_method *)iterator;
		}
		iterator = (pk_object *)iterator->next;
	}
	return (pk_method *)iterator;
}

pk_method * pk_register_method(pk_class * parent, uchar * name, uchar numargs, uint16 offset) {
	pk_object * iterator;
	pk_method * pkm;
	if(parent == NULL) return (pk_method *)parent;
	pkm = pk_create_method(name, numargs, offset);
	((pk_method *)pkm)->parent = parent;
	iterator = (pk_object *)parent->properties;
	if(iterator == NULL) parent->properties = pkm;
	else {
		while(iterator->next != NULL) {
			iterator = (pk_object *)iterator->next;
		}
		iterator->next = pkm;
	}
	return pkm;
}

pk_method * pk_register_method_args(pk_class * parent, uchar * name, uchar numargs) {
	pk_method * pkm = pk_select_method(parent, name);
	pkm->numargs = numargs;
	return pkm;
}

pk_method * pk_register_menu(pk_class * parent, uchar * name, uchar * aliasname) {
	pk_method * pkm = pk_select_method(parent, name);
	if(pkm == NULL) return pkm;
	strcpy(_RECAST(char *, pkm->alias), _RECAST(const char *, aliasname));
#ifdef STANDALONE_COMPILER
	sc_install_menu(aliasname, pkm);
#endif
	return pkm;
}

pk_method * pk_register_event(pk_class * parent, uchar * name, uchar id) {
	pk_method * pkm = pk_select_method(parent, name);
	if(pkm == NULL) return pkm;
	pkm->event = id;
#ifdef STANDALONE_COMPILER
	sc_install_event(id, pkm);
#endif
	return pkm;
}

static uint32 pk_push_tlv(uchar t, uint16 l, uchar * v, uchar * buffer) {
	uint16 sz = 0;
	buffer[0] = t;
	if(l < 127) {
		buffer[1] = l;
		sz = 2;
	} else {
		buffer[1] = 0x82;
		buffer[2] = (uchar)(l >> 8);
		buffer[3] = (uchar)l;
		sz = 4;
	}
	memcpy(buffer + sz, v, l);
	return (l + sz);
}

uint32 pk_flush_root() {								//flush root with ASN.1 BER(X.690) Basic Encoding Rules
	uint32 sz = 0, csz = 0;
	//csz += pk_push_tlv(ASN_TAG_INTEGER, 2, &offset, buffer + sz + csz + 6);
	csz = pk_flush_entries(_pk_root, _pkg_buffer + 6);
	csz += pk_flush_objects(_pk_root, _pkg_buffer + csz + 6);
	//printf("csz : %d\n", csz);
	sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, _pkg_buffer + 6, _pkg_buffer);
	return sz;
}

uint32 pk_flush_entries(pk_object * object, uchar * buffer) {
	pk_object * iterator = object;
	uint32 sz = 0, csz = 0;
	uint16 offset = 0;
	uchar t_buffer[48];
	while(iterator != NULL) {
		switch(iterator->tag) {
			case PK_TAG_CLASS:
				if(((pk_class *)iterator)->properties != NULL) {
					sz += pk_flush_entries((pk_object *)((pk_class *)iterator)->properties, buffer + sz);
				}
				//sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + 2, buffer + sz);

				break;
			case PK_TAG_METHOD:

				offset = end_swap16(((pk_method *)iterator)->offset);
				csz = 0;
				if(strlen(_RECAST(const char *,((pk_method *)iterator)->alias)) != 0) {
					memcpy(t_buffer, &offset, 2);
					strcpy(_RECAST(char *, t_buffer) + 2, _RECAST(const char *, ((pk_method *)iterator)->alias));
					//printf("menu %s\n", ((pk_method *)iterator)->alias);
					sz += pk_push_tlv(ASN_TAG_OCTSTRING, strlen(_RECAST(const char *,((pk_method *)iterator)->alias)) + 2, t_buffer, buffer + sz + csz);
				}
				if(((pk_method *)iterator)->event != 0xFF) {
					memcpy(t_buffer, &offset, 2);
					t_buffer[2] = ((pk_method *)iterator)->event;
					sz += pk_push_tlv(ASN_TAG_INTEGER, 3, t_buffer, buffer + sz + csz);
				}
				break;
			case PK_TAG_PROPERTY: break;
		}	
		iterator = (pk_object *)iterator->next;
		//buffer += sz;
	}
	//sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + 2, buffer + sz);
	return sz;
}

uint32 pk_flush_objects(pk_object * object, uchar * buffer) {
	pk_object * iterator = object;
	uint32 sz = 0, csz = 0, dsz = 0;
	uint16 offset = 0;
	uchar d_buffer[255];
	while(iterator != NULL) {
		switch(iterator->tag) {
			case PK_TAG_CLASS:
				csz = 0;
				csz += pk_push_tlv(ASN_TAG_IA5STRING, strlen(_RECAST(const char *, ((pk_class *)iterator)->name)), ((pk_class *)iterator)->name, buffer + sz + csz + 4);
				if(((pk_class *)iterator)->properties != NULL) {
					dsz = csz;
					csz += pk_flush_objects((pk_object *)((pk_class *)iterator)->properties, buffer + sz + csz + 2);
					//sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + dsz + 2, buffer + sz + dsz);
				}
				sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + 4, buffer + sz);

				break;
			case PK_TAG_METHOD:

				offset = end_swap16(((pk_method *)iterator)->offset);
				csz = 0;
				memcpy(d_buffer + 3, _RECAST(const char *, ((pk_method *)iterator)->name), strlen(_RECAST(const char *,((pk_method *)iterator)->name)));
				d_buffer[2] = ((pk_method *)iterator)->numargs;
				memcpy(d_buffer, (uchar *)&offset, 2);
				//printf("pushed : %s\n", ((pk_method *)iterator)->name);
				sz += pk_push_tlv(ASN_TAG_OCTSTRING, strlen(_RECAST(const char *, ((pk_method *)iterator)->name)) + 3, d_buffer, buffer + sz + csz + 2);
				//csz += pk_push_tlv(ASN_TAG_INTEGER, 1, (uchar *)&((pk_method *)iterator)->numargs, buffer + sz + csz + 2);
				//csz += pk_push_tlv(ASN_TAG_INTEGER, 2, (uchar *)&offset, buffer + sz + csz + 2);
				//sz += pk_push_tlv(ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED, csz, buffer + sz + 2, buffer + sz);
				break;
			case PK_TAG_PROPERTY: break;
		}	
		iterator = (pk_object *)iterator->next;
		//buffer += sz;
	}
	return sz;
}

void pk_init(uchar * inpath) {
//#ifdef STANDALONE_COMPILER
	uchar pkpath[512];
	uchar index = 0;
	_pk_root = NULL;
	if(inpath == NULL) return;
	strcpy(_RECAST(char *,pkpath), _RECAST(const char *, inpath));
	index = (uint32)strchr(_RECAST( char *, pkpath), '.') - (uint32)pkpath;
	pkpath[index] = 0;
	sprintf(_RECAST(char *,pkpath), "%s%s", pkpath, ".orb");
	_pkfile = fopen(_RECAST(const char *, pkpath), "wb");
	if(_pkfile == NULL) return;
//#endif
}

void pk_file_flush(uchar * codes, uint32 csize) {
//#ifdef STANDALONE_COMPILER
	uint32 size = 0;
	if(_pkfile != NULL) {
		size = pk_flush_root();	
		//install codes for script generator
#ifdef STANDALONE_COMPILER
		sc_install_header(_pkg_buffer, size);			//install package header
		sc_install_codes(codes, csize);				//install raw codes
#endif
		size += pk_push_tlv(ASN_TAG_BMPSTRING, csize, codes, _pkg_buffer + size);
		fwrite(_pkg_buffer, size, 1, _pkfile);
		fclose(_pkfile);
	}
//#endif
}
