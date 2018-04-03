%{
int errors = 0;
int yyerror ( char *s );
/*************************************************************************
Compiler for the Stack language
***************************************************************************/
/*=========================================================================
C Libraries, Symbol Table, Code Generator & other C code
=========================================================================*/
#include <stdio.h> /* For I/O */
#include <stdlib.h> /* For malloc here and in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdarg.h>
#include "token.h"
#include "sem_proto.h"
#include "il_streamer.h"
#include "asm_streamer.h"
#include "lex_proto.h"
#include "scr_generator.h"

#define YYDEBUG 1 /* For Debugging */
#define alloca malloc

%}

%union semrec /* The Semantic Records */
{
	uchar bytes[256];
	uint32 state;
	uint16 api_id;
	yytoken token;
}

/*=========================================================================
TOKENS
=========================================================================*/
%start program
%token VARIABLE		257		//foo, a, index, label		--> variable
%token CONSTANT		258		//"asda", {0,0,0}			--> constant value
%token N_CONSTANT	259
%token ARRAY		260		//bytes array

%token P_VAR	300		//string
%token P_NEW	301		//new
%token P_HEX	302		//array
%token P_SIZEOF	310		//sizeof	

%token P_GOTO		349	//goto
%token P_IF		350		//if
%token P_ELSE		351		//else
%token P_SWITCH	352		//switch
%token P_CASE		353		//case 
%token P_DEFAULT	354
%token P_BREAK		355		//break
%token P_FOR		356
%token P_WHILE		357		//while
%token P_DO		358		//while
%token P_CONTINUE	359


%token P_FUNCTION	360		//function
%token P_RETURN	361		//return
%token P_ALIAS		362		//alias
%token P_EVENT		363		//event
%token P_PROTO		364		//proto
%token P_EXTERN		365		//extern
%token P_SYSCALL	366		//syscall

//third gen capability
%token P_CLASS		380		//class
%token P_INTERFACE	381		//interface
%token P_PACKAGE	382
%token P_IMPORT		383
%token P_PUBLIC		385		//public
%token P_PRIVATE	386		//private
%token P_PROTECTED	387		//protected

%token L_SB		398		//left square bracket
%token R_SB		399		//right square bracket
%token L_CL		400		//left curl
%token R_CL		401		//right curl
%token EOS			402		//semicolon (end of statement)
%token EOC			403		//colon (end of label)
%token L_BR		406		//left bracket
%token R_BR		407		//right bracket
%token T_EQ		410		//type equal
%token T_GT		411		//type greater than
%token T_LT		412		//type less than
%token T_GTEQ		413		//type greater than
%token T_LTEQ		414		//type less than
%token T_NE		415		//type not equal
%token T_AND		416		//type and
%token T_OR		417		//type or


%token S_EQ			450		//assignment
%token S_ADD		451		//+
%token S_SUB		452		//-
%token S_ADDEQ		453		//+=
%token S_SUBEQ		454		//-=
%token S_ADDADD		455		//++
%token S_SUBSUB		456		//--
%token S_MUL		457		//multiplication(*)
%token S_DIV		458		//division(/)
%token S_MULEQ		459		//lazy multiplication
%token S_DIVEQ		460		//lazy division
%token S_MOD		461		//modulo
%token S_MODEQ		462		//lazy modulation
%token S_RNEXT		470		//arrow right
%token S_PDOT		471		//dot

%type<bytes> VARIABLE
%type<bytes> CONSTANT
%type<bytes> N_CONSTANT
%type<token> ARRAY
//%type<bytes> constant_expr
%type<bytes> instance_expr
%type<bytes> instance_decl
%type<bytes> constant_val
%type<state> accessor_func
%type<api_id>	lhs_val
%type<api_id>   rhs_val

