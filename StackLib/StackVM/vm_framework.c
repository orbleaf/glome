#include "../defs.h"
#include "../config.h"	
#include "../crypto/inc/cr_apis.h"
#include "vm_stack.h"	
#include "vm_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern vm_object * g_pVaRetval;
extern sys_context g_sVaSysc;
//tk_context_p g_pfrmctx = NULL;

CONST vm_api_entry g_vaRegisteredApis[] = {
 	//var APIs
#if STACK_VAR_APIS
	{1, "set_value", va_set_var, NULL},
 	{2, "get_value", va_get_var, NULL},
 	{3, "delete_value", va_delete_var, NULL},
#endif
	//string APIs
	{4, "index_of", va_index_of, NULL},
	{5, "replace", va_replace, NULL},	   
	{6, "substr", va_substr, NULL},
	{7, "to_bytes", va_bytes, NULL},
#if STACK_FILE_APIS
	//file APIs
	{8, "fopen", va_fopen, NULL},  
#endif
	{9, "close", va_close, NULL},			//default context close
	{10, "read", va_read, NULL},			//default context read
	{11, "write", va_write, NULL},			//default context write
	{12, "seek", va_seek, NULL},			//default context seek
#if STACK_FILE_APIS 
	{12, va_fpopbytag, NULL},
#endif 
	{13, "count", va_arg_count, NULL},
	{14, "arg_create", va_arg_create, NULL},
	{15, "arg_object", va_arg_object, NULL},
	{16, "arg_array", va_arg_array, NULL}, 	 
	{17, "at", va_arg_at, NULL},
	{18, "get", va_arg_get, NULL},
	{19, "to_json", va_arg_serialize, NULL},			// -> to json string
	{20, "from_json", va_arg_deserialize, NULL},			// -> from json string
	{21, "add", va_arg_add, NULL},
	{22, "set", va_arg_set, NULL},
	{23, "remove", va_arg_remove, NULL},
	//support for lambda expression (2018.01.10)
	{31, "exec", va_sys_exec, NULL},
#if 0
	{32, "ti_load", va_invoke_external, NULL},			//invoke another framework
	{33, "ci_load", va_invoke_card, NULL},			//invoke card framework
#endif
	
#if 0
	{37, "iso_create", va_iso_create_message, NULL },
	{38, "iso_push", va_iso_push_element, NULL },
	{39, "iso_get", va_iso_get_element, NULL },
#endif

#if 0
	{48, "begin", va_async_begin, NULL},
	{49, "end", va_async_end, NULL},
	{50, "rollback", va_async_rollback, NULL}
	{51, "commit", va_async_commit, NULL},
#endif

#if STACK_BIT_APIS
	//bit APIs
	{128, "check_bit", va_check_bit, NULL},
	{129, "set_bit", va_set_bit, NULL},	 
	{130, "clear_bit", va_clear_bit, NULL},
#endif
#if STACK_CONVERTER_APIS
	//converter APIs			(131-135)
	{131, "to_hex", va_bin2hex, NULL},
	{132, "from_hex", va_hex2bin, NULL}, 
	{133, "to_dec", va_bin2dec, NULL},
	{134, "to_b64", va_b64_encode, NULL},
	{135, "from_b64", va_b64_decode, NULL},
#endif
#if STACK_CRYPTO_APIS	
	//cryptography APIs		(136-142)
	{136, "cr_create", va_crypto_create, NULL},
	{137, "encrypt", va_crypto_encrypt, NULL },	 
	{138, "decrypt", va_crypto_decrypt, NULL },
	{139, "cr_random", va_random, NULL },
#endif
#if STACK_NETWORK_APIS
	//network APIs		(144-148)
	{144, "net_open", va_net_open, NULL },			//create a new connection (net_context), whose behaviour similar with file context
	{145, "net_transmit", va_net_transmit, NULL},		//(param1 = address, param2=payload (OWB array), param3 = method (GET, POST), param4=type(TCP, UDP)  
	{148, "mail_create", va_net_mail_create, NULL},	//param1=server, param2=port, param3=username, param4=password
	{149, "mail_send", va_net_mail_send, NULL},		//param1=handle, param2=to, param3=subject, param4=msg, param5=from
#endif
#if STACK_GUI_APIS	
	//ui APIs (160-191)
	{160, va_ui_alert, NULL },
	{161, va_ui_create_window, NULL },
	{162, va_ui_destroy_window, NULL },
	{163, va_ui_create_label, NULL },
	{164, va_ui_create_button, NULL },
	{165, va_ui_create_textbox, NULL },
	{166, va_ui_create_image, NULL},
	{168, va_ui_wait, NULL },
	//UI framework APIs
	{169, va_display_text, NULL},
	{170, va_get_input, NULL},
	{171, va_select_item, NULL},
	//UI util APIs
	{176, va_ui_get_text, NULL},
	{177, va_ui_set_text, NULL},
	
	{190, va_ui_push_window, NULL},
	{191, va_ui_pop_window, NULL},
#endif
	{169, "print", va_display_text, NULL},
#if STACK_IO_APIS	
	//UART APIs				(192-195)
	{192, "com_open", va_com_open, NULL },			//create a comm connection (com_context), whose behaviour similar with file context
	//{193, va_com_transmit, NULL},		//send bytes and wait for response at specific period
	{194, "com_readline", va_com_readline, NULL},		//read bytes until newline found
		
	//Port APIs (196-199)
	{196, "port_open", va_port_open, NULL },
	//{197, va_io_write, NULL },
 #endif		
#if STACK_PICC_APIS	//(200-204)
	{200, "picc_open", va_picc_open, NULL},
	{201, "picc_auth", va_picc_auth, NULL},
	{202, "picc_transmit", va_picc_transmit, NULL},
#endif
	//OS APIs (specific)
	//{240, va_delay, NULL},			//OS APIs specific
	{0, "", NULL, NULL}
} ;

//extern vf_handle _vm_file;
uint8 g_vaWaitTag = 0; 

void va_init_context() {
	//GPIO_InitTypeDef GPIO_InitStructure;       
	//g_pfrmctx = ctx;
	//port init
	//__HAL_RCC_GPIOE_CLK_ENABLE();
   	/* PORT E.0, E.1, E.2 */
  	//GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
  	//GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;	 					//default AF_PP
	//GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
  	//GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
  	//HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
	//com init
	//va_com_init();
}

void va_exec_init() {			//called one time before any execution
	//GUI init
	//va_ui_init();
	//va_picc_init();
}

void va_exec_release() {		//called one time after any execution
	//GUI release
	//va_ui_release_all();
	//va_picc_release_all();
}

int32 va_o2f(vm_object * obj) _REENTRANT_ {
	uint8 buffer[7];
	uint8 offset = 0;
	uint8 len = obj->len;
	if(len == 0) return 0;		//NULL object
	if(obj->len > 6) len = 6;
	offset = obj->len - len;
	vm_memcpy(buffer, obj->bytes + offset, len);
	buffer[len] = 0;
	return (int32)atoi((const char *)buffer); 
}	  

void va_return_word(uint16 val) _REENTRANT_ {
	char nbuf[10];
	sprintf(nbuf, "%d", (uint16)val);
	g_pVaRetval = vm_create_object(vm_strlen(nbuf), _RECAST(uchar *, nbuf));
}

#define VA_DISP_TEXT 	STK_CMD_DISPLAY_TEXT 
#define VA_DISP_INKEY	STK_CMD_GET_INKEY
#define VA_DISP_IDLE	STK_CMD_SET_UP_IDLE_TEXT

#define VA_OBJECT_DELIMITER		0x80
#define VA_VAR_SET			1
#define VA_VAR_GET			2
#define VA_VAR_DELETE	3 
 
