#include "../defs.h"
#include "../config.h"
#include "vm_stack.h"
#include "vm_framework.h"
#include "midgard.h"
#include "../stack/pkg_linker.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef __cplusplus_cli
#pragma managed
#include "..\stdafx.h"
#using <mscorlib.dll>
#include "..\StackLib.h"

using namespace System;
using namespace StackLib;
using namespace System::Runtime::InteropServices;
#endif


//#define VM_CODEDEBUG		1
#ifdef STANDALONE_VIRTUAL_MACHINE
FILE * _ilfile = NULL;
#endif
extern const uchar * _ins_name[];

#if VM_CODEDEBUG
static uint32 _current_offset = 0;
static void vm_debug(uint32 offset, uchar * str, ...) {
	if(offset >= _current_offset) {
		//if(_asmfile != NULL) {
			//fprintf(_asmfile, "%08x:%s\n", offset, str);
		printf("%08x:%s\n", offset, str);
		//_current_offset = offset;
		//}
	}
}
#endif

/*
static uint16 _sp = VM_MAX_STACK_SIZE -1;		//stack pointer
static uint16 _bp = 0;		//base pointer
static uint16 _pc = 0;
void * _base_address = NULL;
static void * _vm_stacks[VM_MAX_STACK_SIZE];
static uchar _vm_state;
uint8 g_recursive_exec = FALSE;		//set recursive_exec flag to false
vm_context _vm_file;
BYTE _vm_current_api = 0;
sys_context g_sysc;
vm_object * g_pVaRetval;
*/

#define VM_CMP_EQ			0x10
#define VM_CMP_GT			0x20
#define VM_CMP_LT			0x40

#ifdef __cplusplus_cli
void vm_init(VM_DEF_ARG, uchar * inpath)
{
	//memset(_vm_stacks, 0, sizeof(_vm_stacks));
	memset(&vm_get_context(), 0, sizeof(vm_context));
	vm_get_context().var_root = 0;
	vm_get_context().vars = &vm_get_context().var_root;
	vm_set_pc(0);
	vm_set_sp(VM_MAX_STACK_SIZE -1);		//stack pointer
	vm_set_bp(0);		//base pointer
}

void vm_close(VM_DEF_ARG)
{

}
#endif

#pragma unmanaged

#ifdef STANDALONE_VIRTUAL_MACHINE
void vm_init(VM_DEF_ARG, uchar * exepath, uchar * inpath) {		//original input file path as parameter
	uchar asmpath[512];
	uchar ilpath[512];
	uint16 i = strlen(exepath);
	m_init_alloc();
	//_ilfile = fopen(inpath, "rb");
	do { i--; }
	while(exepath[i] != '\\' && i != 0);
	//exepath[i] = 0;
	//printf(exepath);
	if(i == 0) {
		lk_import_directory(".\\framework");
	} else {
		exepath[i] = 0;
		sprintf(exepath, "%s\\framework", exepath);
		lk_import_directory(exepath);
	}
	vm_set_package(lk_decode_file(inpath));
	lk_dump_classes();
	//memset(_vm_stacks, 0, sizeof(_vm_stacks));
	memset(&vm_get_context(), 0, sizeof(vm_context));
	vm_get_context().var_root = 0;
	vm_get_context().vars = &vm_get_context().var_root;
	vm_set_pc(0);
	vm_set_sp(VM_MAX_STACK_SIZE -1);		//stack pointer
	vm_set_bp(0);		//base pointer
}

#ifndef STANDALONE_INTERPRETER
uint32 vm_fetch(VM_DEF_ARG, uint32 offset, uchar * buffer, uint32 size) {
	uint32 readed = 0;
	
	memcpy(buffer, _RECAST(uchar *, vm_get_package()) + offset, size);
	return size;
}
#endif

void vm_close(VM_DEF_ARG) {
	if(_ilfile != NULL) {
		fclose(_ilfile);
	}
}
#endif

vm_object * vm_pop_stack_arc(VM_DEF_ARG) {
	vm_object * obj = NULL;
	obj = _RECAST(vm_object *, vm_pop_stack());
	vm_set_stack(NULL);
	if(obj != NULL) {
		obj->mgc_refcount--;
	}
	return obj;
}

void vm_push_argument(VM_DEF_ARG, uint8 size, uint8 * buffer) {
	vm_object * obj;
	obj = vm_create_object(size, buffer);  
	vm_push_stack(obj);
}

void vm_push_argument_object(VM_DEF_ARG, vm_object * obj) {
	vm_push_stack(obj);
	obj->mgc_refcount++;
}

uint8 vm_pop_result(VM_DEF_ARG, uint8 max_len, uint8 * buffer) {
   	vm_object * obj = (vm_object *)vm_pop_stack();
	uint8 size;
	if(obj->len == 0) return 0;
	size = (obj->len < max_len)?obj->len:max_len;
	obj->mgc_refcount--;
	memcpy(buffer, obj->bytes, size);
	return size;
}

vm_variable * vm_variable_new(vm_variable ** root, uint8 type, uint16 length, uint8 * bytes) {
	vm_variable * var;
	vm_variable * iterator = root[0];
	//if(root != _var_root) return NULL;		//only current context could create variable, private accessor
	var = (vm_variable *)malloc(sizeof(vm_variable) + length);
	if(var != NULL) {
		var->mgc = type;
		var->len = length;
		memcpy(var->bytes, bytes, length);
		var->next = NULL;
		if(root[0] == NULL) {
			root[0] = var;
		} else {
			while(iterator->next != NULL) {
				iterator = iterator->next;
			}
			iterator->next = var;
		}
	}
	return var;
}

void vm_variable_release(vm_variable ** root, vm_variable * var) {
	vm_variable * iterator = root[0];
	//if(root != _var_root) return;			//only current context could release variable, private accessor
	if(iterator == var) {
		root[0] = iterator->next;
	} else {
		while(iterator != NULL) {
			if(iterator->next == var) {
				iterator->next = var->next;
				break;
			}
			iterator = iterator->next;
		}
	}
	free(var);
}

