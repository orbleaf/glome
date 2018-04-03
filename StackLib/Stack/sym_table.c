#include "../defs.h"
#include "sym_table.h"
#include "asm_streamer.h"
#include "sem_proto.h"

#define MAX_LIST_DEPTH		32
symrec * _symhead = NULL;

symrec * _symstack[MAX_LIST_DEPTH];
uint8 _symindex = 0;

void st_push_table() {
	symrec * iterator = NULL;
	uint32 stln;
	uint32 length;
	symrec * temp;
	symrec * previtr = NULL;
	if(_symindex == MAX_LIST_DEPTH) return;
	_symstack[_symindex++] = _symhead;
	//copy previous table
	iterator = _symhead;
	_symhead = NULL;
	previtr = NULL;
	while(iterator != NULL) {							//check for iterator
		if((iterator->type & SYM_TYPE_EXTERNAL) != 0) {						//iterate by type
			temp = (symrec *)malloc(sizeof(symrec));
			if(temp != NULL) {
				memcpy(temp, iterator, sizeof(symrec));
				temp->next = NULL;
				if(_symhead == NULL) { _symhead = temp; }
				else { previtr->next = temp; }
				previtr = temp; 
				//printf("copy : %s\n", temp->name);
			}
		}
		iterator = iterator->next;
	}
}

void st_pop_table() {
	if(_symindex == 0) return;
	_symhead = _symstack[--_symindex];
}

symrec * st_sym_get_prev_record(symrec * rel) {
	symrec * iterator = _symhead;
	if(iterator == NULL) return NULL;
	while(iterator->next != rel) {
		//printf("current iterator : %s, %d\n", iterator->name, iterator->next);
		iterator = iterator->next;
	}
	//printf("current iterator : %s, %d\n", iterator->name, iterator->next);
	//printf("last : %s, next : %d\n", iterator->name, iterator->next);
	return iterator;
}

symrec * st_sym_add_record(symrec * newrec) {
	symrec * prevrec = NULL;
	prevrec = st_sym_get_prev_record(NULL);
	if(prevrec == NULL) {
		_symhead = newrec;
	} else {
		//printf("%d->%d\n", prevrec, newrec);
		if(newrec != prevrec) prevrec->next = newrec;
	}
	return newrec;
}

symrec * st_sym_select(uchar type, uchar * name) {
	symrec * iterator = _symhead;
	uint32 stln;
	uint32 length;
	if(iterator == NULL) return NULL;
	type = type & 0x7F;
	while(iterator != NULL) {							//check for iterator
		if(((iterator->type & 0x7F) & type) != 0) {						//iterate by type
			stln = strlen(_RECAST(const char *, name));
			length = (iterator->length > stln)?iterator->length:stln;
			if(memcmp(name, iterator->name, length) == 0) {		//compare name
				//printf("select %s\n", iterator->name);
				return iterator;
			}
		}
		//if(memcmp(name, "get_value", 9) == 0) printf("%s = %s,", iterator->name, name);
		//printf("%s,", iterator->name, name);
		iterator = iterator->next;
	}
	return NULL;
}

symrec * st_sym_select_array(uchar * arrval, uint16 length) {
	symrec * iterator = _symhead;
	uint32 stln;
	uchar type = SYM_TYPE_ARRAY;
	if(iterator == NULL) return NULL;
	type = type & 0x7F;
	while(iterator != NULL) {							//check for iterator
		if(((iterator->type & 0x7F) & type) != 0) {						//iterate by type
			stln = length;
			if(memcmp(arrval, iterator->name, stln) == 0) {		//compare name
				//printf("select %s\n", iterator->name);
				return iterator;
			}
		}
		iterator = iterator->next;
	}
	return NULL;
}

symrec * st_sym_create(uchar type, uchar * name, uint32 offset) {
	//allocate memory
	symrec * rec = (symrec *)malloc(sizeof(symrec));	
	memset(rec, 0, sizeof(symrec));
	//initialize value
	rec->type = type;
	rec->length = strlen(_RECAST(const char *, name));
	memcpy(_RECAST(char *, rec->name), _RECAST(const char *, name), 255);
	rec->offset = offset;
	rec->list = NULL;
	rec->ptotal = 0;
	rec->pcount = 0;
	rec->parent = NULL;
	rec->next = NULL;
	rec->sa_currec = NULL;
	return rec;
}

symrec * st_sym_create_array(uchar * arrval, uint8 length, uint32 offset) {
	//allocate memory
	symrec * rec = (symrec *)malloc(sizeof(symrec));	
	memset(rec, 0, sizeof(symrec));
	//initialize value
	rec->type = SYM_TYPE_ARRAY;
	rec->length = length;
	memcpy(rec->name, arrval, length);
	rec->offset = offset;
	rec->list = NULL;
	rec->ptotal = 0;
	rec->pcount = 0;
	rec->parent = NULL;
	rec->next = NULL;
	rec->sa_currec = NULL;
	return rec;
}

symrec * st_sym_insert(uchar type, uchar * name, uint32 offset) {
	symrec * rec = st_sym_create(type, name, offset);
	//printf("INSERT NEW SYMBOL : %s\n", rec->name);
	//if(rec->next == NULL) printf("%s->NULL\n", rec->name);
	rec = st_sym_add_record(rec);
	//if(rec->next != NULL) printf("rec->name[next] != null %s->%s\n", rec->name, rec->next->name);
	return rec;
}

symrec * st_sym_insert_array(uchar * arrval, uint8 length, uint32 offset) {
	symrec * rec = st_sym_create_array(arrval, length, offset);
	//printf("INSERT NEW SYMBOL : %s\n", rec->name);
	rec = st_sym_add_record(rec);
	return rec;
}

