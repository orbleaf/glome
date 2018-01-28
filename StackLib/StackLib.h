// StackLib.h

#pragma once

using namespace System::Runtime::InteropServices;
using namespace System;


using namespace System;

namespace StackLib {

	public ref class StackCompiler
	{
	public :
		// TODO: Add your methods for this class here.
		array<System::Byte>^ Compile(array<System::Byte>^ input, Int32 mode, array<System::String^>^ %lstString, array<System::String^>^ %errString);
		array<System::Byte>^ Compile(array<System::Byte>^ input, System::Int32 mode);
		array<System::Byte>^ Compile(System::String^ path, Int32 mode);
		array<System::Byte>^ Compile(array<System::Byte>^ input, Int32 mode, System::IO::StreamWriter^ lstStream);
		System::String^		 About();
		StackCompiler::StackCompiler();		//constructor
		void StackCompiler::LoadPackage(array<System::Byte>^ input);
		void RegisterApi(System::String^ funcname, Int32 api_id, Int32 num_args);
		void RegisterConstant(System::String^ constant, System::String^ value);
		void CleanUp();
	
	};

	public enum StackMachineState {
			INIT = 0x00,
			RUN = 0x01,
			SUSPEND = 0x03,
			EXCEPTION = 0x04,
			ABORT = 0x07
	};

	public ref class StackEventArgs : System::EventArgs
	{
	private:
		StackMachineState g_stMachineState;
		int g_iCurrentPC;
	public:
		StackEventArgs() { }
		StackEventArgs(int pc) { g_iCurrentPC = pc; }

		property StackMachineState MachineState
		{
			StackMachineState get() { return g_stMachineState; }
			void set(StackMachineState x) { g_stMachineState = x; }
		};
		property int CurrentPC
		{
			int get() { return g_iCurrentPC; }
			void set(int x) { g_iCurrentPC = x; }
		};
	};

	public ref class StackExceptionEventArgs : StackEventArgs
	{
	private:
		String ^ g_strMessage;
	public:
		StackExceptionEventArgs(int pc) { CurrentPC = pc; }
		property String^ Message {
			String^ get()  { return g_strMessage; }
			void set(String^ x) { g_strMessage = x; }
		};
	};

	public ref class StackFetchEventArgs : StackEventArgs 
	{
	public:
		StackFetchEventArgs(int pc) { CurrentPC = pc; }
	};

	public ref class StackApiEventArgs : StackEventArgs 
	{
	private:
		array<Byte>^ g_baReturnedValue;
		array<array<Byte>^>^ g_bbArgs;
	public:
		StackApiEventArgs(int pc) { CurrentPC = pc; }
		property array<Byte>^ ReturnedValue
		{
			array<Byte>^ get() { return g_baReturnedValue; }
			void set(array<Byte>^ x) { g_baReturnedValue = x; }
		};
		property array<array<Byte>^>^ Arguments 
		{
			array<array<Byte>^>^ get() { return g_bbArgs; }
			void set (array<array<Byte>^>^ x) { g_bbArgs = x; }
		};
	};

	public ref class StackMethodEventArgs : StackEventArgs
	{
	private:
		String^ g_strClassName;
		String^ g_strMethodName;
		array<System::Byte>^ g_baMethodBase;
		int g_iMethodOffset;
	public:
		property String^ ClassName
		{
			String^ get() { return g_strClassName; }
			void set(String^ x) { g_strClassName = x; }
		};
		property String^ MethodName
		{
			String^ get() { return g_strMethodName; }
			void set(String^ x) { g_strMethodName = x; }
		};
		property array<System::Byte>^ MethodBase {
			array<System::Byte>^ get() { return g_baMethodBase; }
			void set(array<System::Byte>^ x) { g_baMethodBase = x; }
		};
		property int MethodOffset
		{
			int get() { return g_iMethodOffset; }
			void set(int x) { g_iMethodOffset = x; }
		};
	};

	public delegate void StackApiCallback(System::Object^ sender, StackApiEventArgs^ e);
	public delegate void StackFetchCallback(System::Object^ sender, StackFetchEventArgs^ e);
	public delegate void StackExceptionCallback(System::Object^ sender, StackExceptionEventArgs^ e);
	public delegate void StackLoadMethodCallback(System::Object^ sender, StackMethodEventArgs^ e);

	public ref class StackVirtualMachine //abstract
	{
	private :
		array<System::Byte>^ g_baCodeBase;
	public:
		event StackApiCallback^ ApiCallback;
		event StackLoadMethodCallback^ LoadMethodCallback;
		event StackFetchCallback^ FetchCallback;
		event StackExceptionCallback^ ExceptionCallback;
		
		StackVirtualMachine();
		void Initialize(array<System::Byte>^ codebase, int offset);
		int Decode(int offset) ;
		int Decode();
		void InvokeException(int code);
		//virtual int Fetch(array<System::Byte>^ codebase, int offset, array<System::Byte>^ buffer, int size);
		//uint32 vm_fetch(uchar * codebase, uint32 offset, uchar * buffer, uint32 size);
	};
}