void vm_variable_clear(vm_variable ** root) {
	vm_variable * iterator = root[0];
	vm_variable * var;
	while(iterator != NULL) {
		var = iterator;
		iterator = iterator->next;
		free(var);
	}
	root = NULL;
}

//added 2018.01.10
void vm_exec_function(VM_DEF_ARG, vm_function * func) {
	//vm_context vctx;
	uint8 hbuf[4];
	uint8 tag, hlen;
	uint16 header_size = 0;
	uint16 codestart = 0;
	//save current vm_context base stack, PC should already incremented
	//vm_memcpy(&vctx.handle, &_vm_file, sizeof(vf_handle));
	//vctx.offset = _pc;
	//_vm_stacks[_bp++] = _RECAST(void *, _base_address);				//save current base address
	//_vm_stacks[_bp++] = _RECAST(void *, _pc);				//save PC+3, address of next instruction
	vm_push_base(vm_get_package());
	vm_push_base(_RECAST(void *, vm_get_pc()));		
	//_vm_stacks[_bp++] = vm_create_object(sizeof(vm_context), &vctx);
	//set new vm_context based on func argument
	//vm_memcpy(&_vm_file, &func->base.handle, sizeof(vf_handle));
	//only for terminal (because each bytecodes file are stored with their respectual header codes)
#if 1			
	//hlen = vf_pop_handle(&_vm_file, 0, &tag, &header_size);
	//codestart = header_size + hlen;										//total header length
#endif
	//_pc = func->offset + codestart;
	vm_set_pc(func->offset + codestart);
	//g_recursive_exec = TRUE;
	vm_set_rexec(TRUE);
}

pk_method * vm_load_method(vm_object * clsobj, vm_object * mthobj) {
	uchar * buffer;
	pk_class * pkc = NULL;
	pk_method * pkm = NULL;
	buffer = _RECAST(uchar *, m_alloc(clsobj->len + 1));
	memset(buffer, 0, clsobj->len + 1);
	memcpy(buffer, clsobj->bytes, clsobj->len);
	printf("select : %s\n", buffer);
	pkc = lk_select_class(buffer);
	
	m_free(buffer);
	if(pkc == NULL) { return NULL; }
	//printf("class loaded");
	buffer = _RECAST(uchar *, m_alloc(mthobj->len + 1));
	memset(buffer, 0, mthobj->len + 1);
	memcpy(buffer, mthobj->bytes, mthobj->len);
	//printf("method : %s\n", buffer);
	pkm = lk_select_method(pkc, buffer);
	m_free(buffer);
	//if(pkm != NULL) printf("method loaded");
	return pkm;
}

vm_object * vm_load_bool(uchar value) {
	vm_object * obj = NULL;
	if(value) {
		obj = _RECAST(vm_object *, vm_create_object(4, _RECAST(uchar *, "true")));		//create new object (clone object)
	} else {
		obj = _RECAST(vm_object *, vm_create_object(5, _RECAST(uchar *, "false")));		//create new object (clone object)
	}
	return obj;
}

vm_object * vm_load_constant(VM_DEF_ARG, uint32 offset) {
	vm_object * obj = NULL;
	uint8 llen;
	uint8 lbuf[4];
	uint16 len = 0;
	uchar * buffer = NULL;
	vm_fetch(VM_ARG, offset, lbuf, 4);
	if(lbuf[0] < 128) { len = lbuf[0]; llen = 1; }
	else {
		switch(lbuf[0] & 0x0F) {
			case 0x01: len = lbuf[1]; llen = 2; break;
			case 0x02: len = (lbuf[1] << 8) | lbuf[2]; llen = 3; break;
			default: break;
		}
	}
	if(len > VA_OBJECT_MAX_SIZE) {
		vm_invoke_exception(VM_ARG, VX_OUT_OF_BOUNDS);
		return VM_NULL_OBJECT;
	}
	buffer = _RECAST(uchar *, m_alloc(len + 1));
	buffer[len] = 0;
	vm_fetch(VM_ARG, offset + llen, buffer, len);		//obj = jumptable
	printf("constant : %s\n", buffer);
	obj = _RECAST(vm_object *, vm_create_object(len, buffer));
	obj->mgc_refcount--;
	m_free(buffer);
	return obj;
}

uint8 vm_object_get_type(vm_object * obj) {
	return (obj->mgc_refcount & VM_MAGIC_MASK);
}

uint16 vm_object_get_text(vm_object * obj, uint8 * text) {
	uint16 len = 0;
	switch(vm_object_get_type(obj)) {
		case VM_MAGIC:
			len = obj->len;
			memcpy(text, obj->bytes, len);
			break;
		case VM_EXT_MAGIC:
			len = ((vm_extension *)obj->bytes)->apis->text(obj, text);
			break;
	}
	return len;
}

uint16 vm_object_get_length(vm_object * obj) {
	uint16 len = 0;
	uint8 buffer[64];
	switch(vm_object_get_type(obj)) {
		case VM_MAGIC:
			len = obj->len;
			break;
		case VM_EXT_MAGIC:
			len = ((vm_extension *)obj->bytes)->apis->text(obj, buffer);
			break;
	}
	return len;
}

vm_object * vm_create_object(uint16 length, uchar * bytes) {
	vm_object * newobj = (vm_object *)m_alloc(sizeof(vm_object) + length);
	newobj->mgc_refcount = (VM_MAGIC | 1);
	//newobj->mgc_refcount = 1;
	newobj->len = length;
	if(bytes != NULL) memcpy(newobj->bytes, bytes, length);
	return newobj;
}

uint8 vm_ext_get_tag(vm_object * obj) {
	if(vm_object_get_type(obj) != VM_EXT_MAGIC) return ASN_TAG_OCTSTRING;
	return ((vm_extension *)(obj->bytes))->tag;
}

vm_object * vm_create_extension(uint8 tag, vm_custom_opcode * apis, uint16 length, uchar * bytes) {
	vm_object * newobj = (vm_object *)m_alloc(sizeof(vm_object) + sizeof(vm_extension) + length);
	newobj->mgc_refcount = (VM_EXT_MAGIC | 1);
	newobj->len = sizeof(vm_extension) + length;
	((vm_extension *)(newobj->bytes))->tag = tag;
	((vm_extension *)(newobj->bytes))->apis = apis;
	if(bytes != NULL) memcpy(((vm_extension *)(newobj->bytes))->payload, bytes, length);
	else memset(((vm_extension *)(newobj->bytes))->payload, 0, length);
	return newobj;
}