#if STACK_VAR_APIS
static void va_var_operation(uint8 mode) _REENTRANT_ {
   	vm_object * param;
	//FSHandle fhandle;	
	vf_handle fhandle;
	uint16 i = 0, j;
	uint8 exist = 0;
	BSIterator iterator;
	uint8 tag[3] = { 0xE0, 0x1F, 0x00 };
	uint8 lbuf[VA_OBJECT_MAX_SIZE];
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint16 objLen;     
	uint16 glen = 0;
	uint16 clen;				    
	param = vm_get_argument(0);
	if(param->len == 0) return;// g_pVaRetval;
	tk_kernel_data_load(g_pfrmctx, &fhandle);
	memcpy(&iterator, &fhandle, sizeof(BSAllocHandle));
	objLen = bsIteratorInit(&iterator, (BSAllocHandle *)&iterator);
	while(objLen != 0) {
		objLen = bsIteratorRead(&iterator, 0, bbuf, objLen);
		//list variable
		if(param->len == 0) {
			lbuf[glen + 2] = ASN_TAG_OCTSTRING;	
			for(clen=0;clen<objLen;clen++) if(bbuf[clen] == VA_OBJECT_DELIMITER) break;
			lbuf[glen + 3] = clen;
	   		memcpy(lbuf + glen + 4, bbuf, clen); 
			glen += (clen + 2);
		} else if(objLen >= (param->len +1)) {
			if(memcmp(bbuf, param->bytes, param->len) == 0 && bbuf[param->len] == VA_OBJECT_DELIMITER) break;	//matched
		}
		objLen = bsIteratorNext(&iterator);
	}
	switch(mode) {
		case VA_VAR_GET:
			g_pVaRetval = VM_NULL_OBJECT; 
			if(param->len == 0) {  
				//list variable
				lbuf[0] = ASN_TAG_SET;
				lbuf[1] = glen;
				g_pVaRetval = vm_create_object(glen + 2, lbuf);
				return;
			}
			if(objLen == 0) return; 
			if(objLen == (param->len + 1)) return;
			g_pVaRetval = vm_create_object(objLen - (param->len + 1), bbuf + (param->len + 1));
			break;	
		default:
		case VA_VAR_DELETE:
	 	case VA_VAR_SET:  
			if(objLen != 0) {
				bsReleaseByOffset((BSAllocHandle *)&iterator, iterator.current);	
			}
			if(mode == VA_VAR_DELETE) break;
			memcpy(bbuf, param->bytes, param->len); 
			j = param->len;
			bbuf[j++] = VA_OBJECT_DELIMITER;
			param = vm_get_argument(1);	
			memcpy(bbuf + j, param->bytes, param->len);
			j += param->len;	 
			if(bsErr(iterator.current = bsAllocObject((BSAllocHandle *)&iterator, tag, j))) return;
			bsHandleWriteW(&iterator, iterator.current, bbuf, j);
			break;
	}
}
			
void va_set_var() _REENTRANT_ { 
	OS_DEBUG_ENTRY(va_set_var);
	va_var_operation(VA_VAR_SET);	
	OS_DEBUG_EXIT();
} 
 		
void va_get_var() _REENTRANT_ { 
	OS_DEBUG_ENTRY(va_get_var);
	va_var_operation(VA_VAR_GET);
	OS_DEBUG_EXIT();
}
		
void va_delete_var() _REENTRANT_ {
	OS_DEBUG_ENTRY(va_delete_var);
	va_var_operation(VA_VAR_DELETE);
	OS_DEBUG_EXIT();
}
#endif 	//end of var apis

void va_display_text() {
	uint8 i;
	vm_object * param;
	uint8 buffer[256];
	uint8 arg_count = vm_get_argument_count();
	for(i=0;i<arg_count;i++) {
		param = vm_get_argument(i);
		memset(buffer, 0, sizeof(buffer));
		strncpy(_RECAST(char *, buffer), _RECAST(const char *, param->bytes), param->len);
		printf("%s<br>", buffer);
	}
}

#define VA_STR_INDEX_OF			0
#define VA_STR_LAST_INDEX_OF	1
#define VA_STR_REPLACE			2

static void va_string_operation(uint8 mode) _REENTRANT_ {
	vm_object * pattern;
	vm_object * param;
	uint16 index = 0xFFFF;
	uint16 offset = 0;
	uint8 length;
	uint8 wbuf[256];
	//BYTE buffer[10];
	if(vm_get_argument_count() < 2) return;// g_pVaRetval;
	param = vm_get_argument(0);
	if(param->len != 0) { 		//source string
	  	vm_memcpy(wbuf, param->bytes, param->len);
		length = param->len;
	}
	param = vm_get_argument(1);
	if(param->len != 0) {		//pattern
		if(length < param->len) return;// g_pVaRetval;
		pattern = param;
		//length -= param->len;
	}
	param = vm_get_argument(2);
	if(param->len != 0) {  		//offset/new pattern
		if(mode < 2) { 		//check if index_of or last_index_of
			if(param->len > 10) return;// g_pVaRetval;	
			offset = va_o2f(param);	 
			if(offset > length) return;// g_pVaRetval;
		}  
	}
	switch(mode) {
	  	case VA_STR_INDEX_OF:
			for(offset;offset<length;offset++) {
			 	if(vm_memcmp(wbuf + offset, pattern->bytes, pattern->len) == 0) {
					//mmItoa(MM_ITOA_WORD, g_baOrbBuffer + length, (WORD)offset);
					sprintf(_RECAST(char *, wbuf), "%d", (uint16)offset);
					g_pVaRetval = vm_create_object(vm_strlen(_RECAST(const char *, wbuf + length)), wbuf + length);
				 	break;
				}
			}
			break;
		case VA_STR_REPLACE:
			//for(offset;offset<length;offset++) {
			while(offset < length) {
			 	if(vm_memcmp(wbuf + offset, pattern->bytes, pattern->len) == 0) {
					length -= pattern->len;
					vm_memcpy(wbuf + offset + param->len, wbuf + offset + pattern->len, length - offset);
					vm_memcpy(wbuf + offset, param->bytes, param->len);
					length += param->len;
					offset += param->len;
				} else {
					offset ++;
				}
			}
			g_pVaRetval = vm_create_object(offset, wbuf);
			break;
	}
	return;// g_pVaRetval;
}

void va_index_of() _REENTRANT_ {
	OS_DEBUG_ENTRY(va_index_of);
 	va_string_operation(VA_STR_INDEX_OF);
	OS_DEBUG_EXIT();
}

void va_replace() _REENTRANT_ {
	OS_DEBUG_ENTRY(va_replace);
   	va_string_operation(VA_STR_REPLACE);
	OS_DEBUG_EXIT();
}

static uchar va_is_numeric(uchar * buffer, uchar len) _REENTRANT_ { 
	uint8 n;
	if(len == 0) return FALSE;
	while(len != 0 ) {
		n = buffer[--len];
		if(n > 0x39 || n < 0x30) { return FALSE; }
	} 
	return TRUE;
}

void va_substr() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_substr);
	uint8 offset; 
	uint8 len;	 
	uint8 * opd1;
	uint8 * opd2;
	vm_object * obj;
	vm_object * op1;
	vm_object * op2;
	if(vm_get_argument_count() >= 2) {// g_pVaRetval;
		obj = vm_get_argument(0);  
		op1 = vm_get_argument(1);
		op2 = vm_get_argument(2);
		opd1 = (uint8 *)mmAllocMemP(op1->len + 1);
		opd2 = (uint8 *)mmAllocMemP(op2->len + 1);
		vm_memcpy(opd1, op1->bytes, op1->len); opd1[op1->len] = 0;		//null terminated string
		vm_memcpy(opd2, op2->bytes, op2->len); opd2[op2->len] = 0;		//null terminated string
		if(va_is_numeric(_RECAST(uchar *, op1->bytes), op1->len) == FALSE) { offset = 0; } else { offset = atoi(_RECAST(const char *, opd1)); }			//
		if(va_is_numeric(_RECAST(uchar *, op2->bytes), op2->len) == FALSE) { len = obj->len; } else { len = atoi(_RECAST(const char *, opd2)); }
		if(len > (obj->len - offset)) len = (obj->len - offset);
		mmFreeMem(opd1);
		mmFreeMem(opd2);
		g_pVaRetval = vm_create_object(len, obj->bytes + offset);
	}
	OS_DEBUG_EXIT();
}

