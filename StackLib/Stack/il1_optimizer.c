#include "il1_optimizer.h"

static coderec * _cdrhead = NULL;
static optrec * _opthead = NULL;
static optrec * is_l1_opt_get_prev_record(optrec * rel) {
	optrec * iterator = _opthead;
	if(iterator == NULL) return NULL;
	while(iterator->next != rel) {
		iterator = iterator->next;
	}
	return iterator;
}

static optrec * is_l1_add_optrec(optrec * newrec) {
	optrec * prevrec = NULL;
	prevrec = is_l1_opt_get_prev_record(NULL);
	if(prevrec == NULL) {
		_opthead = newrec;
	} else {
		prevrec->next = newrec;
	}
	return newrec;
}

static optrec * is_l1_create_optrec(uint16 offset, uint16 opt_offset, uchar ins, uint16 index) {
	optrec * rec = (optrec *)malloc(sizeof(optrec));
	rec->next = NULL;
	rec->ins = ins;
	rec->index = index;
	rec->offset = offset;
	rec->opt_offset = opt_offset;
	return rec;
}

optrec * is_l1_push_jump(uint16 offset, uint16 opt_offset, uchar ins, uint16 index) {
	optrec * rec = is_l1_create_optrec(offset, opt_offset, ins, index);
	is_l1_add_optrec(rec);
	return rec;
}

uint32 is_l1_backpatch(uint16 offset, uint16 opt_offset) {
	optrec * iterator = _opthead;
	if(iterator == NULL) return 0;
	while(iterator != NULL) {
		if(iterator->index == offset) {			//jump index matched
			_istream_code_buffer[iterator->opt_offset] = iterator->ins;
			*((uint16 * )(_istream_code_buffer + iterator->opt_offset + 1)) = end_swap16(opt_offset);
			//printf("patched instruction at %08x, to %08x\n", iterator->opt_offset, opt_offset);
		}
		iterator = iterator->next;
	}
	return iterator->offset;
}

static coderec * is_l1_cdr_get_prev_record(coderec * rel) {
	coderec * iterator = _cdrhead;
	if(iterator == NULL) return NULL;
	while(iterator->next != rel) {
		iterator = iterator->next;
	}
	return iterator;
}

static coderec * is_l1_add_coderec(coderec * newrec) {
	coderec * prevrec = NULL;
	prevrec = is_l1_cdr_get_prev_record(NULL);
	if(prevrec == NULL) {
		_cdrhead = newrec;
	} else {
		prevrec->next = newrec;
	}
	return newrec;
}

static coderec * is_l1_create_coderec(uint16 offset, uint16 rdc) {
	coderec * rec = (coderec *)malloc(sizeof(coderec));
	rec->next = NULL;
	rec->offset = offset;
	rec->rdc = rdc;
	return rec;
}

uint16 is_l1_push_coderec(uint16 offset, uint16 rdc) {
	coderec * rec = is_l1_create_coderec(offset, rdc);
	is_l1_add_coderec(rec);
	//printf("create code reduction %08x\n", offset);
	return rdc;
}

uint16 is_l1_translate(uint16 offset) {
	coderec * iterator = _cdrhead;
	coderec * nextitr = NULL;
	uint16 l_offset = offset;
	uint32 rdc = 0;
	if(iterator != NULL)
	while(iterator != NULL) {
		nextitr = iterator->next;
		rdc += iterator->rdc;
		if(nextitr != NULL) {
			if(iterator->offset <= offset && nextitr->offset > offset) {
				l_offset = (iterator->offset - rdc);
				l_offset += (offset - iterator->offset);
				break;
			}
		} else {
			if(iterator->offset <= offset) {
				l_offset = (iterator->offset - rdc);
				l_offset += (offset - iterator->offset);
			}
		}
		iterator = iterator->next;
	}
	//printf("translate %08x -> %08x\n", offset, l_offset);
	return l_offset;
}

