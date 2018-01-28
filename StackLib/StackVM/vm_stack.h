#include "../defs.h"
#include "../stack/il_streamer.h"
#include "../stack/pkg_encoder.h"
#ifndef _VM_STACK__H

#define MAX_BUFFER_SIZE		65536
//.db	-> data byte
//.dw	-> data word
//virtual machine exception						 
#define VX_SYSTEM_EXCEPTION				0 
#define VX_UNIMPLEMENTED_APIS			1
#define VX_UNKNOWN_INSTRUCTION			2
#define VX_STACK_OVERFLOW				3 
#define VX_INSUFFICIENT_HEAP	 		4
#define VX_STACK_UNDERFLOW				5
#define VX_OUT_OF_BOUNDS				6
#define VX_UNRESOLVED_CLASS				7
#define VX_UNRESOLVED_METHOD			8
#define VX_ARGUMENT_MISMATCH			9
#define VX_INVALID_CONTEXT				10

#define VM_STATE_INIT		0x00
#define VM_STATE_RUN 		0x01
#define VM_STATE_SUSPEND	0x03
#define VM_STATE_EXCEPTION	0x04
#define VM_STATE_ABORT		0x07

#define VM_MAGIC		0xE0
#define VM_OBJ_MAGIC		0xF0

typedef struct vm_object {
	uchar mgc_refcount;		//4(high) magic, 4(low) refcounter
	uchar len;
	uchar bytes[1];
} vm_object;

typedef struct sys_context {
	uchar num_of_params;
} sys_context;

typedef struct vm_function {
	uint8 arg_count;
	uint16 offset;
} vm_function;

typedef struct vm_api_entry {
	uint8 id;
	char * name;
 	void (* entry)(); 
 	vm_object * (* exit)(uint8, uint8 *);
} vm_api_entry;

typedef sys_context * sys_handle;

vm_object * vm_pop_stack(void);
vm_object * vm_load_bool(uchar value);
vm_object * vm_load_constant(uint32 offset);
vm_object * vm_create_object(uchar length, uchar * bytes);
void vm_release_object(vm_object * obj);
vm_object * vm_size_object(vm_object * op1);
vm_object * vm_split_object(vm_object * op2, vm_object * op1, vm_object * target);
vm_object * vm_operation_object(uchar opcode, vm_object * op2, vm_object * op1);
uchar vm_cmp_object(vm_object * op2, vm_object * op1);
pk_method * vm_load_method(vm_object * clsobj, vm_object * mthobj);

void vm_garbage_collect(void);												//perform garbage collection (ref counting) with copying collector
void vm_update_mutator(vm_object * old_addr, vm_object * new_addr);			//replace mutator pre-allocated address with new allocated address
void vm_invoke_exception(uchar excp) ;

//vm ports apis
extern void vm_init(uchar * inpath);
extern uint32 vm_fetch(uchar * codebase, uint32 offset, uchar * buffer, uint32 size);
extern void vm_close(void);

//syscall apis
vm_object * vm_get_argument(uchar index);
uchar vm_get_argument_count();
vm_object * vm_syscall(uchar api_id);				//external hook function

#define _VM_STACK__H
#endif
