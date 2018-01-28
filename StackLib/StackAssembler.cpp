
#include "stdafx.h"
#using <mscorlib.dll>
#include "defs.h"
#include "config.h"


#pragma unmanaged
#include "Stack\lex_proto.h"
#include "Stack\sem_proto.h"
#include "Stack\sym_table.h"
#include "Stack\token.h"
#include "Stack\pkg_encoder.h"
#include "Stack\pkg_linker.h"
#include "Stack\il_streamer.h"
#include "Stack\il1_optimizer.h"
#include "Stack\il2_optimizer.h"
#include "Stack\asm_streamer.h"
#include "Stack\scr_generator.h"

#pragma managed
using namespace System;
using namespace System::IO;