void vm_release_object(vm_object * obj) {
	if(obj == NULL) return;
	if(obj->mgc_refcount == 0) {
		m_free(obj);
	}
}

vm_object * vm_size_object(vm_object * op1) {
	uchar objlen = op1->len;
	uchar buffer[5];
	sprintf(_RECAST(char *, buffer), "%i", objlen);
	return vm_create_object(strlen(_RECAST(const char *, buffer)), buffer);
}

uchar vm_is_numeric(uchar * buffer, uint16 len) { 
	uchar n;
	if(len == 0) return FALSE;
	if(len > 11) return FALSE;
	while(len != 0 ) {
		n = buffer[--len];
		if((n == '-') && (len == 0)) break;
		if(n > 0x39 || n < 0x30) { return FALSE; }
	} 
	return TRUE;
}

vm_object * vm_split_object(vm_object * op2, vm_object * op1, vm_object * target) {
	uchar offset, len;
	vm_object * obj;
	uchar * opd1, * opd2;
	opd1 = _RECAST(uchar *, m_alloc(op1->len + 1));
	opd2 = _RECAST(uchar *, m_alloc(op2->len + 1));
	memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;		//null terminated string
	memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;		//null terminated string
	if(vm_is_numeric(op1->bytes, op1->len) == FALSE) { offset = 0; } else { offset = atoi(_RECAST(const char *, opd1)); }			//
	if(vm_is_numeric(op2->bytes, op2->len) == FALSE) { len = target->len; } else { len = atoi(_RECAST(const char *, opd2)); }
	if(len > (target->len - offset)) len = (target->len - offset);
	m_free(opd1);
	m_free(opd2);//
	obj = vm_create_object(len, target->bytes + offset);
	//printf("new size : %d, address : %08x\n", m_size_chunk(obj), obj);
	//memcpy(obj->bytes, target->bytes + offset, len);
	//printf("address : %08x\n", obj);
	return obj;
}

vm_object * vm_operation_object(VM_DEF_ARG, uchar opcode, vm_object * op2, vm_object * op1) {
	int32 value1, value2;
	uint8 valbuf1[64];
	uint8 valbuf2[64];
	uint8 vb1len;
	uint8 vb2len;
	uint16 vb2offset;
	uint16 len;
	uchar * buffer = NULL;
	uchar * obj;
	if(vm_object_get_type(op1) == VM_EXT_MAGIC && vm_object_get_type(op2) == VM_EXT_MAGIC) {
		switch(opcode) {
			case INS_ADD: obj = (uchar *)((vm_extension *)op1->bytes)->apis->add(op1, op2); break;
			case INS_SUB: obj = (uchar *)((vm_extension *)op1->bytes)->apis->sub(op1, op2); break;
			case INS_MUL: obj = (uchar *)((vm_extension *)op1->bytes)->apis->mul(op1, op2); break;
			case INS_DIV: obj = (uchar *)((vm_extension *)op1->bytes)->apis->div(op1, op2); break;
			case INS_AND: obj = (uchar *)((vm_extension *)op1->bytes)->apis->and(op1, op2); break;
			case INS_OR: obj = (uchar *)((vm_extension *)op1->bytes)->apis->or(op1, op2); break;
			case INS_XOR: obj = (uchar *)((vm_extension *)op1->bytes)->apis->xor(op1, op2); break;
			case INS_NOT: obj = (uchar *)((vm_extension *)op1->bytes)->apis->not(op1); break;
			default: break;
		}
	}
	else if(vm_is_numeric(op1->bytes, op1->len) == TRUE && vm_is_numeric(op2->bytes, op2->len) == TRUE) {		//number
		buffer = _RECAST(uchar *, m_alloc(12));
		memcpy(buffer, op1->bytes, op1->len);
		buffer[op1->len] = 0;
		value1 = atoi(_RECAST(const char *, buffer));
		memcpy(buffer, op2->bytes, op2->len);
		buffer[op2->len] = 0;
		value2 = atoi(_RECAST(const char *, buffer));
		switch(opcode) {
			case INS_ADD: value1 = value1 + value2; break;
			case INS_SUB: value1 = value1 - value2; break;
			case INS_MUL: value1 = value1 * value2; break;
			case INS_DIV: 
				if(value2 == 0) {
					vm_invoke_exception(VM_ARG, VX_DIV_BY_ZERO);
					break;
				}
				value1 = value1 / value2; 
				break;
			//logical operation
			case INS_AND: value1 = value1 & value2; break;
			case INS_OR: value1 = value1 | value2; break;
			case INS_XOR: value1 = value1 ^ value2; break;
			case INS_NOT: value1 = !value1; break;
			default: break;
		}
		sprintf(_RECAST(char *, buffer), "%i", value1);
		obj = _RECAST(uchar *, vm_create_object(strlen(_RECAST(const char *, buffer)), buffer));
		m_free(buffer);
	} else {		//string
		switch(opcode) {
			case INS_ADD:
				len = 0;
				vb1len = 0;
				vb2len = 0;
				if(vm_object_get_type(op1) == VM_EXT_MAGIC) {
					vb1len = ((vm_extension *)op1->bytes)->apis->text(op1, valbuf1);
					len += vb1len;
				} else { len += op1->len; }
				vb2offset = len;
				if(vm_object_get_type(op2) == VM_EXT_MAGIC) {
					vb2len = ((vm_extension *)op2->bytes)->apis->text(op2, valbuf2);
					len += vb2len;
				} else { len += op2->len; }
				obj = _RECAST(uchar *, vm_create_object(len, buffer));
				if(vb1len == 0) memcpy(((vm_object *)obj)->bytes, op1->bytes, op1->len); 
				else memcpy(((vm_object *)obj)->bytes, valbuf1, vb1len); 
				if(vb2len == 0) memcpy(((vm_object *)obj)->bytes + vb2offset, op2->bytes, op2->len); 
				else memcpy(((vm_object *)obj)->bytes + vb2offset, valbuf2, vb2len); 
				break;
			default:
				obj = _RECAST(uchar *, vm_create_object(3, _RECAST(uchar *, "NaN")));
				break;
		}
	}
	return _RECAST(vm_object *, obj);
}

