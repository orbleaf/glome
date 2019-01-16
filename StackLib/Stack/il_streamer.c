#include "../defs.h"
#include "../config.h"
#include "il_streamer.h"
#include "asm_streamer.h"
#include "sym_table.h"
#include "il1_optimizer.h"
#include "il2_optimizer.h"
#include "pkg_encoder.h"
#include <stdarg.h>
#include <stdio.h>

uchar _istream_code_buffer[MAX_BUFFER_SIZE];
uchar _istream_data_buffer[MAX_BUFFER_SIZE];
#ifdef STANDALONE_COMPILER
FILE * _ilfile = NULL;
#endif
const char * _ins_name[];
uint32 _cpindex = 0;
uint32 _istream_code_size = 0;
uint32 _current_offset = 0;

#if IS_CODEDEBUG
static void is_debug(uint32 offset, uchar * str, ...) {
	if(offset >= _current_offset) {
		//_current_offset = offset;
		printf("%08x:%s\n", offset, str);
		fflush(0);
	}
}
#endif

void is_init(uchar * inpath) {		//original input file path as parameter
#ifdef STANDALONE_COMPILER
	uchar ilpath[512];
	uchar index = 0;
#endif
	_cpindex = 0;
	_istream_code_size = 0;
#ifdef STANDALONE_COMPILER
	if(inpath == NULL) return;
	strcpy(_RECAST(char *, ilpath), _RECAST(const char *, inpath));
	index = (uint32)strchr(_RECAST(const char *, ilpath), '.') - (uint32)ilpath;
	ilpath[index] = 0;
	sprintf(_RECAST(char *, ilpath), "%s%s", ilpath, ".bin");
	//_ilfile = fopen(ilpath, "wb");
	pk_init(inpath);
#endif
}

uint32 is_push_constant(uint16 size, uchar * value) {
	uint16 retval = _cpindex;
	if(size < 128) {
		_istream_data_buffer[_cpindex++] = size;
	} else if(size < 256) {
		_istream_data_buffer[_cpindex++] = 0x81;
		_istream_data_buffer[_cpindex++] = size;
	} else if(size < 65536) {
		_istream_data_buffer[_cpindex++] = 0x82;
		_istream_data_buffer[_cpindex++] = (size >> 8);
		_istream_data_buffer[_cpindex++] = size & 0xFF;
	}
	memcpy(_istream_data_buffer + _cpindex, value, size);
	_cpindex += size;
	//printf("push constant %08x : %s %d\n", retval, value, size);
	return retval;
}