uint32 is_l1_jumptable_reloc(uint32 current_pc, uint32 jt_offset) {
	uint32 i = 2, index;
	uint32 cp_size = _istream_data_buffer[jt_offset];
	jt_offset += 1;				//cp size
	//default
	index = end_swap16(*((uint16 * )(_istream_data_buffer + jt_offset)));
	*((uint16 * )(_istream_data_buffer + jt_offset)) = end_swap16(is_l1_translate(index));
	//switched labels
	while(i < cp_size) {
		index = end_swap16(*((uint16 * )(_istream_data_buffer + jt_offset + i + 2)));
		*((uint16 * )(_istream_data_buffer + jt_offset + i + 2)) = end_swap16(is_l1_translate(index));
		i += 4;
	}
	return i;
}

void is_l1_clear_coderec(void) {
	coderec * iterator = _cdrhead;
	coderec * candidate = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		candidate = iterator;
		iterator = iterator->next;
		memset(candidate, 0, sizeof(coderec));		//clear previous data
		free(candidate);
		_cdrhead = iterator;
	}
}

void is_l1_clear_optrec(void) {
	optrec * iterator = _opthead;
	optrec * candidate = NULL;
	if(iterator == NULL) return;
	while(iterator != NULL) {							//check for iterator
		candidate = iterator;
		iterator = iterator->next;
		memset(candidate, 0, sizeof(optrec));		//clear previous data
		free(candidate);
		_opthead = iterator;
	}
}