uchar vm_cmp_object(vm_object * op2, vm_object * op1) {
	int32 value1, value2;
	uchar * buffer = NULL;
	uchar * obj; 
	uchar opd1[12];
	uchar opd2[12];
	//printf("%s - %s\n", op2->bytes, op1->bytes);
	if(vm_is_numeric(op1->bytes, op1->len) == TRUE && vm_is_numeric(op2->bytes, op2->len) == TRUE) {		//number
		
		memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;
		memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;
		value1 = atoi(_RECAST(const char *, opd1));
		value2 = atoi(_RECAST(const char *, opd2));
		//printf("%d - %d\n", value1, value2);
		value1 = value1 - value2;
	} else {		//string
		value1 = memcmp(op1->bytes, op2->bytes, ((op1->len > op2->len)? op1->len: op2->len));
	}
	if(value1 == 0) {
		return VM_CMP_EQ;
	} else if(value1 < 0) {
		return VM_CMP_LT;
	} else {
		return VM_CMP_GT;
	}
}

#define VM_GC_DEBUG		0
void vm_garbage_collect(VM_DEF_ARG) _REENTRANT_ {									//reference counting garbage collector
	vm_object * candidate = NULL;
#if VM_GC_DEBUG
	uint16 a_heap, b_heap, i = 0;
#endif
	vm_object * nextchunk = NULL;
	vm_object * shifted = NULL;
	vm_object * iterator = (vm_object *) m_first_chunk();
#if VM_GC_DEBUG
	b_heap = m_get_allocated_space();
#endif
	while(iterator != NULL) {
		if(iterator->mgc_refcount == VM_MAGIC_MASK_ZERO) {				//check vm_object reference counter == 0
			candidate = iterator;
		}
		nextchunk = (vm_object *)m_next_chunk(iterator);
		if(nextchunk == NULL) break;
		if((nextchunk->mgc_refcount & VM_MAGIC_MASK_ZERO) == VM_MAGIC) { 		//check vm_object for shifting
			shifted = _RECAST(vm_object *, m_shift_next_chunk(iterator));
			if(shifted != nextchunk) {
				vm_update_mutator(VM_ARG, nextchunk, shifted);
#if VM_GC_DEBUG
				//printf("shift chunk %08x to %08x\n", nextchunk, shifted);
#endif
				nextchunk = shifted;
			}
		}
		iterator = nextchunk;
		if(candidate != NULL) {
#if VM_GC_DEBUG 
			i++;
#endif
			//printf("[GarbageCollector] : clear an object %d bytes, %s\n", m_size_chunk(candidate), candidate->bytes);
			memset(candidate, 0, m_size_chunk(candidate));
			m_free(candidate);
			candidate = NULL;
		}
	}
#if VM_GC_DEBUG
	a_heap = m_get_allocated_space();
	if(i != 0) {
		printf("freed up %i variables, heap : %d -> %d bytes\n", i, b_heap, a_heap);
	}
#endif

}

void vm_garbage_collect2(VM_DEF_ARG) {									//reference counting garbage collector
	vm_object * candidate = NULL;
#if VM_GC_DEBUG
	uint16 a_heap, b_heap, i = 0;
#endif
	vm_object * nextchunk = NULL;
	vm_object * shifted = NULL;
	vm_object * iterator = _RECAST(vm_object *, m_first_chunk());
#if VM_GC_DEBUG
	b_heap = m_get_allocated_space();
#endif
	while(iterator != NULL) {
		if(iterator->mgc_refcount == VM_MAGIC) {				//check vm_object reference counter == 0
			candidate = iterator;
		}
		nextchunk = _RECAST(vm_object *, m_next_chunk(iterator));
		if((iterator->mgc_refcount & VM_MAGIC_MASK) == VM_MAGIC) { 		//check vm_object for shifting
			shifted = _RECAST(vm_object *, m_shift_next_chunk(iterator));
			if(shifted != nextchunk) {
				vm_update_mutator(VM_ARG, nextchunk, shifted);
#if VM_GC_DEBUG
				printf("shift chunk %08x to %08x\n", nextchunk, shifted);
#endif
				nextchunk = shifted;
			}
		}
		iterator = nextchunk;
		if(candidate != NULL) {
#if VM_GC_DEBUG 
			i++;
#endif
			//printf("[GarbageCollector] : clear an object %d bytes, %s\n", m_size_chunk(candidate), candidate->bytes);
			memset(candidate, 0, m_size_chunk(candidate));
			m_free(candidate);
			candidate = NULL;
		}
	}
#if VM_GC_DEBUG
	a_heap = m_get_allocated_space();
	if(i != 0) {
		printf("freed up %i variables, heap : %d -> %d bytes\n", i, b_heap, a_heap);
	}
#endif

}

void vm_update_mutator(VM_DEF_ARG, vm_object * old_addr, vm_object * new_addr) {									//reallign all objects on heap
	uint16 i = 0;
	for(i =0 ;i<VM_MAX_STACK_SIZE; i++) {
		if((vm_object *)vm_stack(i) == old_addr) vm_stack(i) = new_addr;
	}
}

vm_object * vm_get_argument(VM_DEF_ARG, uchar index) {
	vm_object * obj = NULL;
	uint16 base_arg = vm_get_sp() + vm_get_sysc().num_of_params;
	if(vm_get_sp() <= (base_arg - index)) {
		obj = _RECAST(vm_object *, vm_stack((base_arg - index)));
	}
	return obj;
}

uchar vm_get_argument_count(VM_DEF_ARG) {
	return vm_get_sysc().num_of_params;
}