uint32 is_gencode(uint32 offset, uchar opcode, ...) {
	va_list argptr;
	uint32 szinst = 0;
	symrec * rec;
	uchar dbgbuf[1024];
	uint16 index = 0;
	va_start(argptr, opcode);
	memset(dbgbuf, 0, sizeof(dbgbuf));
	switch(opcode) {
		case INS_LBL:					//create new label
			rec = va_arg(argptr, symrec *);
			if(rec == NULL) { szinst = 0; break; }
			/*szinst = strlen(rec->name);
			_istream_code_buffer[offset] = opcode; 
			_istream_code_buffer[offset + 1] = (uchar)szinst;
			memcpy(_istream_code_buffer + offset + 2, rec->name, szinst);
			szinst += 2;*/
			//printf("%d\n", _istream_code_buffer[offset + 1] + 2);
			is_push_lblrec(offset, rec->name);
			sprintf(_RECAST(char *, dbgbuf), "%s:", rec->name);
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			//printf("%d\n", _istream_code_buffer[offset]);
			szinst = 0;				//predirective instruction (not implemented)
			break;
		case INS_OBJPUSH:
		case INS_OBJPOP:
		case INS_OBJSTORE:
			rec = va_arg(argptr, symrec *);
			index = rec->offset;
			_istream_code_buffer[offset] = opcode;
			_istream_code_buffer[offset + 1] = (uchar)index;
			sprintf(_RECAST(char *, dbgbuf), "\t\t%s %s", _ins_name[opcode], rec->name);
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			szinst = 2;
			break;
		case INS_SWITCH:
			rec = va_arg(argptr, symrec *);
			index = rec->offset;
			_istream_code_buffer[offset] = opcode;
			*((uint16 * )(_istream_code_buffer + offset + 1)) = end_swap16(index);
			sprintf(_RECAST(char *, dbgbuf), "\t\t%s %i", _ins_name[opcode], rec->offset);
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			szinst = 3;
			break;
		case INS_OBJDEL:
		case INS_OBJSUB:
		case INS_OBJSZ:
		case INS_OBJDUP:
		case INS_ADD:
		case INS_SUB:
		case INS_MUL:
		case INS_DIV:
		case INS_MOD:
		case INS_AND:
		case INS_OR:
		case INS_XOR:
		case INS_NOT:
		case INS_RET:
		case INS_NOP:
		case INS_CREQ:			//64	//-> compare equal 
		case INS_CRNE:			//65	//-> compare not equal
		case INS_CRGT:			//66	//-> compare greater than 
		case INS_CRLT:			//67	//-> compare less than
		case INS_CRGTEQ:			//68	//-> compare greater than
		case INS_CRLTEQ:			//69	//-> compare less than 
			_istream_code_buffer[offset] = opcode;
			sprintf(_RECAST(char *, dbgbuf), "\t\t%s", _ins_name[opcode]);
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			szinst = 1;
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
			index = va_arg(argptr, uint32);
			_istream_code_buffer[offset] = opcode;
			_istream_code_buffer[offset + 1] = (uchar)index;
			sprintf(_RECAST(char *, dbgbuf), "\t\t%s_%d %d", _ins_name[opcode], (uint32)opcode & 0x0F, index);
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			szinst = 2;
			break;
		case INS_SCTX:
		case INS_RCTX:
			index = va_arg(argptr, uint32);
			_istream_code_buffer[offset] = opcode;
			_istream_code_buffer[offset + 1] = (uchar)index;
			sprintf(_RECAST(char *, dbgbuf), "\t\t%s %d", _ins_name[opcode], index);
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			szinst = 2;
			break;
		case INS_F2O_0:
		case INS_F2O_1:
		case INS_F2O_2:
		case INS_F2O_3:
		case INS_F2O_4:
		case INS_F2O_5:
		case INS_F2O_6:
		case INS_F2O_7:
		case INS_F2O_8:
		case INS_F2O_9:
		case INS_F2O_10:
		case INS_F2O_11:
		case INS_F2O_12:
		case INS_F2O_13:
		case INS_F2O_14:
		case INS_F2O_15:
		case INS_CALL:
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
		case INS_OBJCONST:
		case INS_JMP:					//jump to specified label
		case INS_JTRUE:
		case INS_JFALSE:
			rec = va_arg(argptr, symrec *);
			index = rec->offset;
			_istream_code_buffer[offset] = opcode;
			*((uint16 * )(_istream_code_buffer + offset + 1)) = end_swap16(index);
			if(strlen((char *)rec->name) != 0) {
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s %s", _ins_name[opcode], rec->name);
			} else {
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s %s", _ins_name[opcode], "(null)");
			}
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			szinst = 3;
			break;
		case INS_OBJNEW:
			rec = va_arg(argptr, symrec *);
			index = rec->offset;
			_istream_code_buffer[offset] = opcode;
			*((uint16 * )(_istream_code_buffer + offset + 1)) = end_swap16(index);
			if(strlen((char *)rec->name) != 0) {
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s %s", _ins_name[opcode], rec->name);
			} else {
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s %s", _ins_name[opcode], "(null)");
			}
			#if IS_CODEDEBUG
			is_debug(offset, dbgbuf);
			#endif
			as_print(offset, dbgbuf);
			szinst = 3;
			break;

		default: break;
	}
	va_end(argptr);              /* Reset variable argument list. */
	if((offset + szinst) > _istream_code_size) {
		_istream_code_size = (offset + szinst);
	}
	return szinst;
}