#define VA_FILE_OPEN 	0
#define VA_FILE_READ 	1
#define VA_FILE_WRITE 	2
#define VA_FILE_CLOSE 	4
#define VA_FILE_BY_TAG	0x10

#if STACK_FILE_APIS
static void va_file_operation(BYTE mode) _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT;
	vm_object * param;
	vm_object * temp;
	uint8 i,j;
	//FSFileParameters fparam;
	//WORD fid;
	uint8 fidbuf[4];
	uint16 offset, length;
	FSHandle * file;// = vm_get_argument(0);
	uint8 isRead = 0;
	//fsGetPropertyB(file, &fparam, sizeof(FSFileParameters));
	switch(mode & 0x0F) {
		case VA_FILE_OPEN:
			if(vm_get_argument_count() < 1) return;// g_pVaRetval;
			param = vm_get_argument(0);		//path
			if(param->len == 0) return;// g_pVaRetval;
			if((param->len & 0x03) != 0x00) return;// g_pVaRetval;		//odd path
			//decode hex path
			temp = vm_create_object(sizeof(FSHandle) + 1, NULL);
			if(temp->len == 0) return;// g_pVaRetval;
			temp->bytes[0] = 0xF2;		//set handle tag
			file = (temp->bytes + 1);		//handle location pointer
			cmSelectCurrentMFW(cmGetSelectedContext(), file, NULL); 
			for(i=0;i<param->len;i+=4) {
				vm_memcpy(fidbuf, param->bytes + i, 4);
				tkHex2Bin(fidbuf, 4, 4);
				if(fsSelectFileByFidW(file, end_swap16(*((WORD *)fidbuf)), NULL) != FS_OK) {
					vm_release_object(temp);  		//delete allocated object
					return;// g_pVaRetval;	 		//return
				} 	
			}
			g_pVaRetval = temp;				//set return value
			break;
		case VA_FILE_READ: 
			isRead = 1;
		case VA_FILE_WRITE:
			if(vm_get_argument_count() < 3) return;// g_pVaRetval;
			temp = vm_get_argument(0);
			if(temp->len != (sizeof(FSHandle) + 1)) return;// g_pVaRetval;	//invalid handle
			if(temp->bytes[0] != 0xF2) return;// g_pVaRetval;		//invalid handle
			file = (temp->bytes + 1);
			fsGetPropertyB(file, &g_strFileParams);
			offset = va_o2f(vm_get_argument(1)); 		//offset
			if(isRead) {
				length = va_o2f(vm_get_argument(2));	//length
				switch(g_strFileParams.desc & 0x07) {
				  	case 0x01:		 //binary
						if(mode & VA_FILE_BY_TAG) {
							if((length = ftPopByTag(file, offset, (BYTE)length, g_baOrbBuffer, 255)) == -1) length = 0;
						} else { 
							length = fsFileReadBinaryW(file, offset, g_baOrbBuffer, length);
						}
						break;
					case 0x02:		 //linfix
					case 0x03:
					case 0x06:		 //cyclic
					case 0x07: 
						if(offset == 0) return;// g_pVaRetval;
						if(offset > g_strFileParams.recnum) return;// g_pVaRetval;  
						if(mode & VA_FILE_BY_TAG) {	
							fsFileReadRecordW(file, offset -1, g_baOrbBuffer, g_strFileParams.reclen);
							offset = 0;
							while(offset < g_strFileParams.reclen) {
							  	if(g_baOrbBuffer[offset++] == (BYTE)length) {
								 	length = g_baOrbBuffer[offset++];
									vm_memcpy(g_baOrbBuffer, g_baOrbBuffer + offset, length);
									offset = g_strFileParams.reclen;
									goto operation_finished;
								} else {
								 	offset += (g_baOrbBuffer[offset] + 1);
								}
							}
							length = 0; 	
						}  else {
							length = fsFileReadRecordW(file, offset -1, g_baOrbBuffer, length);	
						}
						break;
				}
				operation_finished:
				g_pVaRetval = vm_create_object((uint8)length, g_baOrbBuffer);  		//set return to data readed
			} else {
			 	param = vm_get_argument(2);				//data
				switch(g_strFileParams.desc & 0x07) {
				  	case 0x01:		//binary
						length = fsFileWriteBinaryW(file, offset, param->bytes, param->len);
						break;
					case 0x02:		//linfix
					case 0x03:		  
						if(offset == 0) return;// g_pVaRetval;
						if(offset > g_strFileParams.recnum) return;// g_pVaRetval;
						length = fsFileWriteRecordW(file, offset - 1, param->bytes, param->len);
						break;
#if FS_API_APPEND_RECORD
					case 0x06:		//cyclic
					case 0x07:
						if(offset != 0) return;// g_pVaRetval;
						length = fsFileAppendRecordW(file, 0, param->bytes, param->len);
						break;
#endif
				} 
				mmItoa(MM_ITOA_WORD, g_baOrbBuffer, length);
				length = (BYTE)mmStrLen(g_baOrbBuffer);
				//g_pVaRetval = vm_create_object(, g_baOrbBuffer);		//set return to ammount of bytes wrote
				goto operation_finished;
			}
			break;
		case VA_FILE_CLOSE:
			if(vm_get_argument_count() < 1) return;// g_pVaRetval;
			temp = vm_get_argument(0);
			if(temp->len != (sizeof(FSHandle) + 1)) return;// g_pVaRetval;	//invalid handle
			file = (temp->bytes + 1);
			if(temp->bytes[0] == 0xF2) {	   
				temp->bytes[0] = 0x2F;			//set handle tag to closed handle
				//g_pVaRetval = vm_create_object(4, VM_TRUE_OBJECT);		//create new object (clone object)
			}// else {
				//g_pVaRetval = vm_create_object(5, VM_FALSE_OBJECT);		//create new object (clone object)
			//}
			//optional parameter (activate/deactivate)
			param = vm_get_argument(1);
			if(param->len != 0) {
				fsChangeStateB(file, va_o2f(param));
			}
			break;
	 	
	}
	return;// g_pVaRetval;
}

void va_fopen() _REENTRANT_ {
	va_file_operation(VA_FILE_OPEN);
}

void va_fpopbytag() _REENTRANT_ {  
	va_file_operation(VA_FILE_BY_TAG | VA_FILE_READ);
}

void va_fread() _REENTRANT_ {
	va_file_operation(VA_FILE_READ);
}

void va_fwrite() _REENTRANT_ { 
	va_file_operation(VA_FILE_WRITE);
}

void va_fclose() _REENTRANT_ { 
	va_file_operation(VA_FILE_CLOSE);
} 
#endif	//file APIs