#define VTXT_INSUFFICIENT_HEAP	 		"\x04Insufficient Heap"
#define VTXT_STACK_OVERFLOW				"\x04Stack Overflow"
#define VTXT_UNKNOWN_INSTRUCTION		"\x04Unknown Instruction"
#define VTXT_UNIMPLEMENTED_APIS			"\x04Unimplemented APIs"
#define VTXT_SYSTEM_EXCEPTION			"\x04System Exception" 
#define VTXT_STACK_UNDERFLOW			"\x04Stack Underflow"
#define VTXT_OUT_OF_BOUNDS				"\x04Out of Bounds"	 
#define VTXT_UNRESOLVED_CLASS			"\x04Unresolved Class"
#define VTXT_UNRESOLVED_METHOD			"\x04Unresolved Method"
#define VTXT_ARGUMENT_MISMATCH			"\x04Argument Mismatch"
#define VTXT_INVALID_CONTEXT			"\x04Invalid Context"
#define VTXT_DIVIDE_BY_ZERO				"\x04Divide by Zero"

static uchar * _vm_exception_text[] = {	
	_RECAST(uchar *, VTXT_SYSTEM_EXCEPTION), 
	_RECAST(uchar *, VTXT_UNIMPLEMENTED_APIS), 
	_RECAST(uchar *, VTXT_UNKNOWN_INSTRUCTION), 
	_RECAST(uchar *, VTXT_STACK_OVERFLOW),
  	_RECAST(uchar *, VTXT_INSUFFICIENT_HEAP), 
	_RECAST(uchar *, VTXT_STACK_UNDERFLOW),
	_RECAST(uchar *, VTXT_OUT_OF_BOUNDS),
	_RECAST(uchar *, VTXT_UNRESOLVED_CLASS),
	_RECAST(uchar *, VTXT_UNRESOLVED_METHOD),
	_RECAST(uchar *, VTXT_ARGUMENT_MISMATCH),
	_RECAST(uchar *, VTXT_INVALID_CONTEXT),
	_RECAST(uchar *, VTXT_DIVIDE_BY_ZERO),
};	

void vm_set_state(VM_DEF_ARG, uchar state) {
 	vm_get_state() = state;
}

vm_object * vm_syscall(VM_DEF_ARG, uchar api_id) _REENTRANT_ {
	//BYTE i;
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	vm_set_cur_api(api_id);
	while(iterator->entry != NULL) {
	 	if(iterator->id == api_id) {
			vm_set_retval((vm_object *)vm_create_object(0, NULL));
		 	iterator->entry(VM_ARG);
			if(vm_get_retval() == NULL) {
				vm_set_retval((vm_object *)vm_create_object(0, NULL));
			}
			return vm_get_retval();
		}
		iterator++;
	} 
	vm_invoke_exception(VM_ARG, VX_UNIMPLEMENTED_APIS);
}

void vm_invoke_exception(VM_DEF_ARG, uchar excp) { 
	uchar len;
	uchar i = 0, j;
	//i = tkPrintf("cd", STK_CMD_DISPLAY_TEXT, 0x81, STK_DEV_DISPLAY);
	//len = strlen(_vm_exception_text[excp]);
	//i += tkPushBufferB(i, STK_TAG_TEXT_STRING, len, _vm_exception_text[excp]);
	//tkDispatchCommandW(NULL, i); 
	printf("Exception : %s\r\n", _vm_exception_text[excp] + 1);
	//_vm_state = VM_STATE_EXCEPTION;
	vm_set_state(VM_ARG, VM_STATE_EXCEPTION);
} 

#pragma managed



#ifdef __cplusplus_cli
int StackVirtualMachine::Decode(VM_DEF_ARG) {
	return this->Decode(VM_ARG, vm_get_pc());
}

void StackVirtualMachine::InvokeException(VM_DEF_ARG, int code) { 
	uchar len;
	uchar i = 0, j;
	StackExceptionEventArgs^ excArgs = gcnew StackExceptionEventArgs(vm_get_pc());
	excArgs->Message = gcnew String(_RECAST(const char *, _vm_exception_text[code]));
	this->ExceptionCallback(this, excArgs);
	//_vm_state = VM_STATE_EXCEPTION;
	vm_set_state(VM_ARG, VM_STATE_EXCEPTION);
} 