%right S_EQ
%left S_SUB S_ADD
%left S_MUL S_DIV S_MOD
/*=========================================================================
GRAMMAR RULES for the Simple language
=========================================================================*/
%%	
/*********************************************** SEMANTIC RULES SHALL BE WRITTEN HERE (LANGUAGE DEPENDENT) ***********************************************/
program: /* empty */ 
| global_decl program 
| global_decl 
;
stmts: /* empty */
| block_stmt stmts
| block_stmt
;
multi_block_stmt: /* empty */
| matched_if_stmt
| open_if_stmt
;
single_block_stmt: /* empty */
| local_decl EOS 
| L_CL { sp_create_new_scope(NULL); } stmts R_CL { sp_destroy_scope(); } 										/* a block consist of several statements */
| while_stmt
| for_stmt
| case_stmt
| stmt_end 												/* a single statement can be considered a single block as long end with semicolon */
| label_decl										
| error_stmt EOS
| EOS { sp_flush_scope(); } 
;
block_stmt: /* empty */
| single_block_stmt
| multi_block_stmt
;
global_decl: /* empty */
| P_IMPORT { } global_import_path EOS { } 
| P_CLASS VARIABLE { sp_start_class_body($2); } L_CL class_stmts R_CL { sp_end_class_body(); }
| P_EXTERN P_FUNCTION sys_func_decl EOS
| error_stmt 
;
class_stmts: class_stmt class_stmts
| class_stmt
;
case_block_stmt: /* empty */
| case_block_stmt case_single_stmt
| case_single_stmt
;
inline_stmt: inline_stmt ',' stmt
| stmt
;
inline_var_decl: inline_var_decl ',' var_decl { sp_pop_symrec(); }
| var_decl
;
case_expr_param: L_BR exprval R_BR
;
expr_params: L_BR exprs R_BR { sp_clear_operation_stack(); sp_lhs_clear(); }
;
body_func_params: L_BR body_func_param_list R_BR								/* params used during function declaration */
;
stmt_end: stmt EOS { sp_flush_scope(); sp_clear_operation_stack(); sp_lhs_clear(); }
;
lazy_stmts:
| lazy_stmt ',' lazy_stmts
| lazy_stmt
;
local_decl: /* empty */									/* only variable declaration */
| P_VAR inline_var_decl { sp_pop_symrec(); sp_clear_operation_stack(); sp_lhs_clear(); }
;
/*********************************************** SEMANTIC ACTION SHALL BE WRITTEN HERE (MACHINE DEPENDENT) ***********************************************/
global_import_path: VARIABLE { sp_push_construct($1); } '.' { sp_push_construct(_RECAST(uchar *, "\\")); } global_import_path
| VARIABLE { sp_push_construct($1); } 
| '*'
;
var_decl: /* empty */
| VARIABLE { sp_push_symrec(sp_create_variable($1)); }																																/* variable declaration without assignment */
| VARIABLE { sp_push_symrec(sp_create_variable($1)); } S_EQ exprval { symrec * rec = sp_peek_symrec(); sp_store_stack(rec->name); sp_clear_operation_stack(); sp_lhs_clear(); } 								/* variable declaration with assignment */
;
label_decl: VARIABLE EOC { sp_new_label($1); }
;
matched_if_stmt: 
| P_IF { sp_push_symrec(sp_create_label(_RECAST(uchar *, "__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *, "__i"))); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } matched_if_stmt  
	else_stmt
	{ symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
| single_block_stmt
;
else_stmt: /* empty */
| P_ELSE 
	{ symrec * rec = sp_pop_symrec(); symrec * rel = sp_create_label(_RECAST(uchar *, "__l")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_new_label(rec->name); } 
	matched_if_stmt
;
open_if_stmt: 
| P_IF { sp_push_symrec(sp_create_label(_RECAST(uchar *, "__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } multi_block_stmt { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
| P_IF { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } matched_if_stmt 
	P_ELSE 
	{ symrec * rec = sp_pop_symrec(); symrec * rel = sp_create_label(_RECAST(uchar *,"__l")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_new_label(rec->name); } 
	open_if_stmt
	{ symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
;
while_stmt: /* empty */
| P_WHILE { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__h"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__w"))); sp_start_loop(); } expr_params { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); } block_stmt 
	{ symrec * rec = sp_pop_symrec(); sp_end_loop(); sp_new_label(rec->name); }
| P_DO { sp_push_symrec(sp_start_loop()); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__w"))); } block_stmt 
	P_WHILE expr_params EOS { symrec * rec = sp_pop_symrec(); sp_pop_symrec(); sp_flush_scope(); sp_new_label(rec->name); }
;
for_stmt: /* empty */
| P_FOR { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__f"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); } 
	for_expr_stmt { symrec * rec = sp_pop_symrec(); rec = sp_pop_symrec(); sp_new_label(rec->name); }
;
for_init_stmt: /* empty */ { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__fv"))); /* dummy variable */ }
| lazy_stmt ',' for_init_stmt { sp_clear_current_scope(); }
| lazy_stmt { sp_clear_current_scope(); }
| P_VAR inline_var_decl { sp_clear_operation_stack(); sp_lhs_clear(); }
;
for_expr_stmt: L_BR for_init_stmt EOS { sp_flush_scope(); sp_clear_current_scope(); sp_pop_symrec(); sp_start_loop(); }
	exprs EOS { symrec * rec = sp_pop_symrec(); sp_flush_scope(); sp_new_label(rec->name); sp_create_new_scope(NULL); }
	lazy_stmts R_BR { sp_clear_current_scope(); sp_create_new_scope(NULL); } block_stmt { sp_merge_scope_left(); sp_end_scope(); sp_end_loop(); /*symrec * rec = sp_peek_symrec(); sp_new_label(rec->name); */ sp_clear_current_scope(); }
;
stmt: /* empty */
| lazy_stmts { sp_pop_symrec(); }
| P_BREAK { symrec * rec = sp_peek_symrec(); sp_jump_to(INS_JMP, rec->name); }
| P_CONTINUE { sp_continue_loop(); }
| P_RETURN { sp_exit_function_body(); }
| P_RETURN exprval { sp_return_function_body(); }
| P_GOTO VARIABLE { sp_jump_to(INS_JMP, $2); }
;
lazy_stmt: lhs_val { sp_push_inode(sp_create_inode(INS_OBJDEL, NULL)); sp_clear_operation_stack(); sp_lhs_clear();  }
| lazy_expr { sp_push_inode(sp_create_inode(INS_OBJDEL, NULL)); sp_clear_operation_stack(); sp_lhs_clear();  }
| assignment { sp_clear_operation_stack(); sp_lhs_clear(); }
;
assignment: lhs_val S_EQ { sp_flush_scope(); sp_lhs_clear(); sp_set_scope_var(sp_peek_symrec()); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_ADDEQ { sp_lhs_get($1); sp_lhs_clear(); sp_set_scope_var(sp_peek_symrec()); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_SUBEQ { sp_lhs_get($1); sp_lhs_clear(); sp_set_scope_var(sp_peek_symrec()); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_MULEQ { sp_lhs_get($1); sp_lhs_clear(); sp_set_scope_var(sp_peek_symrec()); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_DIVEQ { sp_lhs_get($1); sp_lhs_clear(); sp_set_scope_var(sp_peek_symrec()); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); sp_end_expr(sp_peek_symrec()); }
| lhs_val S_MODEQ { sp_lhs_get($1); sp_lhs_clear(); sp_set_scope_var(sp_peek_symrec()); } lazy_expr { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); sp_end_expr(sp_peek_symrec()); }
| S_ADDADD VARIABLE { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $2)); sp_set_scope_var(sp_peek_symrec()); sp_load_stack($2); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_ADD); sp_store_stack($2); }
| S_SUBSUB VARIABLE { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $2)); sp_set_scope_var(sp_peek_symrec()); sp_load_stack($2); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_SUB); sp_store_stack($2); }
| VARIABLE S_ADDADD { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1)); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope($1, _RECAST(uchar *, "1"), INS_ADD); }
| VARIABLE S_SUBSUB { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1)); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope($1, _RECAST(uchar *, "1"), INS_SUB); }
;
rhs_val: rhs_val S_PDOT VARIABLE L_BR { printf("rhs api access %s\n", $3); sp_push_symrec(sp_start_function_call($3, 1)); } call_param_list R_BR { symrec * rec = sp_pop_symrec(); sp_end_function_call(rec); }
| rhs_val S_PDOT VARIABLE { printf("rhs object access\n"); sp_lz_load_constant($3); $$=18; sp_push_symrec(sp_lhs_load(18, sp_pop_symrec())); }
| rhs_val L_SB lazy_expr R_SB { printf("rhs array access\n"); sp_pop_symrec(); $$=17; sp_push_symrec(sp_lhs_load(17, sp_pop_symrec())); }
| lazy_val { $$=0; }
;
lhs_val: lhs_val S_PDOT VARIABLE { sp_lhs_get($1); printf("lhs object access\n"); sp_lz_load_constant($3); $$=18; sp_lhs_set(18); }
| lhs_val L_SB { sp_lhs_get($1); } lazy_expr R_SB { printf("lhs array access\n"); sp_pop_symrec(); $$=17; sp_lhs_set(17); }
| lhs_val S_PDOT VARIABLE L_BR { sp_lhs_get($1); printf("lhs api access %s\n", $3); sp_push_symrec(sp_start_function_call($3, 1)); } call_param_list R_BR { symrec * rec = sp_pop_symrec(); sp_end_function_call(rec); }
| VARIABLE { printf("push %s\n", $1); sp_lhs_store(sp_push_symrec(st_sym_select(SYM_TYPE_VAR, $1))); $$=0; } 
;
//| lhs_val { sp_push_symrec(sp_lhs_load($1, sp_peek_symrec())); $$=0; }
lazy_expr: L_BR lazy_stmt R_BR { symrec * rec = sp_peek_symrec(); sp_lz_load_variable(rec->name); }
| lazy_expr N_CONSTANT { sp_lz_load_constant($2); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lazy_expr S_ADD lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
| lazy_expr S_SUB lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); }
| lazy_expr S_MUL lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); }
| lazy_expr S_DIV lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); }
| lazy_expr S_MOD lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); }
| lazy_expr T_EQ lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CREQ, NULL)); }
| lazy_expr T_NE lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRNE, NULL)); }
| lazy_expr T_LT lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLT, NULL)); }
| lazy_expr T_GT lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGT, NULL)); }
| lazy_expr T_LTEQ lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLTEQ, NULL)); }
| lazy_expr T_GTEQ lazy_expr { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGTEQ, NULL)); }
| rhs_val {  }
; 
lazy_val: L_BR { sp_create_new_scope(NULL); } lazy_expr R_BR { sp_flush_scope(); sp_destroy_scope(); }
| P_NEW VARIABLE L_BR R_BR { sp_push_symrec(sp_lz_create_instance($2)); }
| instance_decl { sp_push_symrec(sp_lz_load_class($1)); }
| P_SIZEOF L_BR lazy_expr R_BR { sp_push_inode(sp_create_inode(INS_OBJSZ, NULL)); }
| VARIABLE { sp_push_symrec(sp_lz_load_variable($1)); } 
| lazy_func_call { }
| lambda_func_decl 
| array_decl { sp_push_symrec(sp_lz_load_array_constant()); }
| args_expr 
| constant_val { sp_push_symrec(sp_lz_load_constant($1)); }
;
lazy_func_call: instance_expr S_RNEXT VARIABLE L_BR { sp_push_symrec(sp_start_method_call($3)); } method_param_list R_BR { symrec * meth = sp_pop_symrec(); sp_push_symrec(sp_lz_load_variable($1)); sp_push_symrec(sp_end_method_call(sp_pop_symrec(), meth)); }
| VARIABLE L_BR { symrec * rec = sp_push_symrec(sp_start_function_call($1, 0)); printf("start call func: %s\n",rec->name); } call_param_list R_BR { symrec * rec = sp_peek_symrec(); printf("end call func: %s\n",rec->name); sp_end_function_call(rec); }
;
//| api_func_call { sp_pop_symrec(); }
//;
array_decl: P_HEX L_BR { sp_new_array_constant(); } constant_seq R_BR
;
constant_val: CONSTANT { strcpy(_RECAST(char *, $$), _RECAST(const char *, $1)); }
| N_CONSTANT { strcpy(_RECAST(char *, $$), _RECAST(const char *, $1)); }
;
constant_seq :  /* empty */
| constant_val { sp_push_array_constant($1); } ',' constant_seq
| constant_val { sp_push_array_constant($1); } 
;
args_expr: L_CL { sp_push_symrec(sp_start_api_call(15, 0)); } obj_params R_CL { sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
| L_SB { sp_push_symrec(sp_start_api_call(16, 0)); } arr_params R_SB { sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
;
obj_params: { }
| obj_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); } ',' obj_params
| obj_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
;
arr_params: { }
| arr_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); } ',' arr_params
| arr_param { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
;
obj_param: /* empty */ 
| VARIABLE { sp_push_symrec(sp_start_api_call(14, 2)); sp_lz_load_constant($1); } EOC lazy_expr { sp_pop_symrec(); sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
;
arr_param: /* empty */
| obj_param
| lazy_val { }
;
lambda_func_decl: P_FUNCTION L_BR { sp_push_symrec(sp_start_lambda_decl(0)); } lambda_param_list R_BR 
{ symrec * func; sp_push_symrec(sp_execute_lambda_call(sp_pop_symrec())); 
  func = sp_start_lambda_body(sp_pop_symrec()); if(func == NULL) YYABORT;
		sp_push_symrec(func); sp_flush_scope(); } 
	block_stmt 
{ sp_end_lambda_decl(sp_peek_symrec()); }
;
instance_expr: instance_decl { strcpy(_RECAST(char *, $$), _RECAST(const char *, $1)); }
;
instance_decl: VARIABLE { strcpy(_RECAST(char *, $$), _RECAST(const char *, $1)); }
;
case_stmt: P_SWITCH case_expr_param { sp_push_symrec(sp_start_case()); sp_push_symrec(sp_create_label(_RECAST(uchar *, "__d"))); } 
	L_CL case_block_stmt R_CL { symrec * rec = sp_pop_symrec(); symrec * srec = sp_pop_symrec(); sp_end_case(srec); sp_new_label(rec->name); } 
;
case_single_stmt: /* empty */
| P_CASE constant_val { sp_push_constant($2); } EOC { symrec * rec = sp_pop_symrec(); sp_label_case(sp_peek_symrec(), $2); sp_push_symrec(rec); }
| P_DEFAULT EOC { symrec * rec = sp_pop_symrec(); sp_default_case(sp_peek_symrec()); sp_push_symrec(rec); }
| block_stmt
;
exprs: expr T_AND { symrec * srec = sp_prev_symrec(); sp_jump_to(INS_JFALSE, srec->name); } exprs
| expr T_OR { symrec * srec = sp_peek_symrec(); sp_jump_to(INS_JTRUE, srec->name); } exprs
| expr { symrec * srec = sp_pop_symrec(); symrec * rec = sp_peek_symrec(); sp_jump_to(INS_JFALSE, rec->name); sp_push_symrec(srec); }
;
expr: exprval { }
;
exprval: lazy_expr { symrec * srec = sp_pop_symrec(); symrec * rec = sp_peek_symrec(); rec->sa_currec = srec; sp_flush_scope(); }
;
sys_func_decl: VARIABLE L_BR { sp_push_symrec(sp_declare_function($1)); } sys_param_list R_BR sys_extended_decl { symrec * rec = sp_pop_symrec(); if(sp_declare_function_end(rec->name) == NULL) YYABORT; }										/* internal function prototype */
;
sys_extended_decl: /* empty */
| P_SYSCALL CONSTANT { sp_declare_function_extern($2); }
;
sys_param_list: /* empty */
| VARIABLE { sp_declare_function_param($1); } ',' sys_param_list
| VARIABLE { sp_declare_function_param($1); }  
;
lambda_param_list: /* empty */
| VARIABLE { sp_push_symrec(sp_create_lambda_param($1)); } ','  lambda_param_list { symrec * rec = sp_pop_symrec(); sp_lz_store_variable(rec->name); }
| VARIABLE { symrec * rec = sp_create_lambda_param($1); sp_lz_store_variable($1); }
;
call_param_list: /* empty */
| lazy_expr { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); } ',' call_param_list
| lazy_expr { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
;
method_param_list: /* empty */
| lazy_expr { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); }
| lazy_expr { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); } ',' method_param_list
;
accessor_func: P_PUBLIC P_FUNCTION { $$ = TRUE; }
| P_PRIVATE P_FUNCTION { $$ = FALSE; }
| P_FUNCTION { $$ = FALSE; }
;
class_stmt: accessor_func VARIABLE { sp_push_symrec(sp_create_function($2)); sp_create_new_scope(NULL); } body_func_params
	{ symrec * func = sp_start_function_body(sp_pop_symrec(), $1); if(func == NULL) YYABORT;
		sp_push_symrec(func); sp_flush_scope(); } body_func_extended_decl 
	block_stmt 
	{ sp_end_function_body(sp_pop_symrec()); } 			/*function declaration with alias */
