#include "../defs.h"
#include "../config.h"
#include "../stack/sem_proto.h"
#include "../stack/lex_proto.h"

#include "vm_stack.h"
#include "vm_framework.h"
#ifdef CGI_APPLICATION
#include "../libcgi/src/cgi.h"
#include "../libcgi/src/error.h"
#endif
#include "../stack/asm_streamer.h"
#include <stdio.h>


void vi_register_system_apis() {
	//BYTE i;
	vm_api_entry * iterator = (vm_api_entry *)&g_vaRegisteredApis;
	while(iterator->id != 0) {
		printf("install api %s : %d\r\n", iterator->name, iterator->id);
		sp_install_api(iterator->name, iterator->id, 0);
		iterator++;
	} 
}

uint8 sample_code[] = "\
#define null \"\"\r\n\
#define true \"true\"\r\n\
#define false \"false\"\r\n\
\r\n\
//comment;\r\n\
extern function this() syscall 0;\r\n\
class program {\r\n\
	function foreach(x, func) {\r\n\
		print(x.count());\r\n\
			for(var i=0;i<x.count();i++) {\r\n\
				print(x.pen);\r\n\
				print(x[i]);\r\n\
			}\r\n\
	}\r\n\
    public function main() {\r\n\
	var w=\"\";\r\n\
	var t = new c1();\r\n\
	t->x1(function(tx) { print(tx); });	\r\n\
	var jstr = [ \"pen\":1, \"apple\", \"human\" ];\r\n\
        var my_array = jstr.from_json();\r\n\
		for(var i=0;i<7;i++) print(i);\r\n\
		//var my_array = [\"apple\", \"orange\"];\r\n\
        //print(my_array.to_hex());\r\n\
        //print(my_array.to_json().from_json().to_hex());\r\n\
        print(\"number of objects : \" + my_array.count());\r\n\
        print(\"array 1: \" + my_array[1]);\r\n\
        my_array.add(\"pineapple\");\r\n\
        print(my_array.to_json());\r\n\
		//foreach(my_array, function(t) {\r\n\
		//	print(t);\r\n\
		//});\r\n\
		print(my_array.remove(1).to_json());\r\n\
    }\r\n\
}\r\n\r\n\
class c1 {\r\n\
	public function x1(x) {\r\n\
		x(\"from x1\");\r\n\
	}\r\n\
}\r\n\
\r\n\
\r\n";

int main() {
	uint32 err;
	char * source = NULL;
	uint32 headersize = 0;
	pp_config * pconfig;
	as_record * iterator;
	as_init(NULL);
	pk_init(NULL);
	vi_register_system_apis();
#ifdef CGI_APPLICATION
	cgi_init();
	cgi_process_form();	
	cgi_init_headers();
	//_istream_code_size = 0;
	if(cgi_param("script")) {
		source = cgi_param("script);
	} else {
		printf("Orb-Weaver %d.%d CGI Interpreter <br>", IS_MAJOR_VERSION, IS_MINOR_VERSION);
		printf("Copyright 2018 @ Orbleaf Technology<br>");
		cgi_end();
		return 0;
	}
	pconfig = sp_clr_init(_RECAST(uchar *, source), strlen(source));
#else
	printf(sample_code);
	pconfig = sp_clr_init(_RECAST(uchar *, sample_code), strlen(sample_code));
#endif
	err = sp_parse();
	if(err == 0) {
		//printf("link and optimize\n");
		fflush(0);
		is_link_optimize(0);
		//printf("flush header\n");
		fflush(0);
		headersize = pk_flush_root();
		//lk_clear_entries();
		//printf("header size : %d\r\n", headersize);
		//bytecodes = gcnew array<System::Byte>(headersize + _istream_code_size);
		//pin_ptr<System::Byte> ptr = &bytecodes[0];
		//memcpy(_RECAST(uchar *, ptr), _pkg_buffer, headersize);
		//memcpy(_RECAST(uchar *, ptr) + headersize, _istream_code_buffer, _istream_code_size);
		//_istream_code_size = 0;
		lk_set_root(pk_get_root());
		lk_dump_classes();
		//iterator = as_get_enumerator();
		//while(iterator != NULL) {
			//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		//	printf("%s\n", iterator->buffer);
		//	iterator = as_get_next_record(iterator);
		//}

		vm_init(0);
		vm_decode(0, 0);
		vm_close();
	} else {
		
		headersize = pk_flush_root();
		//_istream_code_size = 0;
	}
	lk_clear_entries();
		
	free(pconfig);
	//return bytecodes; 
#ifdef CGI_APPLICATION
	cgi_end();
#endif
	return 0;
}