void st_sym_delete(uchar type, uchar * name) {
	symrec * prevrec = NULL;
	symrec * candidate = st_sym_select(type, name);
	if(candidate == NULL) return;						//not found
	prevrec = st_sym_get_prev_record(candidate);
	if(prevrec == NULL) {
		_symhead = candidate->next;
	} else {
		prevrec->next = candidate->next;
	}
	st_sym_dispose(candidate);
}


void st_sym_dispose(symrec * rec) {
	memset(rec, 0, sizeof(symrec));
	free(rec);
}

void st_sym_clear_locals(symrec * parent) {
	symrec * previtr = NULL;
	symrec * iterator = _symhead;
	symrec * candidate = NULL;
	symrec * temp = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		if(iterator->parent != parent) {	
			previtr = iterator;
			iterator = iterator->next;
		} else {
			candidate = iterator;
			temp = iterator->next;
			if(previtr != NULL) { previtr->next = candidate->next; }
			else { _symhead = candidate->next; }
			if(candidate->list != NULL) {
				st_jmp_clear(candidate);
				switch(candidate->type & 0x0F) {
					case SYM_TYPE_VAR: sp_error(SP_ERR_UNREF_VARIABLE, _RECAST(uchar *, candidate->name)); break;
					case SYM_TYPE_LABEL: sp_error(SP_ERR_UNREF_LABEL, _RECAST(uchar *, candidate->name)); break;
					default: sp_error(SP_ERR_UNREF_SYMBOL, _RECAST(uchar *, candidate->name)); break;
				}
			}
			st_vardump(candidate);
			memset(candidate, 0, sizeof(symrec));		//clear previous data
			free(candidate);
			iterator = temp;
		}
	}
}


void st_sym_update_parent(symrec * old, symrec * newp) {
	symrec * previtr = NULL;
	symrec * iterator = _symhead;
	symrec * candidate = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		if(iterator->parent == old) {
			iterator->parent = newp;
		}
		iterator = iterator->next;
	}
}

void st_sym_clear_globals(void) {
	symrec * previtr = NULL;
	symrec * iterator = _symhead;
	symrec * candidate = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		candidate = iterator;
		iterator = iterator->next;
		if(candidate->list != NULL) {
			st_jmp_clear(candidate);
			switch(candidate->type & 0x0F) {
				case SYM_TYPE_VAR: sp_error(SP_ERR_UNREF_VARIABLE, _RECAST(uchar *, candidate->name)); break;
				case SYM_TYPE_FUNCTION: sp_error(SP_ERR_UNREF_FUNCTION, _RECAST(uchar *, candidate->name)); break;
				case SYM_TYPE_LABEL: sp_error(SP_ERR_UNREF_LABEL, _RECAST(uchar *, candidate->name)); break;
				default: sp_error(SP_ERR_UNREF_SYMBOL, _RECAST(uchar *, candidate->name)); break;
			}
		}
		st_vardump(candidate);
		_symhead = iterator;
		memset(candidate, 0, sizeof(symrec));		//clear previous data
		free(candidate);
	}
	_symhead = NULL;
}

static jmprec * st_jmp_get_last_record(symrec * rec) {
	jmprec * iterator = (jmprec *)rec->list;
	if(iterator == NULL) return NULL;
	while(iterator->next != NULL) {
		iterator = iterator->next;
	}
	return iterator;
}

jmprec * st_jmp_add(symrec * rec, uchar ins, uint32 offset) {			//jump offset for backpatching
	jmprec * prevrec = NULL;
	jmprec * jrec = (jmprec *)malloc(sizeof(jmprec));
	jrec->next = NULL;
	jrec->ins = ins;
	jrec->offset = offset;
	prevrec = st_jmp_get_last_record(rec);
	if(prevrec == NULL) rec->list = jrec;
	else {
		prevrec->next = jrec;
	}
	return jrec;
}

void st_jmp_clear(symrec * rec) {							//clear all jumplist after backpatching
	jmprec * iterator = (jmprec *)rec->list;
	jmprec * candidate = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {
		candidate = iterator;
		iterator = iterator->next;
		memset(candidate, 0, sizeof(jmprec));
		free(candidate);
	}
	rec->list = NULL;										//remove pointer to list
}

void st_vardump(symrec * rec) {
	uchar dbgbuf[280];
	switch(rec->type) {
		case SYM_TYPE_CONST:
			sprintf(_RECAST(char *, dbgbuf),  "***VARDUMP***% 16s    CONSTANT   %04x\n", rec->name, rec->offset);
			as_info(dbgbuf);
			break;
		case SYM_TYPE_VAR:
			sprintf(_RECAST(char *, dbgbuf),  "***VARDUMP***% 16s    VARIABLE   %04x\n", rec->name, rec->offset);
			as_info(dbgbuf);
			break;
		case SYM_TYPE_FUNCTION:
			sprintf(_RECAST(char *, dbgbuf),  "***VARDUMP***% 16s    FUNCTION   %04x\n", rec->name, rec->offset);
			as_info(dbgbuf);
			break;
		case SYM_TYPE_LABEL	:
			sprintf(_RECAST(char *, dbgbuf),  "***VARDUMP***% 16s    LABEL      %04x\n", rec->name, rec->offset);
			as_info(dbgbuf);
			break;
		case (SYM_TYPE_EXTERNAL | SYM_TYPE_FUNCTION):
			sprintf(_RECAST(char *, dbgbuf),  "***VARDUMP***% 16s    EXTERNAL   %04x\n", rec->name, rec->offset);
			as_info(dbgbuf);
			break;
	}
}
