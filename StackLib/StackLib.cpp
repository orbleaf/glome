// This is the main DLL file.

#include "stdafx.h"
#using <mscorlib.dll>
#include "defs.h"
#include "config.h"

#include "StackLib.h"
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
#include "StackVM\vm_stack.h"
#pragma managed
using namespace System;
using namespace StackLib;
using namespace System::Runtime::InteropServices;

extern uchar _istream_code_buffer[];
extern uchar _istream_data_buffer[];
extern uint32 _istream_code_size;
extern uchar _pkg_buffer[];

void StackFlushLst( array<System::String^>^% lstString) {
	as_record * iterator;
	uint16 strCount = 0;
	iterator = as_get_enumerator();
	while(iterator != NULL) {
		//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		strCount ++;
		iterator = as_get_next_record(iterator);
	}
	if(strCount == 0) return;
	lstString = gcnew array<System::String^>(strCount);
	iterator = as_get_enumerator();
	strCount = 0;
	while(iterator != NULL) {
		//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		lstString[strCount++] = gcnew String(_RECAST(const char *, iterator->buffer));
		iterator = as_get_next_record(iterator);
	}
}

void StackFlushErr( array<System::String^>^% errString) {
	err_record * iterator;
	uint16 strCount = 0;
	iterator = sp_error_get_enumerator();
	while(iterator != NULL) {
		//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		strCount ++;
		iterator = sp_error_next_record(iterator);
	}
	if(strCount == 0) return;
	errString = gcnew array<System::String^>(strCount);
	iterator = sp_error_get_enumerator();
	strCount = 0;
	while(iterator != NULL) {
		//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		errString[strCount++] = gcnew String(_RECAST(const char *, iterator->buffer));
		iterator = sp_error_next_record(iterator);
	}
}


StackCompiler::StackCompiler() {
	//pk_init(NULL);
	//printf("Init constructor\n");
}

void StackCompiler::RegisterApi(System::String^ funcname, Int32 api_id, Int32 num_args) {
	
	uchar * strfunc = (uchar*)(void*)Marshal::StringToHGlobalAnsi(funcname);
	sp_install_api(strfunc, api_id, num_args);
}

void StackCompiler::CleanUp() {
	err_record * iterator, * temp;
	as_init(NULL);
	sp_error_clear();
	st_sym_clear_globals();
	is_clear_lblrec();
}

void StackCompiler::RegisterConstant(System::String^ constant, System::String^ value) {
	uchar * str_constant = (uchar*)(void*)Marshal::StringToHGlobalAnsi(constant);
	uchar * str_value = (uchar*)(void*)Marshal::StringToHGlobalAnsi(value);
	lp_update(str_constant, str_value, strlen(_RECAST(const char *, str_value)));
}

array<System::Byte>^ StackCompiler::Compile(array<System::Byte>^ input, Int32 mode, array<System::String^>^% lstString, array<System::String^>^% errString) {
	
	as_init(NULL);
	array<System::Byte>^ bytecodes = this->Compile(input, mode);
	StackFlushLst(lstString);
	StackFlushErr(errString);
	return bytecodes;
}

array<System::Byte>^ StackCompiler::Compile(array<System::Byte>^ input, Int32 mode, System::IO::StreamWriter^ lstStream) {
	as_record * iterator;
	as_init(NULL);
	array<System::Byte>^ bytecodes = this->Compile(input, mode);
	iterator = as_get_enumerator();
	while(iterator != NULL) {
		lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
		iterator = as_get_next_record(iterator);
	}
	return bytecodes;
}

array<System::Byte>^ StackCompiler::Compile(array<System::Byte>^ input, Int32 mode) {
	uint32 err;
	uint32 headersize = 0;
	pp_config * pconfig;
	array<System::Byte>^ bytecodes;
	pin_ptr<System::Byte> p = &input[0];
	_istream_code_size = 0;
	pk_init(NULL);
	pconfig = sp_clr_init(_RECAST(uchar *, p), input->Length);
	err = sp_parse();
	if(err == 0) {
		//printf("link and optimize\n");
		//fflush(0);
		is_link_optimize(mode);
		//printf("flush header\n");
		//fflush(0);
		headersize = pk_flush_root();
		lk_clear_entries();
		bytecodes = gcnew array<System::Byte>(headersize + _istream_code_size);
		pin_ptr<System::Byte> ptr = &bytecodes[0];
		memcpy(_RECAST(uchar *, ptr), _pkg_buffer, headersize);
		memcpy(_RECAST(uchar *, ptr) + headersize, _istream_code_buffer, _istream_code_size);
		_istream_code_size = 0;
	} else {
		headersize = pk_flush_root();
		lk_clear_entries();
		_istream_code_size = 0;
	}
		
	free(pconfig);
	return bytecodes; 
}


array<System::Byte>^ StackCompiler::Compile(System::String^ path, Int32 mode) {
	uint32 err;
	uint32 headersize;
	pp_config * pconfig;
	//pin_ptr<System::String> p = &path[0];
	
	char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(path);
	_istream_code_size = 0;
	pk_init(NULL);
	pconfig= sp_init(_RECAST(uchar *, str2));
	err = sp_parse();
	if(err == 0) {
		is_link_optimize(mode);
	}
	headersize = pk_flush_root();
	lk_clear_entries();
	array<System::Byte>^ bytecodes = gcnew array<System::Byte>(headersize + _istream_code_size);
	pin_ptr<System::Byte> ptr = &bytecodes[0];
	memcpy(_RECAST(uchar *, ptr), _pkg_buffer, headersize);
	memcpy(_RECAST(uchar *, ptr) + headersize, _istream_code_buffer, _istream_code_size);
	_istream_code_size = 0;
	return bytecodes; 
}

void StackCompiler::LoadPackage(array<System::Byte>^ input) {
	pin_ptr<System::Byte> p = &input[0];
	//printf("Load Package\n");
	lk_decode_buffer(input->Length, p);
}

System::String^ StackCompiler::About() {
	return gcnew String("Orb-Weaver 1.4 Compiler, Copyright 2016 @ Orbleaf Technology");
}

extern void * _base_address;
uint32 vm_fetch(uchar * codebase, uint32 offset, uchar * buffer, uint32 size) {
	//array<System::Byte>^ bytecodes = gcnew array<System::Byte>(headersize + _istream_code_size);
	
	memcpy(buffer, _RECAST(uchar *, (uchar *)_base_address + offset), size);
	return size;
}

StackVirtualMachine::StackVirtualMachine() {

}

void StackVirtualMachine::Initialize(array<System::Byte>^ codebase, int offset)
{
	g_baCodeBase = codebase;
	pin_ptr<System::Byte> ptr = &g_baCodeBase[0];
	_base_address = _RECAST(void *, ptr);
	vm_init(0);
}

