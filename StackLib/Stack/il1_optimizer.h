#include "../defs.h"
#include "il_streamer.h"
#ifndef _PH_OPTIMIZER__H

//optimization rec
optrec * is_l1_push_jump(uint16 offset, uint16 opt_offset, uchar ins, uint16 index);
uint32 is_l1_backpatch(uint16 offset, uint16 opt_offset);
void is_l1_clear_optrec(void);

//code rec
uint16 is_l1_push_coderec(uint16 offset, uint16 rdc);
uint16 is_l1_translate(uint16 offset);
uint32 is_l1_jumptable_reloc(uint32 current_pc, uint32 jt_offset);
void is_l1_clear_coderec(void);

//main function
uint32 is_l1_optimize(uint32 size);

#define _PH_OPTIMIZER__H
#endif