void va_seek() _REENTRANT_ {
	if(((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->seek != NULL) {
		((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->seek();
	}
}

void va_read() _REENTRANT_ {
	if(((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->read !=NULL) {
		((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->read();
	}
}

void va_write() _REENTRANT_ {
	if(((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->write != NULL) {
		((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->write();	
	}
}

void va_close() _REENTRANT_ {
	if(((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->close != NULL) {
		((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->close();
	}
}

const char bin2hexchar[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
uint16 tk_bin2hex(uint8 * bytes, uint16 len, uint8 * hexstring) {
	uint16 j;
	for(j=0;j<len;j++) {
		hexstring[j << 1] = bin2hexchar[bytes[j] >> 4];
		hexstring[(j << 1) + 1] = bin2hexchar[bytes[j] & 0x0F];
	}
	hexstring[len << 1] = 0;
	return len << 1;
}

void va_bin2hex() _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	//OS_DEBUG_ENTRY(va_bin2hex);
	vm_object * param;
	uint8 i;
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
	param = vm_get_argument(0);
	//printf("len %d\r\n", param->len);
	if(param->len < 128) {		//return;// g_pVaRetval;
		g_pVaRetval = vm_create_object(param->len * 2, param->bytes);
		//fill hexstring
		tk_bin2hex(param->bytes, param->len, g_pVaRetval->bytes);
	}	
	//OS_DEBUG_EXIT();
	return;// g_pVaRetval;
}

static uint8 tk_hex2byte(uint8 hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f') return (hexchar - 'a') + 10;
	if(hexchar >= 'A' && hexchar <= 'F') return (hexchar - 'A') + 10;
	if(hexchar >= '0' && hexchar <= '9') return hexchar - '0';
	return 0;
}

uint16 tk_hex2bin(uint8 * hexstring, uint8 * bytes) {
	uint16 i = 0;
	uint8 c;
	uint16 len=0;
	while(hexstring[i] != 0) {
		if(i & 0x01) {
			c <<= 4;
			c |= tk_hex2byte(hexstring[i]);
			bytes[len] = c;
			len++;
		} else {
			c = tk_hex2byte(hexstring[i]);
		}
		i++;
	}
	return len;
}

void va_hex2bin() _REENTRANT_ { 
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT;
	//OS_DEBUG_ENTRY(va_hex2bin);
	vm_object * param;
	uint8 i;
	uint8 b;
	uint8 len;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	param = vm_get_argument(0);
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
		//len = param->len / 2;
	//memcpy(g_baOrbBuffer, param->bytes, param->len);
	//tkHex2Bin(g_baOrbBuffer, param->len, param->len);
	len = tk_hex2bin(param->bytes, bbuf);
	g_pVaRetval = vm_create_object(len, bbuf);
	//}
	//OS_DEBUG_EXIT();
	return;// g_pVaRetval;
}

void va_bin2dec() _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	//OS_DEBUG_ENTRY(va_bin2dec);
	vm_object * param;
	uint8 buffer[6];
	uint16 d = 0;
	uint8 i;
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
	param = vm_get_argument(0);
	if(param->len <= 2) {	//return;// g_pVaRetval;
		for(i=param->len;i>0;i--) {
			d *= 0x100;
			d += param->bytes[i-1];
		}
		//mmSprintf(buffer, "%d", d);
		//g_pVaRetval = vm_create_object(vm_strlen(buffer), buffer);		//return decimal value	
		//}
		va_return_word((uint16)d);
	}
	//OS_DEBUG_EXIT();
	return;// g_pVaRetval;	
}

void va_b64_encode() _REENTRANT_ {	
	//OS_DEBUG_ENTRY(va_b64_encode);
	vm_object * param;
  	uint8 out_len = 0;
  	uint8 i = 0;
  	uint8 j = 0;
  	uint8 char_array_3[3];
  	uint8 char_array_4[4];	 
	uint8 * bytes_to_encode;
	uint8 in_len;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];

	CONST uint8 base64_chars[] = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/"; 
	param = vm_get_argument(0);
	bytes_to_encode = param->bytes;
	in_len = param->len;


  	while (in_len--) {
    	char_array_3[i++] = *(bytes_to_encode++);
    	if (i == 3) {
      	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      	char_array_4[3] = char_array_3[2] & 0x3f;

      	for(i = 0; (i <4) ; i++)
      	  	bbuf[out_len++] = base64_chars[char_array_4[i]];
      	i = 0;
    	}
  	}

  	if (i)
  	{
    	for(j = i; j < 3; j++)
      		char_array_3[j] = '\0'; 
    	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    	char_array_4[3] = char_array_3[2] & 0x3f;

    	for (j = 0; (j < i + 1); j++)
      		bbuf[out_len++] = base64_chars[char_array_4[j]];

    	while((i++ < 3))
      		bbuf[out_len++] = '=';
  	}
  	//g_baOrbBuffer[out_len] = 0;		//EOS
  	g_pVaRetval = vm_create_object(out_len, bbuf);		//return decimal value
	//OS_DEBUG_EXIT();
}

static uint8 va_pr2six(uint8 ch) {
	if ((ch >= 'A') && (ch <= 'Z')) ch = ch - 'A';
	else if ((ch >= 'a') && (ch <= 'z')) ch = ch - 'a' + 26;	
	else if ((ch >= '0') && (ch <= '9')) ch = ch - '0' + 52;
	else if (ch == '+') ch = 62;	
	else if (ch == '=') ch = 64;
	else if (ch == '/') ch = 63;
	else ch = 64;
	return ch;
}

void va_b64_decode() _REENTRANT_ {	
	//OS_DEBUG_ENTRY(va_b64_decode);
	vm_object * param;
	uint8 nbytesdecoded;
    register uint8 *bufin;
    register uint8 *bufout;
    register uint8 nprbytes;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	param = vm_get_argument(0);
	  
    nprbytes = param->len;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3; 

    bufout = (uint8 *) bbuf;
    bufin = (uint8 *) param->bytes;

    while (nprbytes > 4) {
		*(bufout++) = (uint8) (va_pr2six(*bufin) << 2 | va_pr2six(bufin[1]) >> 4);
		*(bufout++) = (uint8) (va_pr2six(bufin[1]) << 4 | va_pr2six(bufin[2]) >> 2);
		*(bufout++) = (uint8) (va_pr2six(bufin[2]) << 6 | va_pr2six(bufin[3]));
		bufin += 4;
		nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) *(bufout++) = (uint8) (va_pr2six(*bufin) << 2 | va_pr2six(bufin[1]) >> 4);
    if (nprbytes > 2) *(bufout++) = (uint8) (va_pr2six(bufin[1]) << 4 | va_pr2six(bufin[2]) >> 2);
    if (nprbytes > 3) *(bufout++) = (uint8) (va_pr2six(bufin[2]) << 6 | va_pr2six(bufin[3]));

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    //return nbytesdecoded;  
  	g_pVaRetval = vm_create_object(nbytesdecoded, bbuf);		//return decimal value
	//OS_DEBUG_EXIT();
}

CONST struct va_crypto_codec {
 	uint8 * name;
	uint8 mode;
	uint8 keylen;
} g_vaRegisteredCodec[] = { 
	{ (uint8 *)"DES", CR_MODE_DES, 8 }, 
	{ (uint8 *)"DES2", CR_MODE_DES2, 16 },
	{ (uint8 *)"DES3", CR_MODE_DES3, 24 },
	{ (uint8 *)"AES", CR_MODE_AES, 16 },	
	{ NULL, CR_MODE_DES, 0 },
};

//uint8 p_cryp_buffer[256];		//use static buffer for enciphering/deciphering
void va_crypto_create() _REENTRANT_ {	
	//param[0] = codec 0 = "DES", 1 = "DES2", 2 = "DES3", 3 = "AES"
	//param[1] = mode  "CBC" / null
	//param[2] = key	
	//OS_DEBUG_ENTRY(va_crypto_create);
	vm_object * param;
	vm_object * key;
	uint8 len;	 
	uint8 mode = 0;			//default DES
	uint8 keylen = 8;		//8 bytes per block
	uint8 objLen;
	//VACryptoContext crCtx;
	cr_context_p pctx;
	struct va_crypto_codec * c_iterator = (struct va_crypto_codec *)&g_vaRegisteredCodec[0];  
 	if(vm_get_argument_count() >= 3) {		//return;	
		//check parameter 0, supported codec
		param = vm_get_argument(0);
		while(c_iterator->name != NULL) {	 
			if(vm_memcmp(param->bytes, c_iterator->name, param->len) == 0) { mode = c_iterator->mode; keylen=c_iterator->keylen; break; }
			c_iterator++;
		}
		if(c_iterator->name == NULL) {
			vm_invoke_exception(VX_UNIMPLEMENTED_APIS);
			goto exit_crypto_create;
		}
		//check codec mode ECB/CBC
		param = vm_get_argument(1);
		if(vm_memcmp(param->bytes, "CBC", param->len) == 0) mode |= CR_MODE_CBC;
		key = vm_get_argument(2);
		keylen = key->len;
		//mmMemSet(crCtx.icv, 0, CR_ICV_SIZE);		//clear icv
		//crInitContext(&crCtx, g_baOrbBuffer);		//changed to context on 2015.06.14
		//crCtx.mode = mode;	
		objLen = sizeof(cr_context) + keylen;
		g_pVaRetval = vm_create_object(objLen + 256, NULL);	  
		if(g_pVaRetval->len == 0) goto exit_crypto_create;		//not enough memory, cannot allocate object
		pctx = ((cr_context *)g_pVaRetval->bytes);
		vm_memset(pctx, 0, objLen);					//clear context first before initializing
		cr_init_context(pctx, (uint8 *)g_pVaRetval + objLen + (sizeof(vm_object) -1));			//changed to context on 2015.06.14
		pctx->mode = mode;
		//check icv  
		param = vm_get_argument(3);
		if(param->len != 0) vm_memcpy(pctx->icv, param->bytes, (param->len > CR_ICV_SIZE)?CR_ICV_SIZE:param->len);
		//set context key pointer
		pctx->key = (BYTE*)pctx + sizeof(cr_context);
		if(key->len < keylen) keylen = key->len;
		vm_memcpy(pctx->key, key->bytes, keylen); 	
	}
	exit_crypto_create:
	//OS_DEBUG_EXIT();
	return;
}

static void va_crypto_do(BYTE mode) _REENTRANT_ { 	
	//OS_DEBUG_ENTRY(va_crypto_do);
	vm_object * param;
	vm_object * ctxt;
	uint8 len;
	uint8 * proc_buffer;
	cr_context pctx;
	//CRContext crCtx;	
 	if(vm_get_argument_count() >= 2) {		//return;	
		//check parameter 0, crypto handler
		param = vm_get_argument(0);
		ctxt = vm_get_argument(1);
		if(ctxt->len > 224) {
			vm_invoke_exception(VX_OUT_OF_BOUNDS);
			goto exit_crypto_do;
		}
		proc_buffer = (uint8 *)((cr_context *)param->bytes)->handle;
		if(proc_buffer == NULL) goto exit_crypto_do;
		vm_memset(proc_buffer, 0, (ctxt->len + 16) & 0xF0);
		vm_memcpy(proc_buffer, ctxt->bytes, ctxt->len);
		((cr_context *)param->bytes)->mode = mode | (((cr_context *)param->bytes)->mode & 0x7F);
		vm_memcpy(&pctx, (cr_context *)param->bytes, sizeof(cr_context));
		len = cr_do_crypt(&pctx, 0, ctxt->len);
		g_pVaRetval = vm_create_object(len, proc_buffer);		//return decimal value
	}
	exit_crypto_do:
	//OS_DEBUG_EXIT();
	return;
}

void va_crypto_encrypt() _REENTRANT_ {
 	va_crypto_do(CR_MODE_ENCRYPT);
}

void va_crypto_decrypt() _REENTRANT_ { 
 	va_crypto_do(CR_MODE_DECRYPT);
}

void va_random() _REENTRANT_ {	   
	//OS_DEBUG_ENTRY(va_random);
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint8 len = va_o2f(vm_get_argument(0));			//added 2016.06.08
	if(len != 0) {		//return;
		cr_randomize(bbuf, len);
		g_pVaRetval = vm_create_object(len, bbuf);		//return decimal value
	}
	//OS_DEBUG_EXIT();
}

#if 0 		//DEPRECATED APIS
static void va_crypto_operation(BYTE mode) _REENTRANT_ {
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	vm_object * param;
	vm_object * key;
	uint8 len;	 
	//BYTE plen;
	CRContext crCtx;
	if(vm_get_argument_count() < 3) return;// g_pVaRetval;
	//if((param = vm_get_argument(0)) != (vm_object *)VM_NULL_OBJECT) {
	mode |= (va_o2f(vm_get_argument(0)) & 0x3F);		//0=DES,1=DES2key,2=DES3Key,3=AES			
	//}
	//if((key = vm_get_argument(1)) != (vm_object *)VM_NULL_OBJECT) {			//get cryptokey
	key = vm_get_argument(1);
	//mode = va_o2f(param);	
	//if((param = vm_get_argument(2)) == (vm_object *)VM_NULL_OBJECT) return;// g_pVaRetval;
	param = vm_get_argument(2);
	//start operation in memory	
	//len = param->len;
	//len = ((param->len + 16) & 0xF0);
	vm_memset(g_baOrbBuffer, 0, ((param->len + 16) & 0xF0));		 
	vm_memcpy(g_baOrbBuffer, param->bytes, param->len);
	crInitContext(&crCtx, g_baOrbBuffer);			   //changed 2015.06.14
	crCtx.mode = mode;
	crCtx.key = key->bytes;
	//len = crMemOperation(mode, key->bytes, param->bytes, param->len, g_baOrbBuffer);
	len = crDoCrypt(&crCtx, 0, param->len);
	g_pVaRetval = vm_create_object(len, g_baOrbBuffer);		//return decimal value		
}

//codec
void va_encrypt_ecb() _REENTRANT_ {
 	va_crypto_operation(CR_MODE_ENCRYPT | CR_MODE_ECB);	
}

void va_decrypt_ecb() _REENTRANT_ {
	va_crypto_operation(CR_MODE_DECRYPT | CR_MODE_ECB);
} 

void va_encrypt_cbc() _REENTRANT_ {
 	va_crypto_operation(CR_MODE_ENCRYPT | CR_MODE_CBC);	
}

void va_decrypt_cbc() _REENTRANT_ {
	va_crypto_operation(CR_MODE_DECRYPT | CR_MODE_CBC);
}
#endif			//END DEPRECATED APIS

#if STACK_TOOLKIT_SELECT_APIS
void va_select_item() _REENTRANT_ {	   //SELECT ITEM
	vm_object * param;
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	uint8 i = 0, j;
	uint8 len;					
	//check if qualifier contain DCS
	//i = tkPrintf("cd", STK_CMD_SELECT_ITEM, 0, STK_DEV_ME);
	i = tkPushHeaderB(STK_CMD_SELECT_ITEM, 0, STK_DEV_ME);
	param = vm_get_argument(0);
	if(param->len != 0) {
		len = vrConvertText(param->bytes, param->len, g_baOrbBuffer + i, TK_BUFFER_SIZE - i);
		i = tkPushBufferB(i, STK_TAG_ALPHA, len, g_baOrbBuffer + i);	
	}
	for(j=1;j<vm_get_argument_count();j++) {
		param = vm_get_argument(j);	   
		g_baOrbBuffer[i] = j; 	//item id	
		len = 1;
		len += vrConvertText(param->bytes, param->len, g_baOrbBuffer + (i + 1), TK_BUFFER_SIZE - (i + 1));
		i = tkPushBufferB(i, STK_TAG_ITEM, len, g_baOrbBuffer + i);
	} 
	tkDispatchCommandW(NULL, i);
	g_vaWaitTag = STK_TAG_ITEM_ID;
	vm_set_state(VM_STATE_SUSPEND);
	//return g_pVaRetval;
}
#endif

#if STACK_TOOLKIT_TIMER_APIS
void va_set_timer() _REENTRANT_ {  
	//vm_object * g_pVaRetval = (vm_object *)VM_NULL_OBJECT; 
	FSHandle temp_fs;	   
	vm_object * param;
	//vm_context vctx; 
	TKApplicationContext actx;
	BYTE buffer[18];
	BYTE l, h;
	//BYTE dbuffer[3];
	BYTE tid;
	BYTE tag, hlen;
	WORD tsize;
	BYTE k;
	WORD index;
	WORD i, codestart;
	WORD tick;
	//set timer initial value  
	mmMemSet(buffer + 2, 0, 3);		//clear timer value to 000000
	if((param = vm_get_argument(1))	== VM_NULL_OBJECT) return;
	tick = va_o2f(param);	 
	for(k=3;k!=1;k--) {
		buffer[k+1] = (tick % 60);
		tick /= 60;
	}
	buffer[2] = tick;
	for(k=3;k!=0;k--) {
		l = buffer[k+1] % 10;
		h = buffer[k+1] / 10;
		buffer[k+1] = (h << 4) | (l & 0x0F);
	}
	//} else return g_pVaRetval;
	//timer id
	if((param = vm_get_argument(0))	!= VM_NULL_OBJECT) {
		if(param->len > 2) return;// g_pVaRetval;
		index = param->bytes[0];
		index <<= 8;
		index |= param->bytes[1];
	} else return;// g_pVaRetval;
	vm_memcpy(&temp_fs, &_vm_file, sizeof(FSHandle));

	//decode ASN.1 structure for package and look for entry point for matching menu_id
	hlen = tkPopFile(&temp_fs, 0, &tag, &tsize);
	if(tag == (ASN_TAG_SEQ | ASN_TAG_CONSTRUCTED)) {		//check for constructed sequence
		codestart = tsize + hlen;										//total sequence length
		//re-initialize vm to execute current selected menu
		vm_memcpy(&actx, &temp_fs, sizeof(FSHandle));		//set handle to current active bytecode where this api executed
		actx.offset = codestart + index;
#if TK_MANAGER_ENABLED
		tid = tkRegisterServiceToTimer(TK_SID_STACK, sizeof(TKApplicationContext), &actx);
#else
		
#endif
		if(tid != 0) {
			i = tkPushHeaderB(STK_CMD_TIMER_MANAGEMENT, 0, STK_DEV_ME);
			i = tkPushBufferB(i, STK_TAG_TIMER_IDENTIFIER, 1, &tid);
			i = tkPushBufferB(i, STK_TAG_TIMER_VALUE, 3, buffer + 2);
			tkDispatchCommandW(NULL, i);	
			g_vaWaitTag = STK_TAG_RESULT;
			vm_set_state(VM_STATE_SUSPEND);	
		}
	}
	//return g_pVaRetval;
}
#endif

#define VA_BIT_CHECK		3
#define VA_BIT_SET		   	1
#define VA_BIT_CLR			2

#if STACK_BIT_APIS
static void va_bit_operation(BYTE mode) {
	vm_object * param;
	uint16 bnum;
	uint8 offset, mask;//, res;
	//if(vm_get_argument_count() <  2) return;// g_pVaRetval;
	param = vm_get_argument(0);
	bnum = va_o2f(vm_get_argument(1));		//bit number
	offset = bnum / 8;	//byte number
	mask = bnum % 8;	//mask
	mask = 1 << mask;
	if(param->len <= offset) return;// g_pVaRetval;
	switch(mode) {
		case VA_BIT_CHECK:
			bnum = '0';
			if(param->bytes[offset] & mask) {
				bnum = '1';	
			}
			g_pVaRetval = vm_create_object(1, &bnum);
			break;
		case VA_BIT_SET:
			param->bytes[offset] |= mask;
			break;
		case VA_BIT_CLR:
			param->bytes[offset] &= ~mask;
			break;
		default: break;
	}
	//return g_pVaRetval;
}

void va_check_bit() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_check_bit);
	va_bit_operation(VA_BIT_CHECK);
	//OS_DEBUG_EXIT();
}

void va_set_bit() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_set_bit);
	va_bit_operation(VA_BIT_SET);
	//OS_DEBUG_EXIT();
}

void va_clear_bit() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_clear_bit);
	va_bit_operation(VA_BIT_CLR);
	//OS_DEBUG_EXIT();
} 
#endif

#if 0
void va_arg_findtag() _REENTRANT_ {
	uint8 len;		 
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	vm_object * obj = vm_get_argument(0);
	uint8 tag = va_o2f(vm_get_argument(1));
	//len = mmPopTlvByTag(obj->bytes, obj->len, tag, bbuf);
	len = tk_pop_by_tag(obj->bytes, obj->len, tag, bbuf);
	if(len == (uint8)-1) return;
	g_pVaRetval = vm_create_object(len, bbuf);
}
#endif

void va_arg_count() _REENTRANT_ {
	uint8 cntr = 0;
	uint8 tag, i, tlen, len;
	vm_object * obj = vm_get_argument(0);
 	if(obj->len == 0) return;
	if(obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ ) return;		//invalid array/object mark	
	tlen = obj->len;
	for(i=2;i<tlen;cntr++) {			 
		tag = obj->bytes[i];
		len = obj->bytes[i + 1];
		i += (len + 2);
	}	
	va_return_word(cntr);
}
  
void va_arg_create() _REENTRANT_ {
	//uint16 len = strlen((const char *)key) + strlen((const char *)value) + 2;
	//OS_DEBUG_ENTRY(va_arg_create);
	vm_object * obj;
	vm_object * key = vm_get_argument(0);
	vm_object * val = vm_get_argument(1);
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	uint16 len = key->len + val->len + 3;
	if(len <= 250) {		//return;

		bbuf[0] = ASN_TAG_OBJDESC;	  
		bbuf[1] = len - 2;
		vm_memcpy(bbuf + 2, key->bytes, key->len);
		bbuf[key->len + 2] = VA_OBJECT_DELIMITER; 
		vm_memcpy(bbuf + key->len + 3, val->bytes, val->len);
		g_pVaRetval = vm_create_object(len, bbuf);
		if(g_pVaRetval->len != 0) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;				//set to object
	}
	//OS_DEBUG_EXIT();
}

void va_arg_object() _REENTRANT_ {
	//va_list ap;
	//OS_DEBUG_ENTRY(va_arg_object);
    uint8 j;
	vm_object * obj, * ibj;
	uint16 len = 0;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
    //double sum = 0;
	uint8 count =  vm_get_argument_count();
	//cid = va_o2f(vm_get_argument(0));

    //va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    for (j = 0; j < count; j++) {
        //obj = va_arg(ap, vm_object *);		/* Increments ap to the next argument. */
		obj = vm_get_argument(j);
		len += obj->len;
	}
	if(len <= 250) {		//return;
		//va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
		//obj = (vm_object *)malloc(len + sizeof(vm_object));
		len = 2;
		for (j = 0; j < count; j++) {
			//ibj = va_arg(ap, vm_object *);   
			ibj = vm_get_argument(j);
			if((ibj->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
				//check for OWB object
				vm_memcpy(bbuf + len, ibj->bytes, ibj->len);
				len += ibj->len;
			} else {
				//default string object
				vm_memcpy(bbuf + len + 2, ibj->bytes, ibj->len);
				bbuf[len] = ASN_TAG_OCTSTRING;
				bbuf[len + 1] = ibj->len;
				len += (ibj->len + 2);
			}
		}
		bbuf[0]= ASN_TAG_SEQ;	//'*';		//object mark 
		bbuf[1]= len - 2;	//'*';		//actual object content length (not incuding header)
		//va_end(ap);
		g_pVaRetval = vm_create_object(len, bbuf);
		if(g_pVaRetval->len != 0) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
	}
	//OS_DEBUG_EXIT();
}
							   
void va_arg_array() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_array);
    uint8 j;
	vm_object * obj, * ibj;
	uint16 len = 0;
    //double sum = 0;	
	uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	uint8 count =  vm_get_argument_count();

    //va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
    for (j = 0; j < count; j++) {
        //obj = va_arg(ap, vm_object *);		/* Increments ap to the next argument. */ 
		obj = vm_get_argument(j);
		len += obj->len;
	}
	if(len <= 250) {	//return;
		//va_start(ap, count);					/* Requires the last fixed parameter (to get the address) */
		//obj = (vm_object *)malloc(len + sizeof(vm_object));
		len = 2;
		for (j = 0; j < count; j++) {
			//ibj = va_arg(ap, vm_object *);	  
			ibj = vm_get_argument(j);
			if((ibj->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
				//check for OWB object
				vm_memcpy(bbuf + len, ibj->bytes, ibj->len);
				len += ibj->len;
			} else {
				//default string object
				vm_memcpy(bbuf + len + 2, ibj->bytes, ibj->len);
				bbuf[len] = ASN_TAG_OCTSTRING;
				bbuf[len + 1] = ibj->len;
				len += (ibj->len + 2);
			}
		}
		bbuf[0]= ASN_TAG_SET;	//'#';		//array mark
		bbuf[1]= len - 2;	//'*';		//actual object content length (not incuding header)
		//va_end(ap);
		g_pVaRetval = vm_create_object(len, bbuf);
		if(g_pVaRetval->len != 0) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
	}
	//OS_DEBUG_EXIT();
}

void va_arg_at() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_at);
	uint8 i;
	uint8 len;
	uint8 j;
	uint8 tlen;
	uint8 tag;
	vm_object * ibj;		 
	vm_object * obj = vm_get_argument(0);
	uint8 index = va_o2f(vm_get_argument(1));
	if(obj->len != 0 && obj->bytes[0] == ASN_TAG_SET) {		//return;		//invalid array mark
		//obj = obj->bytes;
		tlen = obj->bytes[1];
		for(i=2,j=0;i<tlen;j++) {
			tag = obj->bytes[i];
			len = obj->bytes[i + 1];
			if(j == index) { 
				if(tag == ASN_TAG_OCTSTRING) {
					g_pVaRetval = vm_create_object(len, obj->bytes + i + 2);
				} else {
					g_pVaRetval = vm_create_object(len + 2, obj->bytes + i);
					if(g_pVaRetval->len != 0) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
				}
				break;
			}
			i += (len + 2);
		}
	}
	//OS_DEBUG_EXIT();
	//return VM_NULL_OBJECT;		//index out of bounds
}
			  
void va_arg_get() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_get);
	uint8 i = 1;
	uint8 len;
	uint8 j;   
	uint8 tlen;
	uint8 tag;
	vm_object * ibj;		
	vm_object * obj = vm_get_argument(0);
	vm_object * key = vm_get_argument(1);
	if(obj->len != 0) {
		if(obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ ) goto exit_arg_get;		//invalid array/object mark	  
		//obj = obj->bytes;
		tlen = obj->bytes[1];
		for(i=2;i<tlen;) {			 
			tag = obj->bytes[i];
			len = obj->bytes[i + 1];
			if(tag == ASN_TAG_OBJDESC && obj->bytes[i + 2 + key->len] == VA_OBJECT_DELIMITER  && vm_memcmp(&obj->bytes[i + 2], key->bytes, key->len) == 0 ) {
				len = len - (key->len + 1);
				g_pVaRetval = vm_create_object(len, &obj->bytes[i + 2] + key->len + 1);
				if(g_pVaRetval->len != 0) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
				break;
			}
			i += (len + 2);
		}
	}
	exit_arg_get:
	//OS_DEBUG_EXIT();
	return;
}

static uint8 va_arg_serialize_s(uint8 * buffer, uint8 ** strbuf, uint8 objlen) _REENTRANT_ {
	uint8 c;
	uint8 i, plen;
	uint8 j = 0;
	uint8 len = 0;
	uint8 state =0;
	uint8 wc;
	while(j++ < objlen && (c = *(strbuf[0])++)) {
		switch(c) {
			case ASN_TAG_SET:		//array		ASN_TAG_SET
				state=0;
				buffer[len++]= '[';
				c = *(strbuf[0])++;
				j++;
				len += va_arg_serialize_s(buffer + len, strbuf, c);
				j += c;
				buffer[len++]= ']';
				break;
			case ASN_TAG_SEQ:		//object	ASN_TAG_SEQ	
				state=0;
				buffer[len++]= '{';
				c = *(strbuf[0])++;
				j++;
				len += va_arg_serialize_s(buffer + len, strbuf, c);
				j += c;
				buffer[len++]= '}';
				break;
			case VA_OBJECT_DELIMITER:
				if(state == 1) {
					state++;
					buffer[len++]= '\"';
				}
				buffer[len++]= ':';
				break;
			case ASN_TAG_OBJDESC:					//ASN_TAG_OBJDESC
				if(state == 0) { 
					if(len != 0) buffer[len++]= ',';
					c = *(strbuf[0])++;
					j++;
					len += va_arg_serialize_s(buffer + len, strbuf, c);
					j += c;
				} 
				break;
			case ASN_TAG_OCTSTRING:
				if(state == 0) { 
					if(len != 0) buffer[len++]= ',';
					c = *(strbuf[0])++;
					j++;
					buffer[len++] = '\"';
					memcpy(buffer + len, strbuf[0], c);
					(strbuf[0]) += c;
					len += c;
					j+=c;
					buffer[len++] = '\"';
				}
				break;
			case ' ': 
				if(state == 0) break;
				if(state == 4) break;
			default:
				if(state == 2 || state == 0) {
					state++;
					if(state == 2 && c >='0' && c <='9') {
						state=4;		//numeric state
					} else 
						buffer[len++]= '\"';
				}
				switch(c) {
					case '\"': wc = '\"'; goto push_spchar;
					case '\\': wc = '\\'; goto push_spchar;
					case '/' : wc = '/'; goto push_spchar;
					case '\b': wc = 'b'; goto push_spchar;
					case '\f': wc = 'f'; goto push_spchar;
					case '\n': wc = 'n'; goto push_spchar;
					case '\r': wc = 'r'; goto push_spchar;
					case '\t': wc = 't'; goto push_spchar;
						push_spchar:
						buffer[len++]= '\\';
						buffer[len++]= wc;
						break;
					default:
						buffer[len++]= c;
						break;
				}
				break;
		}
	}
	if((state & 0x01) != 0) buffer[len++]= '\"';
	return len;
}

void va_arg_serialize() _REENTRANT_ {			// -> to json string
	//OS_DEBUG_ENTRY(va_arg_serialize);
	uint8 dlen;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	vm_object * obj = vm_get_argument(0);
	uint8 * tptr = obj->bytes;
	if((obj->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {		//return;		//should be OWB object
		//printf("to_json");
		dlen = va_arg_serialize_s(bbuf, &tptr, obj->len);	 
		g_pVaRetval = vm_create_object(dlen, bbuf);
	}
	//OS_DEBUG_EXIT();
}

static uint8 va_arg_deserialize_s(uint8 * buffer, uint8 ** strbuf, uint8 slen, uint8 * index) _REENTRANT_ {
	uint8 c;
	uint8 len = 0;
	uint8 ilen;
	uint8 state =0;
	uint8 ldx;
	uint8 klen=0;
	while(index[0]++ < slen && (c = *(strbuf[0])++)) {
		switch(c) {
			case '[':		//start array
				buffer[len++] = ASN_TAG_SET;
				ilen = va_arg_deserialize_s(buffer + len + 1, strbuf, slen, index);
				buffer[len++] = ilen;
				len += ilen;
				break;
			case '{':		//start object
				buffer[len++] = ASN_TAG_SEQ;
				ilen = va_arg_deserialize_s(buffer + len + 1, strbuf, slen, index);
				buffer[len++] = ilen;
				len += ilen;
				break;
			case '}':		//end object
			case ']':		//end array
			case 0:			//end string
				return len;
			case '\"':		//start key-value
				switch(state) {
					case 0: buffer[len++]=ASN_TAG_OBJDESC; ldx = len++; klen = 0; break;
					case 1: buffer[ldx] = klen; break;
					case 2: break;
					case 3: buffer[ldx] = klen; break;
					case 4: goto reset_state;
				}
				state++;
				break;
			case ',':
				if(state != 1 || state != 3) { 			//don't skip comma when inside quotation mark
					state = 0;
				} else goto push_char;
				break;
			case ':':
			default:
			push_char:
				klen++;
				buffer[len++] = c;
				if(state == 2) {					   //fixed for "key":number		(2017.06.07)
					if(c >= '0' && c <= '9') state=4;
				}
				if(state == 4) {
				 	if(c >= '0' && c <= '9') 
						buffer[ldx] = klen;
					else 
						goto reset_state;
				}
				break;
			case ' ': 
				if(state == 1 || state == 3) goto push_char; 		//inside quotation mark
				if(state == 4) {  
			reset_state:
					state = 0;
				}
				break;	//skip white space
		}
	}
	return len;
}

void va_arg_deserialize() _REENTRANT_ {	//-> from json string
	//OS_DEBUG_ENTRY(va_arg_deserialize);
	uint8 dlen;
	uint8 t = 0;		
	uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	vm_object * obj = vm_get_argument(0);
	uint8 * tptr = obj->bytes;	   
	dlen = va_arg_deserialize_s(bbuf, &tptr, obj->len, &t);
	g_pVaRetval = vm_create_object(dlen, bbuf);
	if(g_pVaRetval->len != 0) g_pVaRetval->mgc_refcount |= VM_OBJ_MAGIC;
	//OS_DEBUG_EXIT();
}

void va_arg_set_operation(vm_object * obj, vm_object * key, vm_object * val) _REENTRANT_  {
   	uint8 i = 1;
	uint8 len;
	uint8 j;   
	uint8 tlen;
	uint8 tag;
	uint8 index = -1;
	uint8 cntr = 0;
	uint8 dlen = 0;		
	uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	if(obj->len == 0) goto exit_arg_set;
	if(obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ ) goto exit_arg_set;		//invalid array/object mark	  
	if(vm_is_numeric(key->bytes, key->len)) index = va_o2f(key);
	//obj = obj->bytes;
	tlen = obj->len;
	memcpy(bbuf, obj->bytes, obj->len);
	dlen = 2;
	for(i=2;i<tlen;cntr++) {			 
		tag = obj->bytes[i];
		len = obj->bytes[i + 1];
		if(index == (uint8)-1) {			//search by key
			if(tag == ASN_TAG_OBJDESC && obj->bytes[i + 2 + key->len] == VA_OBJECT_DELIMITER  && memcmp(&obj->bytes[i + 2], key->bytes, key->len) == 0 ) {
				if(val->len == 0) goto skip_operation;		//deletion
				bbuf[dlen] = tag;
				memcpy(bbuf + dlen + 2, key->bytes, key->len);
				bbuf[dlen + 2 + key->len] = VA_OBJECT_DELIMITER;		
				memcpy(bbuf + dlen + 3 + key->len, val->bytes, val->len);
				bbuf[dlen + 1] = (key->len + val->len + 1); 
				dlen += (key->len + val->len + 3);
			} else 
				goto normal_operation;
		} else {
		 	if(cntr == index) {
				if(val->len == 0) goto skip_operation;		//deletion
				if((val->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
					memcpy(bbuf + dlen, val->bytes, val->len);
					dlen += val->len;
				} else {
					bbuf[dlen] = ASN_TAG_OCTSTRING;
					bbuf[dlen + 1] = val->len;
				   	memcpy(bbuf + dlen + 2, val->bytes, val->len); 
					dlen += (val->len + 2);
				}
			} else {
				normal_operation:
				memcpy(bbuf + dlen, obj->bytes + i, len + 2);
				dlen += (len + 2);
			}	
		}
		skip_operation:
		i += (len + 2);
	}
	bbuf[1] = (dlen - 2);										//set object to new length
	//return on context	
	g_pVaRetval = vm_create_object(dlen, bbuf); 
	if(g_pVaRetval->len != 0) {
		g_pVaRetval->mgc_refcount = VM_OBJ_MAGIC | ((obj->mgc_refcount + 1) & 0x0F);		//copy header bytes, set to object in case didn't
		vm_update_mutator(obj, g_pVaRetval);						//update mutator
		obj->mgc_refcount &= 0xF0;									//clear refcount
		vm_release_object(obj);										//release header
	}
	exit_arg_set:
	return;
}

void va_arg_add() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_add);
	uint8 i = 1;
	uint8 len;
	uint8 index = -1;
	uint8 cntr = 0;
	uint8 dlen = 0;	
	uint8 bbuf[VA_OBJECT_MAX_SIZE];	
	vm_object * obj = vm_get_argument(0);
	vm_object * val = vm_get_argument(1);	
	if(obj->len == 0) goto exit_arg_add;
	if(obj->bytes[0] != ASN_TAG_SET && obj->bytes[0] != ASN_TAG_SEQ ) goto exit_arg_add;		//invalid array/object mark
	memcpy(bbuf, obj->bytes, obj->len);
	dlen = obj->len;
	if((val->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
		memcpy(bbuf + dlen, val->bytes, val->len);
		dlen += val->len;
	} else {
		bbuf[dlen] = ASN_TAG_OCTSTRING;
		bbuf[dlen + 1] = val->len;
	   	memcpy(bbuf + dlen + 2, val->bytes, val->len); 
		dlen += (val->len + 2);
	}
	bbuf[1] = (dlen - 2);										//set object to new length
	//return on context	
	g_pVaRetval = vm_create_object(dlen, bbuf); 
	if(g_pVaRetval->len != 0) {
		g_pVaRetval->mgc_refcount = VM_OBJ_MAGIC | ((obj->mgc_refcount + 1) & 0x0F);		//copy header bytes, set to object in case didn't
		vm_update_mutator(obj, g_pVaRetval);						//update mutator
		obj->mgc_refcount &= 0xF0;									//clear refcount
		vm_release_object(obj);										//release header
	}
	exit_arg_add:
	//OS_DEBUG_EXIT();
	return;
}

void va_arg_set() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_set);
 	va_arg_set_operation(vm_get_argument(0), vm_get_argument(1), vm_get_argument(2));
	//OS_DEBUG_EXIT();
}
		
void va_arg_remove() _REENTRANT_ {
	//OS_DEBUG_ENTRY(va_arg_remove);
 	va_arg_set_operation(vm_get_argument(0), vm_get_argument(1), (vm_object *)"\xE0\x00\x00\x00");
	//OS_DEBUG_EXIT();
}

void va_set_execution_context() _REENTRANT_ {
	//extern BYTE g_vmExecContext;
 	//if(vm_get_argument_count() > 0) {
	//	g_vmExecContext = va_o2f(vm_get_argument(0));		//tag to wait (if exist)
	//}  	
}

void va_bytes() _REENTRANT_ {	   //SELECT ITEM  
	//OS_DEBUG_ENTRY(va_bytes);
	uint8 i, j, k;
	uint8 len;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	vm_object * param;
	j = vm_get_argument_count();
	//if(j == 0) return;// g_pVaRetval;
	for(i=0,k=0;i<j;i++) { 
		param = vm_get_argument(i);
		bbuf[k++] = va_o2f(param);	
	}
	g_pVaRetval = vm_create_object(k, bbuf); 
	//OS_DEBUG_EXIT();
}
