#include "../defs.h"
#include "../stack/il_streamer.h"
#include "../stack/pkg_encoder.h"
#ifndef _VM_STACK__H

#define MAX_BUFFER_SIZE				65536
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
#define VX_DIV_BY_ZERO					11

#define VM_STATE_INIT		0x00
#define VM_STATE_RUN 		0x01
#define VM_STATE_SUSPEND	0x03
#define VM_STATE_EXCEPTION	0x04
#define VM_STATE_ABORT		0x07

#define VM_MAGIC_MASK		0xF0
#define VM_MAGIC			0x80		//string type
#define VM_OBJ_MAGIC		0xE0		//object type
#define VM_EXT_MAGIC		0xD0		//custom operand
#define VM_MAGIC_MASK_ZERO	(VM_MAGIC | 0x0F)

#define VM_MAX_VAR_NAME				16
#define VA_OBJECT_MAX_SIZE			0x1000

typedef struct vm_object {
	uchar mgc_refcount;		//4(high) magic, 4(low) refcounter
	uint16 len;
	uchar bytes[1];
} vm_object;

typedef struct sys_context {
	uchar num_of_params;
} sys_context;

typedef struct vm_variable {
	uint8 mgc;
	struct vm_variable * next;
	uint8 name[VM_MAX_VAR_NAME];
	uint16 len;
	uint8 bytes[1];
} vm_variable;

typedef struct vm_context {
	vm_variable ** vars;
	uint16 offset;
	vm_variable * var_root;
} vm_context;

typedef struct vm_function {
	uint8 arg_count;
	uint16 offset;
} vm_function;

#define VM_MAX_STACK_SIZE		64
typedef struct vm_instance {
	vm_context base;
	uint16 sp;		//stack pointer
	uint16 bp;		//base pointer
	uint16 pc;
	void * base_address;
	void * vm_stacks[VM_MAX_STACK_SIZE];
	uchar vm_state;
	uchar recursive_exec;		//set recursive_exec flag to false
	uint16 current_api;
	sys_context sysc;
	vm_object * ret;
} vm_instance;

#define VM_DEF_ARG			vm_instance * ctx
#define VM_ARG				ctx

typedef struct vm_api_entry {
	uint8 id;
	char * name;
 	void (* entry)(VM_DEF_ARG); 
 	vm_object * (* exit)(uint8, uint8 *);
} vm_api_entry;

typedef struct vm_custom_opcode {
	uint16 (* text)(vm_object *, uint8 * buffer);
	vm_object * (* add)(vm_object *, vm_object *);
	vm_object * (* mul)(vm_object *, vm_object *);
	vm_object * (* div)(vm_object *, vm_object *);
	vm_object * (* sub)(vm_object *, vm_object *);
	//logical operation
	vm_object * (* and)(vm_object *, vm_object *);
	vm_object * (* or)(vm_object *, vm_object *);
	vm_object * (* xor)(vm_object *, vm_object *);
	vm_object * (* not)(vm_object *);
} vm_custom_opcode;

typedef struct vm_extension {
	uint8 tag;
	vm_custom_opcode * apis;
	uint8 payload[0];
} vm_extension;

typedef sys_context * sys_handle;

#define vm_push_stack(x)	ctx->vm_stacks[ctx->sp--] = x
#define vm_pop_stack()		ctx->vm_stacks[++ctx->sp]
#define vm_push_base(x) 	ctx->vm_stacks[ctx->bp++] = x
#define vm_set_stack(x)		ctx->vm_stacks[ctx->sp] = x
#define vm_get_stack(x)		ctx->vm_stacks[ctx->sp]
#define vm_push_base(x)		ctx->vm_stacks[ctx->bp++] = x
#define vm_pop_base()		ctx->vm_stacks[--ctx->bp]
#define vm_get_base(i)		ctx->vm_stacks[ctx->bp - i]
#define vm_set_base(i,x)	ctx->vm_stacks[ctx->bp - i] = x
#define vm_stack(x)			ctx->vm_stacks[x]
#define vm_get_sp()			ctx->sp
#define vm_set_sp(x)		ctx->sp = x
#define vm_get_pc()			ctx->pc
#define vm_set_pc(x)		ctx->pc = x
#define vm_add_pc(x)		ctx->pc += x
#define vm_get_bp()			ctx->bp
#define vm_set_bp(x)		ctx->bp = x
#define vm_get_package()	ctx->base_address
#define vm_set_package(x)	ctx->base_address = x
#define vm_get_state()		ctx->vm_state
#define vm_get_sysc()		ctx->sysc
#define vm_get_rexec()		ctx->recursive_exec
#define vm_set_rexec(x)		ctx->recursive_exec = x
#define vm_get_context()	ctx->base
#define vm_get_cur_api()	ctx->current_api
#define vm_set_cur_api(x)	ctx->current_api = x
#define vm_get_retval()		ctx->ret
#define vm_set_retval(x)	ctx->ret = x

vm_object * vm_pop_stack_arc(VM_DEF_ARG);
vm_object * vm_load_bool(uchar value);
vm_object * vm_load_constant(VM_DEF_ARG, uint32 offset);
vm_object * vm_create_object(uint16 length, uchar * bytes);
vm_object * vm_create_extension(uint8 tag, vm_custom_opcode * apis, uint16 length, uchar * bytes);
uint8 vm_object_get_type(vm_object * obj);
uint16 vm_object_get_text(vm_object * obj, uint8 * text);
uint16 vm_object_get_length(vm_object * obj);
uint8 vm_ext_get_tag(vm_object * obj);
void vm_release_object(vm_object * obj);
vm_object * vm_size_object(vm_object * op1);
vm_object * vm_split_object(vm_object * op2, vm_object * op1, vm_object * target);
vm_object * vm_operation_object(VM_DEF_ARG, uchar opcode, vm_object * op2, vm_object * op1);
uchar vm_cmp_object(vm_object * op2, vm_object * op1);
pk_method * vm_load_method(vm_object * clsobj, vm_object * mthobj);
//context
vm_context * vm_get_current_context();
vm_variable * vm_variable_new(vm_variable ** root, uint8 type, uint16 length, uint8 * bytes);
void vm_variable_release(vm_variable ** root, vm_variable * var);
void vm_variable_clear(vm_variable ** root);

void vm_garbage_collect(VM_DEF_ARG);												//perform garbage collection (ref counting) with copying collector
void vm_update_mutator(VM_DEF_ARG, vm_object * old_addr, vm_object * new_addr);			//replace mutator pre-allocated address with new allocated address
void vm_invoke_exception(VM_DEF_ARG, uchar excp) ;

//vm ports apis
extern void vm_init(VM_DEF_ARG, uchar * inpath);
extern uint32 vm_fetch(VM_DEF_ARG, uint32 offset, uchar * buffer, uint32 size);
extern void vm_close(VM_DEF_ARG);

//syscall apis
vm_object * vm_get_argument(VM_DEF_ARG, uchar index);
uchar vm_get_argument_count(VM_DEF_ARG);
vm_object * vm_syscall(VM_DEF_ARG, uchar api_id);				//external hook function


#define _VM_STACK__H
#endif
