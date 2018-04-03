#ifndef _SYM_TABLE__H
#include "../defs.h"

#define SYM_TYPE_CONST		0x01
#define SYM_TYPE_VAR		0x02
#define SYM_TYPE_FUNCTION	0x04
#define SYM_TYPE_LABEL		0x08
#define SYM_TYPE_CLASS		0x10
#define SYM_TYPE_ARRAY		0x20
#define SYM_TYPE_EXTERNAL	0x80

#define SYM_UNDEFINED		0xFFFFFFFF

typedef struct symrec {		//symbol record
	struct symrec * parent;		//parent function
	struct symrec * next;		//pointer to next record
	uchar type;				//record type
	uchar length;			//record name length
	uchar name[514];		//record name
	uint32 offset;				//for function, label, const, var
	void * list;				//sublist for backpatching algorithm (label)
	uchar pcount;				//parameter count (for function only)
	uchar ptotal;				//total parameter for this function
	void * sa_currec;			//static analyzer current record assign to this variable
} symrec;

typedef struct jmprec {
	struct jmprec * next;			//pointer to next record
	uchar ins;
	uint32 offset;				//offset for matching jump instruction
} jmprec;

void st_push_table();
void st_pop_table();
symrec * st_sym_create_array(uchar * arrval, uint8 length, uint32 offset);
symrec * st_sym_select_array(uchar * arrval, uint16 length);
symrec * st_sym_insert_array(uchar * arrval, uint8 length, uint32 offset) ;
symrec * st_sym_create(uchar type, uchar * name, uint32 offset);
symrec * st_sym_add_record(symrec * newrec);
symrec * st_sym_select(uchar type, uchar * name);
symrec * st_sym_insert(uchar type, uchar * name, uint32 offset);
void st_sym_delete(uchar type, uchar * name);
void st_sym_dispose(symrec * rec);
void st_sym_clear_locals(symrec * parent);
void st_sym_update_parent(symrec * old, symrec * newp);
void st_sym_clear_globals(void);
jmprec * st_jmp_add(symrec * rec, uchar ins, uint32 offset);			//jump offset for backpatching
void st_jmp_clear(symrec * rec);							//clear all jumplist after backpatching
void st_vardump(symrec * rec);

#define _SYM_TABLE__H
#endif