static uint32 is_jumptable_reloc(uint32 pc, uint32 jt_offset, uint32 offset2add) {
	uint32 i = 2, index;
	uint32 cp_size = _istream_data_buffer[jt_offset];
	jt_offset += 1;				//cp size		
	#if IS_REL_JUMP_ADDRESS
	index = end_swap16(*((uint16 * )(_istream_data_buffer + jt_offset)));
	*((uint16 * )(_istream_data_buffer + jt_offset)) = end_swap16(index - pc);
	#endif
	while(i<cp_size) {
		index = end_swap16(*((uint16 * )(_istream_data_buffer + jt_offset + i)));
		index += offset2add;
		#if IS_REL_JUMP_ADDRESS
		*((uint16 * )(_istream_data_buffer + jt_offset + i)) = end_swap16(index - pc);
		#else
		*((uint16 * )(_istream_data_buffer + jt_offset + i)) = end_swap16(index);
		#endif

		index = end_swap16(*((uint16 * )(_istream_data_buffer + jt_offset + i + 2)));
		#if IS_REL_JUMP_ADDRESS
		*((uint16 * )(_istream_data_buffer + jt_offset + i + 2)) = end_swap16(index - pc);
		#else
		*((uint16 * )(_istream_data_buffer + jt_offset + i + 2)) = end_swap16(index);
		#endif
		i += 4;
	}
	return i;
}

