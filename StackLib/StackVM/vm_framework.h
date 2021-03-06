/*!\file 			vm_framework.h
 * \brief     	Orb-Weaver Framework APIs
 * \details   	Orb-Weaver Framework APIs provide all necessary function for Native API integration to stack machine
 * \author    	AGP
 * \version   	1.4
 * \date      	Created by Agus Purwanto on 06/18/2017.
 * \pre       	
 * \bug       	
 * \warning   	
 * \copyright 	OrbLeaf Technology
\verbatim
1.0
 * initial release, all APIs declared on this header	(2016.06.18)
 * added : va_trap_apis for persistent storage trap allocator, ported from icos_orb	(2016.06.21)
 * added : persistent storage variable APIs (2017.06.21)
 * added : SMTP APIs for sendmail (RFC822) based on http://www.codeproject.com/KB/mcpp/CSmtp.aspx (2017.06.21)
 * changed : added interrupt mechanism for pin event (2017.09.11)
 * added: va_ui_create_image to support image rendering from resource (2017.12.09)
 * added: va_sys_exec_31 to support execution of vm_function object generated by F2O (2018.01.10)
\endverbatim
 */

#include "../defs.h"
#include "../config.h"
#include "vm_stack.h"

#ifndef VM_FRAMEWORK__H

#define VA_OBJECT_DELIMITER			0x80
#define VA_OBJECT_MAX_SIZE			0x200

#define VA_STATE_WAIT						0x08
#define VA_STATE_RUN						0x01
#define VA_STATE_IDLE						0x00

typedef struct va_default_context{
	void * ctx;
	uint32 offset;
	void (* close)();
	void (* read)();
	void (* write)();
	void (* seek)();
} va_default_context;

#define VA_PINTYPE_IO						0x03
#define VA_PINTYPE_PWM					0x04
typedef struct va_port_context {
	va_default_context base;
	uint8 state;
	uint8 id;
	uint16 pin;
	void * handle;			//to be released
	uint8 dutycycle;
	uint16 freq;
} va_port_context;

typedef struct va_net_context {
	va_default_context base;
	uint8 type[3];
} va_net_context;

typedef struct va_com_context {
	va_default_context base;
	//USART_HandleTypeDef handle;
} va_com_context;

typedef struct va_picc_key {
	void * ctx;
	uint8 block;
	uint8 keyid;
	uint8 key[6];
	struct va_picc_key * next;
} va_picc_key;

typedef struct va_picc_context {
	va_default_context base;
	uint8 state;					//open/close state
} va_picc_context;

typedef struct va_port_config {
	uint16 id;
	void * port;
	uint32 pin;
} va_port_config;

extern CONST vm_api_entry g_vaRegisteredApis[];
//global
void va_sys_exec() _REENTRANT_ ;			//execute a function object
void va_set_execution_context() _REENTRANT_ ;
//default context read/write operation
void va_read() _REENTRANT_ ;
void va_write() _REENTRANT_ ;
void va_close() _REENTRANT_ ;
void va_seek() _REENTRANT_;
//var
void va_set_var() _REENTRANT_ ;
void va_get_var() _REENTRANT_ ;
void va_delete_var() _REENTRANT_ ; 
//string
void va_index_of() _REENTRANT_ ;
void va_last_index_of() _REENTRANT_ ;
void va_replace() _REENTRANT_ ;
void va_substr() _REENTRANT_ ;
//bytes creation
void va_bytes() _REENTRANT_ ;
//file
void va_fopen() _REENTRANT_ ;  
void va_fread() _REENTRANT_ ;
void va_fwrite() _REENTRANT_ ;
void va_fclose() _REENTRANT_ ;
void va_fpopbytag() _REENTRANT_ ;  
//json				
void va_arg_findtag() _REENTRANT_ ;	
void va_arg_count() _REENTRANT_;			
void va_arg_create() _REENTRANT_ ;
void va_arg_object() _REENTRANT_ ;
void va_arg_array() _REENTRANT_ ; 	 
void va_arg_at() _REENTRANT_ ;
void va_arg_get() _REENTRANT_ ;
void va_arg_serialize() _REENTRANT_ ;			// -> to json string
void va_arg_deserialize() _REENTRANT_ ;			// -> from json string
void va_arg_add() _REENTRANT_ ;
void va_arg_set() _REENTRANT_ ;		
void va_arg_remove() _REENTRANT_ ;

//toolkit (21-36)
void va_select_item() _REENTRANT_;
void va_display_text() _REENTRANT_;
void va_get_input() _REENTRANT_;
void va_set_timer() _REENTRANT_ ; 
//invoke external
void va_invoke_external() _REENTRANT_ ;
//picc APIs
void va_picc_open() _REENTRANT_ ;
void va_picc_auth() _REENTRANT_;
void va_picc_transmit();
void va_picc_init();				//init global PICC context
void va_picc_release_all();		//release global PICC context
//iso8583 (37-39)
void va_iso_create_message() _REENTRANT_ ; 
void va_iso_push_element() _REENTRANT_ ;
void va_iso_get_element() _REENTRANT_ ;

//generic toolkit
void va_toolkit_create() _REENTRANT_ ;  
void va_toolkit_push_ext() _REENTRANT_ ;
void va_toolkit_push_raw() _REENTRANT_ ;
void va_toolkit_dispatch() _REENTRANT_ ;
void va_toolkit_get_result() _REENTRANT_ ;
//bit operation 
void va_check_bit() _REENTRANT_ ; 
void va_set_bit() _REENTRANT_ ;
void va_clear_bit() _REENTRANT_ ;
//converter
void va_bin2hex() _REENTRANT_;
void va_hex2bin() _REENTRANT_; 
void va_bin2dec() _REENTRANT_;
void va_dec2bin() _REENTRANT_;
void va_b64_encode() _REENTRANT_ ;
void va_b64_decode() _REENTRANT_ ;
//codec
void va_crypto_create() _REENTRANT_ ;
void va_crypto_encrypt() _REENTRANT_ ; 
void va_crypto_decrypt() _REENTRANT_ ;
void va_random() _REENTRANT_ ;
void va_digest() _REENTRANT_ ;
//security
void va_verify_pin() _REENTRANT_ ;
//toolkit manager			  
void va_terminal_profile() _REENTRANT_ ; 
//cross APIs
void va_wib_set_return_var() _REENTRANT_ ; 
void va_get_info() _REENTRANT_ ;
//default syscall return
vm_object * va_syscall_ret(uint8 size, uint8 * buffer) _REENTRANT_ ;

void va_delay();
void va_ui_init();
void va_ui_release_all();
void va_ui_alert();
void va_ui_create_window();
void va_ui_destroy_window();
void va_ui_create_label();
void va_ui_create_button();
void va_ui_create_textbox();
void va_ui_create_image();
void va_ui_get_text();
void va_ui_set_text();
void va_ui_wait();
//void va_ui_present(tk_context_p ctx, int32 ms) ;
void va_ui_push_window();			//non blocking
void va_ui_pop_window();				//non blocking
//IO com APIs
void va_com_init() ;		//system startup
void va_com_open();
void va_com_transmit();
void va_com_readline();
//IO port APIs
void va_port_open();
//network APIs
void va_net_open();
void va_net_transmit();
void va_net_mail_create();
void va_net_mail_send();

//default util
int32 va_o2f(vm_object * obj) _REENTRANT_ ;
void va_return_word(uint16 val) _REENTRANT_ ;

#define VM_FRAMEWORK__H
#endif