| P_EXTERN P_FUNCTION sys_func_decl EOS
| error_stmt
;
body_func_extended_decl: 	/* empty */
| P_ALIAS CONSTANT { sp_install_menu($2, sp_peek_symrec()); }		/* install current function as menu (setup menu) */
| P_EVENT CONSTANT { sp_install_event($2, sp_peek_symrec()); }		/* install current function as event (setup event) */
;
body_func_param_list: /* empty */
| VARIABLE { sp_push_symrec(sp_create_param($1)); } ','  body_func_param_list { symrec * rec = sp_pop_symrec(); sp_lz_store_variable(rec->name); }
| VARIABLE { symrec * rec = sp_create_param($1); sp_lz_store_variable($1); }
;
error_stmt: error { YYACCEPT; }
;
%%

void sp_init_parser(void) {
	//yylval.token = (yytoken *)malloc(sizeof(yytoken));
	errors = 0;
	yyinit();
}

uint32 sp_parse(void) {
	//start parse
	yyparse();
	//cleanup for any resources
	sp_cleanup_parser();			//semantic parser clean up
	lp_clear_globals();				//lexical analyzer clean up
	
	return errors;
}

extern void sp_error_push(uchar * msg);
void sp_error(uchar * msg, ...) {
	uchar buffer[512];
	uchar pbuffer[520];
	va_list argptr;
	va_start(argptr, msg);
	vsprintf(_RECAST(char *, buffer), _RECAST(const char *, msg), argptr);
	sprintf(_RECAST(char *, pbuffer), "[line: %d] %s", p_config->line + 1, buffer);
	sp_error_push(pbuffer);
	//printf("%s\n", buffer);
	//yyerror ( _RECAST(char *, buffer));
	va_end(argptr);              /* Reset variable argument list. */
	//errors ++;
}