uint32 is_l1_optimize(uint32 size) {
	uint32 pc = 0;
	uchar opcode = 0;
	uchar operand;
	uint16 index;
	uint32 rdc = 0;
	lblrec * curlbl = NULL;
	//optimizing control flow (no code reduction)
	while(pc != size) {
		opcode = _istream_code_buffer[pc];
		operand = _istream_code_buffer[pc+1];
		
		switch(opcode) {
			case INS_LBL:					//create new label
				pc += (operand + 2);
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
			case INS_RET:
			case INS_CREQ:			//64	//-> compare equal 
			case INS_CRNE:			//65	//-> compare not equal
			case INS_CRGT:			//66	//-> compare greater than 
			case INS_CRLT:			//67	//-> compare less than
			case INS_CRGTEQ:			//68	//-> compare greater than
			case INS_CRLTEQ:			//69	//-> compare less than 
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
			case INS_RCTX:
			case INS_OBJPUSH:		
			case INS_OBJPOP:
			case INS_OBJSTORE:
				pc += 2;		
				break;
			case INS_SWITCH:				//constant pool jumptable must be optimized
			case INS_CALL:
			case INS_F2O:
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
				pc += 3;
				break;
			case INS_SCTX:
				curlbl = is_get_lblrec(pc);
				if(curlbl != NULL) {
					curlbl->refcount++;
				} 
				pc += 2;
				break;
			case INS_JTRUE:
			case INS_JFALSE:
			case INS_JMP:					//jump to specified label
				index = pc;
				loop_next_jump:
				index = end_swap16(*((uint16 * )(_istream_code_buffer + index + 1)));
				if(_istream_code_buffer[index] == INS_JMP) goto loop_next_jump;
				if((pc + 3) != index) {
					curlbl = is_get_lblrec(index);
					if(curlbl != NULL) {
						curlbl->refcount++;
					} 
				}
				*((uint16 * )(_istream_code_buffer + pc + 1)) = end_swap16(index);
				pc += 3;
				break;
			default: break;
		}
	}
	is_clear_unused_lblrec();
	pc = 0;							//start peephole optimizer for cond/uncond jump operation
	while(pc != (size - rdc)) {
		opcode = _istream_code_buffer[pc];
		operand = _istream_code_buffer[pc+1];
		switch(opcode) {
			case INS_LBL:					//create new label (remove label)
				rdc += is_l1_push_coderec(pc + rdc, operand + 2);
				memcpy(_istream_code_buffer + pc, _istream_code_buffer + (pc + (operand + 2)), (size - rdc) - pc);
				//printf("%d\n", _istream_code_buffer[pc]);
				//pc += (operand + 2);
				break;
			case INS_OBJPUSH:
			case INS_OBJPOP:
				pc += 2;
				break;
			/*case INS_OBJPUSH:	
				if((_istream_code_buffer[pc + 2] == INS_OBJPUSH) && (_istream_code_buffer[pc + 3] == operand) && (is_get_lblrec(pc + 2) == NULL)) {
					//printf("objpush optimized %08x\n", pc + rdc);
					_istream_code_buffer[pc + 2] = INS_OBJDUP;						//change objpop to store
					rdc += is_l1_push_coderec(pc + rdc, 1);
					memcpy(_istream_code_buffer + pc + 3, _istream_code_buffer + pc + 4, (size - rdc) - pc);
					pc += 3;
				} else {
					pc += 2;
				}
				break;		
			case INS_OBJPOP:
				curlbl = is_get_lblrec(pc + 2);
				//if(curlbl == NULL) printf("optimized at %08x\n", pc+rdc);
				if((_istream_code_buffer[pc + 2] == INS_OBJPUSH) && (_istream_code_buffer[pc + 3] == operand) && (is_get_lblrec(pc + 2) == NULL)) {
					//printf("objpop optimized %08x\n", pc + rdc);
					_istream_code_buffer[pc] = INS_OBJSTORE;						//change objpop to store
					rdc += is_l1_push_coderec(pc + rdc, 2);
					memcpy(_istream_code_buffer + pc + 2, _istream_code_buffer + pc + 4, (size - rdc) - pc);
					pc += 2;
				} else {
					pc += 2;
				}
				break;*/
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
			case INS_RET:
			case INS_CREQ:			//64	//-> compare equal 
			case INS_CRNE:			//65	//-> compare not equal
			case INS_CRGT:			//66	//-> compare greater than 
			case INS_CRLT:			//67	//-> compare less than
			case INS_CRGTEQ:			//68	//-> compare greater than
			case INS_CRLTEQ:			//69	//-> compare less than 
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
			case INS_SCTX:
			case INS_RCTX:
			case INS_OBJSTORE:
				pc += 2;		
				break;
			case INS_SWITCH:				//constant pool jumptable must be optimized
				pc += 3;
				break;
			case INS_F2O:
			case INS_CALL:
			case INS_JTRUE:
			case INS_JFALSE:
			case INS_JMP:					//jump to specified label
				index = pc;
				index = end_swap16(*((uint16 * )(_istream_code_buffer + index + 1)));
				if(index == ((pc + 3) + rdc)) {
					//printf("jump code optimize %08x\n", pc+rdc);
					memcpy(_istream_code_buffer + pc, _istream_code_buffer + pc + 3, (size - rdc) - pc);
					rdc += is_l1_push_coderec(pc + rdc, 3);
					pc += 0;
				} else if(index > pc) {
					//push to backpatch list
					is_l1_push_jump(pc + rdc, pc, opcode, index);
					pc += 3;
				} else if( index < pc) {
					//use table to relocate the reduced code
					*((uint16 * )(_istream_code_buffer + pc + 1)) = end_swap16(is_l1_translate(index));
					pc += 3;
				}
				break;
			default: break;
		}

		//check for backpatched (pc+rdc) = original_pc, pc = optimized_pc
		is_l1_backpatch(pc+rdc, pc);
		//update label offset with new optimized offset
		curlbl = is_get_lblrec(pc+rdc);
		if(curlbl != NULL) {
			curlbl->offset = pc;
		}
	}		
	is_l1_clear_coderec();		//clear code reductions
	is_l1_clear_optrec();		//clear optimization records

	size = size - rdc;			//size = optimized size
	rdc = 0;
	pc = 0;						//restarting peephole optimizer for load/store
	while(pc != (size - rdc)) {
		opcode = _istream_code_buffer[pc];
		operand = _istream_code_buffer[pc+1];
		switch(opcode) {
			case INS_LBL:					//create new label (remove label)
				rdc += is_l1_push_coderec(pc + rdc, operand + 2);
				memcpy(_istream_code_buffer + pc, _istream_code_buffer + (pc + (operand + 2)), (size - rdc) - pc);
				//printf("%d\n", _istream_code_buffer[pc]);
				//pc += (operand + 2);
				break;
			case INS_OBJPUSH:	
				if((_istream_code_buffer[pc + 2] == INS_OBJPUSH) && (_istream_code_buffer[pc + 3] == operand) && (is_get_lblrec(pc + 2) == NULL)) {
					//printf("objpush optimized %08x\n", pc + rdc);
					_istream_code_buffer[pc + 2] = INS_OBJDUP;						//change objpop to store
					rdc += is_l1_push_coderec(pc + rdc, 1);
					memcpy(_istream_code_buffer + pc + 3, _istream_code_buffer + pc + 4, (size - rdc) - pc);
					pc += 3;
				} else {
					pc += 2;
				}
				break;		
			case INS_OBJPOP:
				curlbl = is_get_lblrec(pc + 2);
				//if(curlbl == NULL) printf("optimized at %08x\n", pc+rdc);
				if((_istream_code_buffer[pc + 2] == INS_OBJPUSH) && (_istream_code_buffer[pc + 3] == operand) && (is_get_lblrec(pc + 2) == NULL)) {
					//printf("objpop optimized %08x\n", pc + rdc);
					_istream_code_buffer[pc] = INS_OBJSTORE;						//change objpop to store
					rdc += is_l1_push_coderec(pc + rdc, 2);
					memcpy(_istream_code_buffer + pc + 2, _istream_code_buffer + pc + 4, (size - rdc) - pc);
					pc += 2;
				} else {
					pc += 2;
				}
				break;
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
			case INS_RET:
			case INS_CREQ:			//64	//-> compare equal 
			case INS_CRNE:			//65	//-> compare not equal
			case INS_CRGT:			//66	//-> compare greater than 
			case INS_CRLT:			//67	//-> compare less than
			case INS_CRGTEQ:			//68	//-> compare greater than
			case INS_CRLTEQ:			//69	//-> compare less than 
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
			case INS_SCTX:
			case INS_RCTX:
			case INS_OBJSTORE:
				pc += 2;		
				break;
			case INS_SWITCH:				//constant pool jumptable must be optimized
				pc += 3;
				break;
			case INS_F2O:
			case INS_CALL:
			case INS_JTRUE:
			case INS_JFALSE:
			case INS_JMP:					//jump to specified label
				index = pc;
				index = end_swap16(*((uint16 * )(_istream_code_buffer + index + 1)));
				if(index == ((pc + 3) + rdc)) {
					//printf("jump code optimize %08x\n", pc+rdc);
					memcpy(_istream_code_buffer + pc, _istream_code_buffer + pc + 3, (size - rdc) - pc);
					rdc += is_l1_push_coderec(pc + rdc, 3);
					pc += 0;
				} else if(index > pc) {
					//push to backpatch list
					is_l1_push_jump(pc + rdc, pc, opcode, index);
					pc += 3;
				} else if( index < pc) {
					//use table to relocate the reduced code
					*((uint16 * )(_istream_code_buffer + pc + 1)) = end_swap16(is_l1_translate(index));
					pc += 3;
				}
				break;
			default: break;
		}

		//check for backpatched (pc+rdc) = original_pc, pc = optimized_pc
		is_l1_backpatch(pc+rdc, pc);
		//update label offset with new optimized offset
		curlbl = is_get_lblrec(pc+rdc);
		if(curlbl != NULL) {
			curlbl->offset = pc;
		}
	}
	//optimized switch constant pool
	pc = 0;
	while(pc != (size - rdc)) {
		opcode = _istream_code_buffer[pc];
		operand = _istream_code_buffer[pc+1];
		switch(opcode) {
			case INS_LBL:					//create new label
				pc += (operand + 2);
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
			case INS_RET:
			case INS_CREQ:			//64	//-> compare equal 
			case INS_CRNE:			//65	//-> compare not equal
			case INS_CRGT:			//66	//-> compare greater than 
			case INS_CRLT:			//67	//-> compare less than
			case INS_CRGTEQ:			//68	//-> compare greater than
			case INS_CRLTEQ:			//69	//-> compare less than 
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
			case INS_SCTX:
			case INS_RCTX:
			case INS_OBJPUSH:		
			case INS_OBJPOP:
			case INS_OBJSTORE:
				pc += 2;		
				break;
			case INS_F2O:
			case INS_CALL:
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
				pc += 3;
				break;
			case INS_JTRUE:
			case INS_JFALSE:
			case INS_JMP:					//jump to specified label
				pc += 3;
				break;
			case INS_SWITCH:				//constant pool jumptable must be optimized
				index = end_swap16(*((uint16 * )(_istream_code_buffer + pc + 1)));
				is_l1_jumptable_reloc(pc, index);
				pc += 3;
				break;
			default: break;
		}
	}
	//constant pool
	is_l1_clear_coderec();
	is_l1_clear_optrec();
	return pc;
}