static uint32 is_link_code(uint32 size) {
	uint32 pc = 0;
	uchar opcode = pc;
	uint16 index;
	uchar dbgbuf[280];
	lblrec * curlbl = NULL;
	#if IS_CODEDEBUG
	_current_offset = 0;
	#endif
	#if IS_CODEDEBUG
	printf("\nLink code and data\n\n");
	#endif
	while(pc != size) {
		opcode = _istream_code_buffer[pc];
		curlbl = is_get_lblrec(pc);
		if(curlbl != NULL) {
			sprintf(_RECAST(char *, dbgbuf), "%s:", curlbl->name);
			#if IS_CODEDEBUG
			is_debug(pc, dbgbuf);
			#endif
			as_print(pc, dbgbuf);
			//update package method record
			if(curlbl->tag != NULL) {
				((pk_method *)curlbl->tag)->offset = pc;
			}
		}
		//printf("link %s\n", _ins_name[opcode]);
		switch(opcode) {
			case INS_LBL:					//create new label
				pc += _istream_code_buffer[pc + 1];
				pc += 2;
				break;
			case INS_OBJPUSH:
			case INS_OBJPOP:
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s %d", _ins_name[opcode], (uint32)_istream_code_buffer[pc + 1]);
				#if IS_CODEDEBUG
				is_debug(pc, dbgbuf);
				#endif
				as_print(pc, dbgbuf);
				pc += 2;
				break;
			case INS_F2O_0:
			case INS_F2O_1:
			case INS_F2O_2:
			case INS_F2O_3:
			case INS_F2O_4:
			case INS_F2O_5:
			case INS_F2O_6:
			case INS_F2O_7:
			case INS_F2O_8:
			case INS_F2O_9:
			case INS_F2O_10:
			case INS_F2O_11:
			case INS_F2O_12:
			case INS_F2O_13:
			case INS_F2O_14:
			case INS_F2O_15:
				index = end_swap16(*((uint16 * )(_istream_code_buffer + pc + 1)));					//relocate index
				//#if IS_REL_JUMP_ADDRESS
				//*((uint16 * )(_istream_code_buffer + pc + 1)) = end_swap16(index - pc);
				//#endif
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s 0x%x", _ins_name[opcode], (uint32)index);
				#if IS_CODEDEBUG
				is_debug(pc, dbgbuf);
				#endif
				as_print(pc, dbgbuf);
				pc += 3;
				break;
			case INS_CALL:
			case INS_JMP:					//jump to specified label
			case INS_JFALSE:
			case INS_JTRUE:
				index = end_swap16(*((uint16 * )(_istream_code_buffer + pc + 1)));	//relocate index
				#if IS_REL_JUMP_ADDRESS
				*((uint16 * )(_istream_code_buffer + pc + 1)) = end_swap16(index - pc);
				#endif
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s 0x%x", _ins_name[opcode], (uint32)index);
				#if IS_CODEDEBUG
				is_debug(pc, dbgbuf);
				#endif
				as_print(pc, dbgbuf);
				pc += 3;
				break;
			case INS_SWITCH:
				is_jumptable_reloc(pc, end_swap16(*((uint16 * )(_istream_code_buffer + pc + 1))), size);		//relocate jumptable contents
			case INS_OBJNEW:	
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
			case INS_OBJCONST:
				index = end_swap16(*((uint16 * )(_istream_code_buffer + pc + 1)));//size;					//relocate index
				index += size;
				#if IS_REL_JUMP_ADDRESS
				*((uint16 * )(_istream_code_buffer + pc + 1)) = end_swap16(index - pc);
				#else
				*((uint16 * )(_istream_code_buffer + pc + 1)) = end_swap16(index);
				#endif
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s 0x%x", _ins_name[opcode], (uint32)index);
				#if IS_CODEDEBUG
				is_debug(pc, dbgbuf);
				#endif
				as_print(pc, dbgbuf);
				pc += 3;
				break;
			case INS_OBJDEL:
			case INS_OBJSUB:
			case INS_OBJSZ:
			case INS_OBJDUP:
			case INS_ADD:
			case INS_SUB:
			case INS_MUL:
			case INS_DIV:
			case INS_MOD:
			case INS_AND:
			case INS_OR:
			case INS_XOR:
			case INS_NOT:
			case INS_RET:
			case INS_NOP:
			case INS_CREQ:			//64	//-> compare equal 
			case INS_CRNE:			//65	//-> compare not equal
			case INS_CRGT:			//66	//-> compare greater than 
			case INS_CRLT:			//67	//-> compare less than
			case INS_CRGTEQ:			//68	//-> compare greater than
			case INS_CRLTEQ:			//69	//-> compare less than 
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s", _ins_name[opcode]);
				#if IS_CODEDEBUG
				is_debug(pc, dbgbuf);
				#endif
				as_print(pc, dbgbuf);
				pc += 1;
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
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s_%d %d", _ins_name[opcode], (uint32)opcode & 0x0F, (uint32)_istream_code_buffer[pc + 1]);
				#if IS_CODEDEBUG
				is_debug(pc, dbgbuf);
				#endif
				as_print(pc, dbgbuf);
				pc += 2;		
				break;
			case INS_SCTX:
			case INS_RCTX:
			case INS_OBJSTORE:
				sprintf(_RECAST(char *, dbgbuf), "\t\t%s %d", _ins_name[opcode], (uint32)_istream_code_buffer[pc + 1]);
				#if IS_CODEDEBUG
				is_debug(pc, dbgbuf);
				#endif
				as_print(pc, dbgbuf);
				pc += 2;		
				break;
			default: printf("%02x ", opcode); pc++; break;
		}
	}
#ifdef STANDALONE_COMPILER
	as_info(_RECAST(uchar *, "==========================================================="));
	printf("===========================================================\n");
	sprintf(_RECAST(char *, dbgbuf), " AREA\t\tSTART\t\tEND\t\tSIZE");
	as_info(dbgbuf);
	printf("%s\n", dbgbuf);
	as_info(_RECAST(uchar *,"-----------------------------------------------------------"));
	printf("-----------------------------------------------------------\n");
	sprintf(_RECAST(char *, dbgbuf), " CODE\t\t%04xH\t\t%04xH\t\t%d", 0, pc-1, pc);
	as_info(dbgbuf);
	printf("%s\n", dbgbuf);
	sprintf(_RECAST(char *, dbgbuf), " CONST\t\t%04xH\t\t%04xH\t\t%d", pc, pc+_cpindex -1, _cpindex);
	as_info(dbgbuf);
	printf("%s\n", dbgbuf);
	as_info(_RECAST(uchar *,"==========================================================="));
	printf("===========================================================\n");
#endif
	//constant pool
	memcpy(_istream_code_buffer + pc, _istream_data_buffer, _cpindex);
	pc += _cpindex;
	_cpindex = 0 ;		//clear constant pool
	//clean up labels
	is_clear_lblrec();
	fflush(0);
	return pc;
}