/*=========================================================================
MAIN
=========================================================================*/
#ifdef STANDALONE_COMPILER
void main( int argc, char *argv[] )
{ 
	//extern FILE *yyin;
	int i = 1;
	err_record * iterator;
	uint16 strCount = 0;
	uchar gen_asm = 0, gen_il = 0, gen_scr = 0;
	//++argv;			//skip exepath
	uchar opt_level = 0;
	//sp_init_parser();
	if(argc == 1) goto print_usage;
	while(i != (argc - 1)) {
		if(strcmp(argv[i], "-ca") == 0) {
			gen_il = 1;
		} else if(strcmp(argv[i], "-c") == 0) {
			gen_asm = 1;
		} else if(strcmp(argv[i], "-cd") == 0) {
			gen_scr = 1;
		} else if(strcmp(argv[i], "-o1") == 0) {
			opt_level |= IS_OPTIMIZE_L1;
		} else if(strcmp(argv[i], "-o2") == 0) {
			opt_level |= IS_OPTIMIZE_L2 | IS_OPTIMIZE_L1;
		} else {
			print_usage:
			printf("usage: stack [options] filename\n	\
					options:\
					\n-ca\tcompilassembler [source->bin]\
					\n-cd\tscript generator [source->apdu]\
					\n-c\tcompile [source->asm]\
					\n-o1\tpeephole optimization\
					\n-o2\tdeadcode elimination");
			exit(0);
		}
		i++;
	}
	//yyin = fopen( argv[i], "r" );
	if(sp_init(_RECAST(uchar *, argv[i])) != NULL) {
		if(gen_il) is_init(_RECAST(uchar *, argv[i]));
		if(gen_asm) as_init(_RECAST(uchar *, argv[i]));
		if(gen_scr) sc_init(_RECAST(uchar *, argv[i]));
		i = sp_parse();
		if(i == 0) {
			is_link_optimize(opt_level);
			if(gen_il) {	
				is_file_flush();
			}
			if(gen_asm) {
				as_flush();
			}
			if(gen_scr) {
				sc_flush();
				sc_cleanup();
			}
		} else {
			iterator = sp_error_get_enumerator();
			while(iterator != NULL) {
				//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
				strCount ++;
				printf("%s\n", iterator->buffer);
				iterator = sp_error_next_record(iterator);
			}
		}
		//fclose(yyin);
	}	
}
#endif

/*=========================================================================
YYERROR
=========================================================================*/
extern void sp_error(uchar *, ...);
int yyerror ( char *s ) /* Called by yyparse on error */
{
	errors++;
	//printf("Error (%i) on file %s line %i: %s\n", errors, p_config->filename, p_config->line + 1, s);
	sp_error(SP_ERR_PARSER, s, yylval.bytes);
	return 0;
}
/**************************** End Grammar File ***************************/