int StackVirtualMachine::Decode(VM_DEF_ARG, int offset) {
	StackApiEventArgs^ apiArgs;
	StackExceptionEventArgs^ evtArgs;
	StackMethodEventArgs^ mtdArgs;
	StackFetchEventArgs^ fchArgs;
	pin_ptr<System::Byte> ptr;
#else

void vm_decode(VM_DEF_ARG, uint32 offset, uchar num_args, ...) {
	va_list argptr;
#endif
	#if VM_CODEDEBUG
	uchar dbgbuf[2048];
	uchar varname[256];
	#endif
	uchar ibuf[3];
	uchar opcode;
	uchar psw = 0;
	uint16 index = 0;
	vm_object * obj;
	vm_object * op1, * op2;
	void * new_base_addr = vm_get_package();
	vm_set_pc(offset);
#ifndef __cplusplus_cli
	va_start(argptr, num_args);						//start variadic arguments
	for(index=0; index< num_args; index++) {
		obj = va_arg(argptr, vm_object *);
		vm_push_stack(obj);							//push to argument stack (VM function call)
	}
	va_end(argptr);              					//end of variadic arguments
#endif						
	vm_push_base(_RECAST(vm_object *, 0x0000));				//push 0xFFFF maximum PC address to base pointer for ret
	vm_push_base(_RECAST(vm_object *, 0xFFFF));				//push 0xFFFF maximum PC address to base pointer for ret

	//load program->main()
#ifdef __cplusplus_cli
	vm_set_pc(offset);
#else
	obj = _RECAST(vm_object *, vm_load_method(vm_create_object(sizeof("program"), _RECAST(uchar *, "program")), vm_create_object(sizeof("main?0"), _RECAST(uchar *, "main?0"))));
	if(obj == NULL) { printf("extcall exception!!"); vm_set_pc(0xFFFF); return; }
	//printf("load method : %d\n", end_swap16(((pk_method *)obj)->offset));
	vm_set_pc(((pk_method *)obj)->offset);
	*((uint16 * )(ibuf + 1)) = end_swap16(index);
	new_base_addr = ((pk_object *)obj)->codebase;
	//printf("offset : %x\n", _pc);
	vm_set_package(new_base_addr);
#endif

#ifdef __cplusplus_cli
	if(vm_get_state() == VM_STATE_RUN) {
#else
	if(vm_get_state() != VM_STATE_EXCEPTION) vm_set_state(VM_ARG, VM_STATE_RUN);
	while(vm_get_state() == VM_STATE_RUN) {   
#endif
		vm_fetch(VM_ARG, vm_get_pc(), ibuf, 3);
		opcode = ibuf[0];		
		vm_set_rexec(FALSE);	//set recursive_exec flag to false
		//printf("fetch ok %02x\n", opcode);
		switch(opcode) {
			case INS_LBL:					//create new label
				break;
			case INS_OBJPUSH:
				index = ibuf[1];
				index += 1;
				obj = _RECAST(vm_object *, vm_get_base(index));
				if(obj != NULL) {
					obj->mgc_refcount ++;
				}
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				//printf("push ok\n");
				memset(dbgbuf, 0, sizeof(dbgbuf));
				sprintf(dbgbuf, "\t\t%s ", _ins_name[opcode]);
				memcpy(dbgbuf + strlen(dbgbuf), obj->bytes, obj->len);
				//sprintf(dbgbuf, "\t\t%s %s", _ins_name[opcode], obj->bytes);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(2);
				break;
			case INS_OBJSTORE:
				index = ibuf[1];
				index += 1;
				obj = _RECAST(vm_object *, vm_stack(vm_get_sp() + 1));
				goto vmd_store_variable;
			case INS_OBJPOP:
				index = ibuf[1];
				index += 1;
				obj = vm_pop_stack_arc(VM_ARG);
				vmd_store_variable:
				if(vm_get_base(index) != obj) {
					op1 = _RECAST(vm_object *,  vm_get_base(index));
					if(op1 != NULL) {
						op1->mgc_refcount--;
					}
					vm_set_base(index, obj);
					if(obj != NULL) {
						obj->mgc_refcount++;
					}
				}
				#if VM_CODEDEBUG
				memset(dbgbuf, 0, sizeof(dbgbuf));
				sprintf(dbgbuf, "\t\t%s ", _ins_name[opcode]);
				memcpy(dbgbuf + strlen(dbgbuf), obj->bytes, obj->len);
				//sprintf(dbgbuf, "\t\t%s %s", _ins_name[opcode], obj->bytes);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(2);
				break;
			case INS_SWITCH:
				obj = vm_pop_stack_arc(VM_ARG);
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				op1 = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else	
				op1 = vm_load_constant(VM_ARG, index);
				#endif
				//jumptable
				for(psw =2; psw < op1->len; psw += 4) {
					index = end_swap16(*((uint16 * )(op1->bytes + psw)));
					#if IS_REL_JUMP_ADDRESS
					op2 = vm_load_constant(VM_ARG, index + vm_get_pc());
					#else
					op2 = vm_load_constant(VM_ARG, index);
					#endif
					//printf("%s\n", op2->bytes);
					if(vm_cmp_object(obj, op2) == VM_CMP_EQ) {			//object match found
						index = end_swap16(*((uint16 * )(op1->bytes + psw + 2)));
						vm_release_object(op2);							//release constant object
						goto start_jump;
					}
					vm_release_object(op2);								//release constant object
				}	
				//default index
				index = end_swap16(*((uint16 * )(op1->bytes)));			//default jump offset
				start_jump:
				vm_release_object(op1);									//release jump table
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %i", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				//printf("next pc %08x\n", index + _pc);
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				break;
			case INS_OBJDEL:
				obj = vm_pop_stack_arc(VM_ARG);
				vm_release_object(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_OBJSUB:
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_pop_stack_arc(VM_ARG);
				obj = vm_split_object(op2, op1, obj);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_OBJSZ:
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_size_object(op1);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_OBJDUP:
				op1 = _RECAST(vm_object *, vm_stack(vm_get_sp() + 1));
				obj = vm_create_object(op1->len, op1->bytes);		//create new object (clone object)
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %s", _ins_name[opcode], obj->bytes);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_ADD:
			case INS_MUL:
			case INS_DIV:
			case INS_SUB:
			case INS_AND:
			case INS_OR:
			case INS_XOR:
				op2 = vm_pop_stack_arc(VM_ARG);
			case INS_NOT:
				op1 = vm_pop_stack_arc(VM_ARG);
				obj = vm_operation_object(VM_ARG, opcode, op2, op1);
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;
			case INS_SYSCALL0:
			case INS_SYSCALL1:
			case INS_SYSCALL2:
			case INS_SYSCALL3:
			case INS_SYSCALL4:
			case INS_SYSCALL5:
			case INS_SYSCALL6:
			case INS_SYSCALL7:
			case INS_SYSCALL8:
			case INS_SYSCALL9:
			case INS_SYSCALL10:
			case INS_SYSCALL11:
			case INS_SYSCALL12:
			case INS_SYSCALL13:
			case INS_SYSCALL14:
			case INS_SYSCALL15:
				vm_get_sysc().num_of_params = (opcode - INS_SYSCALL0);		//initialize syscall context
				
#if __cplusplus_cli
				apiArgs = gcnew StackApiEventArgs(vm_get_pc());
				apiArgs->Arguments = gcnew array<array<Byte>^>(vm_get_sysc().num_of_params);
				//pop argument stack
				for(index = 0;index < vm_get_sysc().num_of_params;index ++) {
					vm_object * varg = vm_pop_stack_arc(VM_ARG);
					array<System::Byte>^ temp = gcnew array<System::Byte>(varg->len);
					ptr = &temp[0];
					memcpy(_RECAST(uchar *, ptr), varg->bytes, varg->len);
					apiArgs->Arguments[index] = temp;
				}
				
				this->ApiCallback(this, apiArgs);
				if(apiArgs->ReturnedValue != nullptr) {
					pin_ptr<System::Byte> rptr = &apiArgs->ReturnedValue[0];
					obj = vm_create_object(apiArgs->ReturnedValue->Length, rptr);
				} else {
					obj = NULL;
				}
#else
				if(ibuf[1] < 254) {		  	//basic API
					vm_add_pc(2);
					obj = vm_syscall(VM_ARG, ibuf[1]);
					//pop argument stack
					if(vm_get_rexec() == FALSE) {
						for(index = 0;index < vm_get_sysc().num_of_params;index ++) {
							vm_pop_stack_arc(VM_ARG);
						}
						if(vm_get_state() == VM_STATE_RUN) {
							vm_push_stack(obj);
						}
					}
				} else {					//extended API  
					vm_add_pc(3);
					vm_push_stack(vm_create_object(0, NULL));
				}
#endif
				
				//_vm_stacks[_sp--] = obj;
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %d", _ins_name[opcode], ibuf[1]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				//_pc += 2;
				break;
			//F2O support argument count and vm_context (added 2018.01.10)
			case INS_F2O_0:			//96	//-> function to object
			case INS_F2O_1:			//97	//-> function to object
			case INS_F2O_2:			//98	//-> function to object
			case INS_F2O_3:			//99	//-> function to object
			case INS_F2O_4:			//100	//-> function to object
			case INS_F2O_5:			//101	//-> function to object
			case INS_F2O_6:			//102	//-> function to object
			case INS_F2O_7:			//103	//-> function to object
			case INS_F2O_8:			//104	//-> function to object
			case INS_F2O_9:			//105	//-> function to object
			case INS_F2O_10:			//106	//-> function to object
			case INS_F2O_11:			//107	//-> function to object
			case INS_F2O_12:			//108	//-> function to object
			case INS_F2O_13:			//109	//-> function to object
			case INS_F2O_14:			//110	//-> function to object
			case INS_F2O_15:			//111	//-> function to object
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				obj = vm_create_object(sizeof(vm_function), NULL);				//copy current execution handle (vm_context)
				//obj->mgc_refcount |= VM_OBJ_MAGIC;
				((vm_function *)obj->bytes)->arg_count = (opcode - INS_F2O_0);		//set argument count
				((vm_function *)obj->bytes)->offset = index;									//set function offset
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(3);
				break;
			case INS_SCTX:
				//increment base pointer
				index = ibuf[1];
				for(psw = 0; psw < index; psw ++) {
					vm_push_base(vm_create_object(0, NULL));
				}
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %d", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(2);
				break;
			case INS_RCTX:
				//_bp -= ibuf[1];			//increment base pointer
				index = ibuf[1];
				for(psw = 0; psw < index; psw ++) {
					obj = _RECAST(vm_object *, vm_pop_base());
					vm_stack(vm_get_bp()) = vm_create_object(0, NULL);
					if(obj != NULL) obj->mgc_refcount --;
				}
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %d", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(2);
				break;
			case INS_RET:
				index = _RECAST(uint16, vm_pop_base());
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s", _ins_name[opcode]);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_set_pc(index);
				vm_set_package(vm_pop_base());
				new_base_addr = vm_get_package();
				break;
			case INS_EXTCALL0:
			case INS_EXTCALL1:
			case INS_EXTCALL2:
			case INS_EXTCALL3:
			case INS_EXTCALL4:
			case INS_EXTCALL5:
			case INS_EXTCALL6:
			case INS_EXTCALL7:
			case INS_EXTCALL8:
			case INS_EXTCALL9:
			case INS_EXTCALL10:
			case INS_EXTCALL11:
			case INS_EXTCALL12:
			case INS_EXTCALL13:
			case INS_EXTCALL14:
			case INS_EXTCALL15:
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				op1 = vm_pop_stack_arc(VM_ARG);		//class instance
				#if IS_REL_JUMP_ADDRESS
				obj = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				obj = vm_load_constant(VM_ARG, index);
				#endif			
#if __cplusplus_cli
				mtdArgs = gcnew StackMethodEventArgs();
				mtdArgs->ClassName = gcnew String(_RECAST(const char *, op1->bytes), 0, op1->len);
				mtdArgs->MethodName = gcnew String(_RECAST(const char *, obj->bytes), 0, obj->len);
				
				this->LoadMethodCallback(this, mtdArgs);
				if(mtdArgs->MethodBase == nullptr) { 
					this->InvokeException(VM_ARG, VX_UNIMPLEMENTED_APIS);
					vm_set_pc(0xFFFF);
					break; 
				}
				ptr = &mtdArgs->MethodBase[0];
				index = mtdArgs->MethodOffset;
				index -= vm_get_pc();
				*((uint16 * )(ibuf + 1)) = end_swap16(index);
				new_base_addr = _RECAST(void *, ptr);
				
#else
				obj = (vm_object *)vm_load_method(op1, obj);
				if(obj == NULL) { printf("extcall exception!!"); vm_set_pc(0xFFFF); break; }
				index = ((pk_method *)obj)->offset;
				index -= vm_get_pc();
				*((uint16 * )(ibuf + 1)) = end_swap16(index);
				new_base_addr = ((pk_object *)obj)->codebase;
#endif
				//printf("index : %d\n", ((pk_method *)obj)->offset);
				//_pc = 0;
			case INS_CALL:
				//printf("call function\n");
				vm_push_base(_RECAST(void *, vm_get_package()));					//save current base address
				vm_push_base(_RECAST(void *, (vm_get_pc() + 3)));				//save PC+3, address of next instruction
			case INS_JMP:									//jump to specified label
				vm_set_package(new_base_addr);
				//printf("base %d\n", _base_address);
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				//printf("PC: %d\n", _pc);
				break;
			case INS_OBJNEW: 
			case INS_OBJCONST:
				index = end_swap16(*((uint16 * )(ibuf + 1)));
				#if IS_REL_JUMP_ADDRESS
				obj = vm_load_constant(VM_ARG, index + vm_get_pc());
				#else
				obj = vm_load_constant(VM_ARG, index);
				#endif
				obj->mgc_refcount++;	
				vm_push_stack(obj);	
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %s", _ins_name[opcode], obj->bytes);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif		
				vm_add_pc(3);
				break;
			case INS_JFALSE:
				op1 = vm_pop_stack_arc(VM_ARG);
				#if IS_REL_JUMP_ADDRESS
				index = 3;
				#else
				index = (vm_get_pc() + 3);
				#endif
				if(memcmp(op1->bytes, "false", op1->len) == 0) {
					index = end_swap16(*((uint16 * )(ibuf + 1)));
				}
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				#if IS_REL_JUMP_ADDRESS
				vm_set_pc(index + vm_get_pc());
				#else
				vm_set_pc(index);
				#endif
				break;
			case INS_CREQ:		//64	//-> jump if equal (relative to pc)
			case INS_CRNE:			//65	//-> jump not equal (relative to pc)
			case INS_CRGT:			//66	//-> jump greater than (relative to pc)
			case INS_CRLT:			//67	//-> jump less than (relative to pc)
			case INS_CRGTEQ:			//68	//-> jump greater than (relative to pc)
			case INS_CRLTEQ:			//69	//-> jump less than (relative to pc)
				op2 = vm_pop_stack_arc(VM_ARG);
				op1 = vm_pop_stack_arc(VM_ARG);
				psw = vm_cmp_object(op2, op1);
				
				switch(opcode) {
					case INS_CREQ: obj = vm_load_bool((psw & VM_CMP_EQ)); break;
					case INS_CRNE: obj = vm_load_bool((psw & VM_CMP_EQ) == 0); break;
					case INS_CRGT: obj = vm_load_bool((psw == VM_CMP_GT)); break;
					case INS_CRLT: obj = vm_load_bool((psw == VM_CMP_LT)); break;
					case INS_CRGTEQ: obj = vm_load_bool((psw != VM_CMP_LT)); break;
					case INS_CRLTEQ: obj = vm_load_bool((psw != VM_CMP_GT)); break;
					default: obj = NULL; break;
				}
				vm_push_stack(obj);
				#if VM_CODEDEBUG
				sprintf(dbgbuf, "\t\t%s %08x", _ins_name[opcode], index);
				vm_debug(vm_get_pc(), dbgbuf);
				#endif
				vm_add_pc(1);
				break;

			default: /* printf("%08x\n", _pc); */			//unknown instruction exception
				//_pc += 1;
				//printf("%08x %02x\n", _pc, opcode); 
#ifdef __cplusplus_cli
				this->InvokeException(VM_ARG, VX_UNKNOWN_INSTRUCTION);
#else
				vm_invoke_exception(VM_ARG, VX_UNKNOWN_INSTRUCTION);
#endif
				break;
		}
		if(vm_get_bp() >= vm_get_sp()) {		//stack overflow exception
#ifdef __cplusplus_cli
				this->InvokeException(VM_ARG, VX_STACK_OVERFLOW);
#else
		  	vm_invoke_exception(VM_ARG, VX_STACK_OVERFLOW);
#endif
		}
		//printf("sp : %d, bp : %d\n", _sp, _bp);
		vm_garbage_collect(VM_ARG);	
#if __cplusplus_cli
		fchArgs = gcnew StackFetchEventArgs(vm_get_pc());
		
		this->FetchCallback(this, fchArgs);
		switch(fchArgs->MachineState) {
			case StackMachineState::RUN: break;
			case StackMachineState::SUSPEND: vm_set_state(VM_ARG, VM_STATE_SUSPEND); break;
			case StackMachineState::EXCEPTION: vm_set_state(VM_ARG, VM_STATE_EXCEPTION); break;
			case StackMachineState::ABORT: vm_set_state(VM_ARG, VM_STATE_ABORT); break;
			default: break;
		}
#endif
		//getch();
		if(vm_get_pc() == 0xFFFF) {	  		//normally terminated
			vm_set_state(VM_ARG, VM_STATE_INIT);
		}
	}
#ifdef __cplusplus_cli
	return vm_get_pc();
#endif
}

void va_sys_exec(VM_DEF_ARG) _REENTRANT_ {
	uint8 arglen, i;
	vm_function * func;
	uint16 base_arg = vm_get_sp() + vm_get_sysc().num_of_params;
	//OS_DEBUG_ENTRY(va_sys_exec);
	if(vm_get_argument_count(VM_ARG) == 0) {
		vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
		goto exit_sys_exec;
	}
	arglen = vm_get_argument_count(VM_ARG) - 1;
	((vm_object *)vm_get_argument(VM_ARG, 0))->mgc_refcount--;
	func = (vm_function *)(vm_get_argument(VM_ARG, 0)->bytes);
	//check function argument with syscall argument
	if(func->arg_count != arglen) {
		vm_invoke_exception(VM_ARG, VX_ARGUMENT_MISMATCH);
		goto exit_sys_exec;
	}
	//shift arguments by 1
	for(i=1;i<vm_get_argument_count(VM_ARG);i++) {
		vm_stack(base_arg) = vm_stack(base_arg - 1)  ;
		base_arg--;
	}
	vm_get_sp() += 1;
	//execute function (recursive execution from syscall)
	vm_exec_function(VM_ARG, func);
	exit_sys_exec:
	//OS_DEBUG_EXIT();
	return;
}

#ifdef STANDALONE_INTERPRETER
void vm_init(VM_DEF_ARG, uchar * exepath, uchar * inpath) {		//original input file path as parameter
	m_init_alloc();
	//memset(_vm_stacks, 0, sizeof(_vm_stacks));
	lk_import_directory("\\packages");
	memset(&vm_get_context(), 0, sizeof(vm_context));
	vm_get_context().var_root = 0;
	vm_get_context().vars = &vm_get_context().var_root;
	vm_set_pc(0);
	vm_set_sp(VM_MAX_STACK_SIZE -1);
	vm_set_bp(0);
}

void vm_close(VM_DEF_ARG) {
	
}
#endif

#ifdef STANDALONE_VIRTUAL_MACHINE 
#include "build_vm.h"
void main( int argc, char *argv[] )
{ 
	extern FILE *yyin;
	int i;
	vm_instance vcit;
	//++argv; --argc;
	if(argc < 2) return;
	vm_init(&vcit, argv[0], argv[1]);
	vm_decode(&vcit, 0, 0);
	vm_close(&vcit);
	getch();
}
#endif
