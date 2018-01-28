// StackConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>

#include "defs.h"
#include "config.h"

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

#if 1

int testcompile(int argc, _TCHAR * argv[])
{
	//extern FILE *yyin;
	static int i = 1;
	err_record * iterator;
	uint16 strCount = 0;
	uchar gen_asm = 0, gen_il = 0, gen_scr = 0;
	//++argv;			//skip exepath
	uchar opt_level = 0;
	//sp_init_parser();
	if(argc == 1) goto print_usage;
	while(i != (argc - 1)) {
		if(strcmp(_RECAST(const char *, argv[i]), _RECAST(const char *, "-ca")) == 0) {
			gen_il = 1;
		} else if(strcmp(_RECAST(const char *, argv[i]), _RECAST(const char *, "-c")) == 0) {
			gen_asm = 1;
		} else if(strcmp(_RECAST(const char *, argv[i]), _RECAST(const char *, "-cd")) == 0) {
			gen_scr = 1;
		} else if(strcmp(_RECAST(const char *, argv[i]), _RECAST(const char *, "-o1")) == 0) {
			opt_level |= IS_OPTIMIZE_L1;
		} else if(strcmp(_RECAST(const char *, argv[i]), _RECAST(const char *, "-o2")) == 0) {
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
			//is_link_optimize(opt_level);
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
		} 
		{
			//printf("parse failed\n");
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


//int _tmain(int argc, _TCHAR * argv[]) {
//	_TCHAR * arguments[] = { _RECAST(_TCHAR *, "stack"), _RECAST(_TCHAR *, "-ca"), _RECAST(_TCHAR *, "-c"), _RECAST(_TCHAR *, "-cd"), _RECAST(_TCHAR *, "C:\\Users\\Agus\\Documents\\Visual Studio 2005\\Projects\\StackLib\\debug\\sample.stk") };
//	testcompile(5, arguments);
//	return 0;
//}
#endif

