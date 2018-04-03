#include "../defs.h"
#include "il_streamer.h"
#ifndef _SC_ELIMINATOR__H

//optimization rec
optrec * is_l2_push_jump(uint16 offset, uint16 opt_offset, uchar ins, uint16 index);
uint32 is_l2_backpatch(uint16 offset, uint16 opt_offset);
void is_l2_clear_optrec(void);

//code rec
uint16 is_l2_push_coderec(uint16 offset, uint16 rdc);
uint16 is_l2_translate(uint16 offset);
uint32 is_l2_jumptable_reloc(uint32 current_pc, uint32 jt_offset);
void is_l2_clear_coderec(void);

void is_l2_remove_var(uint16 start_offset, uchar var_id, uint32 size);
void is_l2_replace_var(uint16 start_offset, uchar var_id, uchar new_id, uint32 size);
uint32 is_l2_count_var_load(uint16 start_offset, uchar var_id, uint32 size);

uint32 is_l2_optimize(uint32 size);
#define _SC_ELIMINATOR__H
#endif