void is_link_optimize(uchar mode) {
	uint32 raw_size = _istream_code_size;
	uchar dbgbuf[280];
	if(mode & IS_OPTIMIZE_L1) {
		raw_size = _istream_code_size;
		_istream_code_size = is_l1_optimize(_istream_code_size);				//optimize code only
		sprintf(_RECAST(char *, dbgbuf), "L1 : code reduced from %d bytes to %d bytes", raw_size, _istream_code_size);
		as_info(dbgbuf);
	}
	if(mode & IS_OPTIMIZE_L2) {
		raw_size = _istream_code_size;
		_istream_code_size = is_l2_optimize(_istream_code_size);				//optimize code only
		sprintf(_RECAST(char *, dbgbuf), "L2 : code reduced from %d bytes to %d bytes", raw_size, _istream_code_size);
		as_info(dbgbuf);
	}
	if(mode != 0) {
		as_reset(_RECAST(uchar *,"OPTIMIZED CODE"));
	}
	//_current_offset = is_optimize_l1(_current_offset);				//optimize code only
	_istream_code_size = is_link_code(_istream_code_size);				//combine code+data
}

void is_file_flush() {
#ifdef STANDALONE_COMPILER
	
#ifdef IL_MAX_CODESIZE
	if(_istream_code_size > IL_MAX_CODESIZE) {
		printf("Maximum %d bytes codesize reached\n", IL_MAX_CODESIZE);
		return;
	}
#endif
	if(_ilfile != NULL) {
		fwrite(_istream_code_buffer, _istream_code_size, 1, _ilfile);
		fclose(_ilfile);
	}
	pk_file_flush(_istream_code_buffer, _istream_code_size);
#endif
}

const char * _ins_name[] = {
	"nop","1","2","3","4","5","6","7","sctx","rctx",													//0
	"0","1","2","3","4","5","objconst","objnew","objdup","objdel",											//1
	"0","1","objsz","objsub","4","objpush","objpop","7","8","objstore",									//2
	"0","1","add","sub","mul","div","mod","7","8","9",												//3
	"and","or","xor","not","4","5","6","7","8","9",													//4
	"0","1","2","3","4","5","6","7","8","9",													//5
	"0","1","label","jump","creq","crneq","crgt","crlt","crgteq","crlteq",						//6
	"jump","jfalse","jtrue","3","switch","5","6","7","8","9",													//7
	"0","1","2","3","4","5","6","7","8","9",													//8
	"0","1","2","3","4","5","f2o_0","f2o_1","f2o_2","f2o_3",													//9
	"f2o_4","f2o_5","f2o_6","f2o_7","f2o_8","f2o_9","f2o_10","f2o_11","f2o_12","f2o_13",													//10
	"f2o_14","f2o_15","2","3","4","5","6","7","8","9",													//11
	"0","1","2","3","4","5","ret","call","syscall","syscall",													//12

	"syscall","syscall","syscall","syscall","syscall","syscall","syscall","syscall","syscall","syscall",		//13
	"syscall","syscall","syscall","syscall","4","5","6","7","8","9",		//14
	"0","1","2","3","4","5","6","7","8","9",		//15
	"extcall","extcall","extcall","extcall","extcall","extcall","extcall","extcall","extcall","extcall",		//16
	"extcall","extcall","extcall","extcall","extcall","extcall","6","7","8","9",													//17
	"0","1","2","3","4","5","6","7","8","9",													//5
	"0","1","2","3","4","5","6","7","8","9",						//6
	"0","1","2","3","4","5","6","7","8","9",													//7
	"0","1","2","3","4","5","6","7","8","9",													//8
	"0","1","2","3","4","5","6","7","8","9",													//9
	"0","1","2","3","4","5","6","7","8","9",													//10
	"0","1","2","3","4","5","6","7","8","9",													//11
	"0","1","2","3","4","5","6","7","8","9",													//12
};

static lblrec * _lblhead = NULL;
static lblrec * is_lbr_get_prev_record(lblrec * rel) {
	lblrec * iterator = _lblhead;
	if(iterator == NULL) return NULL;
	while(iterator->next != rel) {
		iterator = iterator->next;
	}
	return iterator;
}

static lblrec * is_add_lblrec(lblrec * newrec) {
	lblrec * prevrec = NULL;
	prevrec = is_lbr_get_prev_record(NULL);
	if(prevrec == NULL) {
		_lblhead = newrec;
	} else {
		prevrec->next = newrec;
	}
	return newrec;
}

static lblrec * is_create_lblrec(uint16 offset, uchar * name) {
	lblrec * rec = (lblrec *)malloc(sizeof(lblrec));
	rec->next = NULL;
	rec->offset = offset;
	rec->refcount = 0;
	rec->tag = NULL;
	memset(rec->name, 0, sizeof(rec->name));
	strcpy(_RECAST(char *,rec->name), _RECAST(const char *, name));
	return rec;
}

uint16 is_push_lblrec(uint16 offset, uchar * name) {
	lblrec * rec = is_create_lblrec(offset, name);
	return is_add_lblrec(rec)->offset;
}

lblrec * is_get_lblrec(uint16 offset) {
	lblrec * iterator = _lblhead;
	lblrec * candidate = NULL;
	if(iterator == NULL) return NULL;
	while(iterator != NULL) {							//check for iterator
		if(iterator->offset == offset) { candidate = iterator; };
		iterator = iterator->next;
	}
	return candidate;
}

static lblrec * is_lbl_get_prev_record(lblrec * rel) {
	lblrec * iterator = _lblhead;
	if(iterator == NULL) return NULL;
	if(iterator == rel) return NULL;
	while(iterator->next != rel) {
		//printf("current iterator : %s, %d\n", iterator->name, iterator->next);
		iterator = iterator->next;
	}
	//printf("current iterator : %s, %d\n", iterator->name, iterator->next);
	//printf("last : %s, next : %d\n", iterator->name, iterator->next);
	return iterator;
}

void is_clear_unused_lblrec(void) {
	lblrec * iterator = _lblhead;
	lblrec * candidate = NULL;
	lblrec * prev_itr = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		if(iterator->refcount == 0) {
			candidate = iterator;
		}
		iterator = iterator->next;
		if(candidate != NULL) {
			prev_itr = is_lbl_get_prev_record(candidate);
			if(prev_itr == NULL) {
				_lblhead = candidate->next;
			} else {
				prev_itr->next = candidate->next;
			}
			memset(candidate, 0, sizeof(lblrec));		//clear previous data
			free(candidate);
			candidate = NULL;
		}
	}
}

void is_clear_lblrec(void) {
	lblrec * iterator = _lblhead;
	lblrec * candidate = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		candidate = iterator;
		iterator = iterator->next;
		memset(candidate, 0, sizeof(lblrec));		//clear previous data
		free(candidate);
		_lblhead = iterator;
	}
	_lblhead = NULL;
}

#ifdef STANDALONE_INTERPRETER
uint32 vm_fetch(uchar * codebase, uint32 offset, uchar * buffer, uint32 size) {
	//printf("offset %x, size : %x\r\n", offset, size);
	memcpy(buffer, _istream_code_buffer + offset, size);
	//printf("fetch %x%x\r\n", buffer[0], buffer[1]);
	return size;
}
#endif
