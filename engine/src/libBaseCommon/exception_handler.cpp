#include "stdafx.h"
#include "exception_handler.h"
#include "function_util.h"
#include "time_util.h"
#include "process_util.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <dbghelp.h>
#else
#include <setjmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <execinfo.h>
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include <signal.h>
#include <iostream>
#include <memory>
#include <mutex>
#include "file_util.h"

#ifdef _WIN32
#pragma comment( lib, "dbghelp.lib" )
#endif

namespace
{
#define _MAX_STACK_SIZE 64

#define _MAX_EXCEPTION_BUF_SIZE 16*1024
#define _MAX_EXTRA_MEMORY		10*1024*1024

#ifdef _WIN32
	void* g_pExtraMemory = nullptr;	// 这块内存是程序启动时保留的，保证在程序崩溃时有足够的内存来调用dump相关的操作
	char* g_pExceptionBuf = nullptr;	// 主要是用于崩溃是可能会用到内存时用

	void createErrorLog(const char* szFileName, struct _EXCEPTION_POINTERS *pExceptionInfo)
	{
		// Errorlog 生成时间
		base::time_util::STime curTime = base::time_util::getLocalTimeTM();
		int32_t nLen = _snprintf(g_pExceptionBuf, _MAX_EXCEPTION_BUF_SIZE, "Exception Time: %d-%d-%d (%d:%d:%d)\n\n", curTime.nYear, curTime.nMon, curTime.nDay, curTime.nHour, curTime.nMin, curTime.nSec);
		// 错误码及系统环境(内存 虚拟内存 读写IO CPU)
		int32_t nCpu = 0;
		uint64_t nSMem = 0, nVMen = 0, nReadBytes = 0, nWriteBytes = 0;

		base::process_util::getMemoryUsage(nSMem, nVMen);
		base::process_util::getIOBytes(nReadBytes, nWriteBytes);
		nCpu = base::process_util::getCPUUsage();

		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "Error Code: %d\r\n", (uint32_t)GetLastError());
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "smem:%I64d vmem:%I64d read:%I64d write:%I64d cpu:%d\r\n", nSMem / 1024, nVMen / 1024, nReadBytes / 1024, nWriteBytes / 1024, nCpu);

		// 线程信息
		DWORD nThreadId = ::GetCurrentThreadId();
		DWORD nProcessId = ::GetCurrentProcessId();

		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "Process ID=%d, Thread ID=%d\r\n", nProcessId, nThreadId);

		// 异常错误码
		PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "Exception code: %08x\r\n", pExceptionRecord->ExceptionCode);

		// 寄存器信息
		PCONTEXT pContext = pExceptionInfo->ContextRecord;
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "\r\nRegisters:\r\n");
#ifdef _WIN64
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "RAX:%I64x\tRBX:%I64x\r\n", pContext->Rax, pContext->Rbx);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "RCX:%I64x\tRDX:%I64x\r\n", pContext->Rcx, pContext->Rdx);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "ESI:%I64x\tEDI:%I64x\r\n", pContext->Rsi, pContext->Rdi);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "R8:%I64x\tR9:%I64x\r\n", pContext->R8, pContext->R9);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "R10:%I64x\tR11:%I64x\r\n", pContext->R10, pContext->R11);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "R12:%I64x\tR13:%I64x\r\n", pContext->R12, pContext->R13);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "R14:%I64x\tR15:%I64x\r\n", pContext->R14, pContext->R15);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "RIP:%I64x\tRSP:%I64x\tRBP:%I64x\r\n", pContext->Rip, pContext->Rip, pContext->Rbp);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "Flags:%08x\r\n", pContext->EFlags);
#else
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "EAX:%08x\tEBX:%08x\r\n", pContext->Eax, pContext->Ebx);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "ECX:%08x\tEDX:%08x\r\n", pContext->Ecx, pContext->Edx);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "ESI:%08x\tEDI:%08x\r\n", pContext->Esi, pContext->Edi);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "EIP:%08X\tESP:%08x\tEBP:%08x\r\n", pContext->Eip, pContext->Esp, pContext->Ebp);
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "Flags:%08x\r\n", pContext->EFlags);
#endif	
		nLen += _snprintf(g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen, "\n________________________________________\n\n");

		nLen += (int32_t)base::getStackInfo(1, 20, g_pExceptionBuf + nLen, _MAX_EXCEPTION_BUF_SIZE - nLen);

		// 输出日志文件
		FILE* pFile = fopen(szFileName, "w");
		if (nullptr != pFile)
		{
			nLen = (int32_t)strlen(g_pExceptionBuf);
			fwrite(g_pExceptionBuf, sizeof(char), nLen, pFile);
			fclose(pFile);
		}
	}

	class CGenWinDump
	{
	public:
		CGenWinDump()
		{
			::SymInitialize(::GetCurrentProcess(), nullptr, FALSE);
		}

		~CGenWinDump()
		{
			::SymCleanup(::GetCurrentProcess());
		}

		bool		loadSymbol();
		size_t		getStackInfo(void** pStack, uint32_t nDepth, char* szBuf, size_t nBufSize);
		size_t		getStackInfo(uint32_t nBegin, uint32_t nEnd, char* szBuf, size_t nBufSize);
		uint32_t	getStack(uint32_t nBegin, uint32_t nEnd, void** pStack, uint32_t nStackSize);

	private:
		std::mutex m_mutex;
	};

	CGenWinDump* g_pGenWinDump = nullptr;

	uint32_t CGenWinDump::getStack(uint32_t nBegin, uint32_t nEnd, void** pStack, uint32_t nStackSize)
	{
		STACKFRAME64 sf;
		memset(&sf, 0, sizeof(STACKFRAME64));

		CONTEXT context;
		::RtlCaptureContext(&context);
#ifdef _M_IX86
		sf.AddrPC.Offset = context.Eip;
		sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrStack.Offset = context.Esp;
		sf.AddrStack.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = context.Ebp;
		sf.AddrFrame.Mode = AddrModeFlat;
		uint32_t nMachineType = IMAGE_FILE_MACHINE_I386;
#elif defined _M_X64	// x64 (AMD64 or EM64T)
		sf.AddrPC.Offset = context.Rip;
		sf.AddrPC.Mode = AddrModeFlat;
		sf.AddrStack.Offset = context.Rsp;
		sf.AddrStack.Mode = AddrModeFlat;
		sf.AddrFrame.Offset = context.Rbp;
		sf.AddrFrame.Mode = AddrModeFlat;
		uint32_t nMachineType = IMAGE_FILE_MACHINE_AMD64;
#else
#error "platform not supported!"
#endif

		HANDLE hProcess = ::GetCurrentProcess();
		HANDLE hThread = ::GetCurrentThread();

		++nEnd;
		++nBegin;
		uint32_t i = 0;
		for (; i < nEnd; ++i)
		{
			if (!::StackWalk64(nMachineType, hProcess, hThread, &sf, &context, 0, ::SymFunctionTableAccess64, ::SymGetModuleBase64, 0))
				break;

			if (0 == sf.AddrFrame.Offset)
				break;

			if (i >= nBegin && (i - nBegin) < nStackSize)
				pStack[i - nBegin] = reinterpret_cast<void*>(sf.AddrPC.Offset);
		}

		return __min(i - nBegin, nStackSize);
	}

	size_t CGenWinDump::getStackInfo(void** pStack, uint32_t nDepth, char* szBuf, size_t nBufSize)
	{
		if (szBuf == nullptr)
			return 0;

		size_t nLen = 0;
		for (uint32_t i = 0; i < nDepth && nLen < nBufSize; ++i)
		{
			char szSymbolBuf[sizeof(PIMAGEHLP_SYMBOL64) + 1024] = { 0 };
			PIMAGEHLP_SYMBOL64 pSymbol = (PIMAGEHLP_SYMBOL64)szSymbolBuf;
			pSymbol->SizeOfStruct = sizeof(szSymbolBuf);
			pSymbol->MaxNameLength = 1024;
			DWORD64 pAddr = reinterpret_cast<DWORD64>(pStack[i]);
			HMODULE hModule = (HMODULE)::SymGetModuleBase64(::GetCurrentProcess(), pAddr);
			char szFileName[MAX_PATH] = { '?' };
			if (hModule != nullptr)
			{
				char szAllFileName[MAX_PATH] = { 0 };
				DWORD nRet = ::GetModuleFileNameA(hModule, szAllFileName, MAX_PATH);
				if (nRet != 0)
				{
					szAllFileName[nRet] = 0;
					strncpy(szFileName, szAllFileName, MAX_PATH);
				}
			}

			nLen += _snprintf(szBuf + nLen, nBufSize - nLen, "\t%s ", szFileName);
			char szSymbolName[MAX_PATH] = { '?' };
			DWORD64 nAddress = pAddr;
			if (!::SymGetSymFromAddr64(GetCurrentProcess(), pAddr, 0, pSymbol))
			{
				if (this->loadSymbol())
				{
					if (::SymGetSymFromAddr64(GetCurrentProcess(), pAddr, 0, pSymbol))
					{
						nAddress = pSymbol->Address;
						strncpy(szSymbolName, pSymbol->Name, MAX_PATH);
					}
				}
			}
			else
			{
				nAddress = pSymbol->Address;
				strncpy(szSymbolName, pSymbol->Name, MAX_PATH);
			}
			nLen += _snprintf(szBuf + nLen, nBufSize - nLen, "%s[0x%I64x]()\r\n", szSymbolName, nAddress);
		}

		if (nLen > 2)
		{
			szBuf[nLen - 1] = 0;
			szBuf[nLen - 2] = 0;
			nLen -= 2;
		}
		return nLen;
	}

	size_t CGenWinDump::getStackInfo(uint32_t nBegin, uint32_t nEnd, char* szBuf, size_t nBufSize)
	{
		void* zStack[256] = { nullptr };
		this->m_mutex.lock();
		uint32_t nDepth = this->getStack(nBegin, nEnd, zStack, _countof(zStack));
		size_t nLen = this->getStackInfo(zStack, nDepth, szBuf, nBufSize);
		this->m_mutex.unlock();

		return nLen;
	}

	void genDump(const char* szName, struct _EXCEPTION_POINTERS* pExp)
	{
		if (szName == nullptr || pExp == nullptr)
			return;

		MINIDUMP_EXCEPTION_INFORMATION miniDumpExpInfo;

		HANDLE hDumpFile = ::CreateFileA(
			szName,
			GENERIC_WRITE,
			0,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);

		if (hDumpFile == INVALID_HANDLE_VALUE)
			return;

		miniDumpExpInfo.ThreadId = ::GetCurrentThreadId();
		miniDumpExpInfo.ExceptionPointers = pExp;
		miniDumpExpInfo.ClientPointers = TRUE;

		::MiniDumpWriteDump(
			::GetCurrentProcess(),
			::GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&miniDumpExpInfo,
			nullptr,
			nullptr);

		::CloseHandle(hDumpFile);
	}

	bool CGenWinDump::loadSymbol()
	{
		auto funLoadModuleProc = [](PCSTR szModuleName, DWORD64 nModuleBase, ULONG nModuleSize, PVOID pUserContext)->BOOL
		{
			uint64_t nRet = ::SymLoadModule64(::GetCurrentProcess(), nullptr, szModuleName, szModuleName, nModuleBase, nModuleSize);
			if (0 == nRet)
				return base::getLastError() == ERROR_SUCCESS;
			else
				return TRUE;
		};

		return TRUE == ::EnumerateLoadedModules64(::GetCurrentProcess(), (PENUMLOADED_MODULES_CALLBACK64)funLoadModuleProc, this);
	}

	// 到这里基本死了
	LONG WINAPI exceptionHandler(struct _EXCEPTION_POINTERS* pExceptionInfo)
	{
		if (nullptr == pExceptionInfo)
			return EXCEPTION_CONTINUE_SEARCH;

		// 释放程序启动时预留的内存，保证dump能够产生
		delete[] g_pExtraMemory;

		char szPath[MAX_PATH] = { 0 };
		base::function_util::snprintf(szPath, MAX_PATH, "%s/dump", base::process_util::getCurrentWorkPath());

		// 不关心返回值
		base::file_util::createRecursionDir(szPath);

		base::time_util::STime curTime = base::time_util::getLocalTimeTM();
		char szFile[MAX_PATH] = { 0 };
		base::function_util::snprintf(szFile, MAX_PATH, "%s/core_%s%04d_%02d_%02d_%02d_%02d_%02d.dmp", szPath, base::process_util::getInstanceName(), curTime.nYear, curTime.nMon, curTime.nDay, curTime.nHour, curTime.nMin, curTime.nSec);
		genDump(szFile, pExceptionInfo);

		base::function_util::snprintf(szFile, MAX_PATH, "%s/%s_%04d_%02d_%02d_%02d_%02d_%02d.log", szPath, base::process_util::getInstanceName(), curTime.nYear, curTime.nMon, curTime.nDay, curTime.nHour, curTime.nMin, curTime.nSec);
		createErrorLog(szFile, pExceptionInfo);
		return EXCEPTION_EXECUTE_HANDLER;
	}

	BOOL WINAPI breakCtrlC(DWORD msgType)
	{
		return (msgType == CTRL_C_EVENT) || (msgType == CTRL_CLOSE_EVENT);
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
	{
		return nullptr;
	}

	// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效
	void disableSetUnhandledExceptionFilter(void)
	{
		HMODULE hKernel32 = LoadLibrary(L"kernel32.dll");
		if (hKernel32 == nullptr)
			return;

		void* pOrgEntry = ::GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
		if (pOrgEntry == nullptr)
			return;

		unsigned char newJump[5];
		DWORD_PTR dwOrgEntryAddr = (DWORD_PTR)pOrgEntry;
		dwOrgEntryAddr += 5; //jump instruction has 5 byte space.

		void* pNewFunc = &MyDummySetUnhandledExceptionFilter;
		DWORD_PTR dwNewEntryAddr = (DWORD_PTR)pNewFunc;
		DWORD_PTR dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

		newJump[0] = 0xE9;  //jump
		memcpy(&newJump[1], &dwRelativeAddr, sizeof(DWORD));
		SIZE_T bytesWritten;
		DWORD dwOldFlag, dwTempFlag;
		::VirtualProtect(pOrgEntry, 5, PAGE_READWRITE, &dwOldFlag);
		BOOL bRet = ::WriteProcessMemory(::GetCurrentProcess(), pOrgEntry, newJump, 5, &bytesWritten);
		::VirtualProtect(pOrgEntry, 5, dwOldFlag, &dwTempFlag);
	}

	void invalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
	{

	}


#else
	/*
	Signal     Value     Action   Comment
	----------------------------------------------------------------------
	SIGHUP        1       Term    Hangup detected on controlling terminal or death of controlling process
	SIGINT        2       Term    Interrupt from keyboard
	SIGQUIT       3       Core    Quit from keyboard
	SIGILL        4       Core    Illegal Instruction
	SIGABRT       6       Core    Abort signal from abort(3)
	SIGFPE        8       Core    Floating point exception
	SIGKILL       9       Term    Kill signal
	SIGSEGV      11       Core    Invalid memory reference
	SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
	SIGALRM      14       Term    Timer signal from alarm(2)
	SIGTERM      15       Term    Termination signal
	SIGUSR1   30,10,16    Term    User-defined signal 1
	SIGUSR2   31,12,17    Term    User-defined signal 2
	SIGCHLD   20,17,18    Ign     Child stopped or terminated
	SIGCONT   19,18,25    Cont    Continue if stopped
	SIGSTOP   17,19,23    Stop    Stop process
	SIGTSTP   18,20,24    Stop    Stop typed at terminal
	SIGTTIN   21,21,26    Stop    Terminal input for background process
	SIGTTOU   22,22,27    Stop    Terminal output for background process

	The signals SIGKILL and SIGSTOP cannot be caught, blocked, or ignored.
	*/
	// void signalHandler( int32_t signo, siginfo_t* , void* ptr )
	// {
	// 	struct sigaction sig_action;
	// 	sig_action.sa_sigaction = SIG_DFL;
	// 	sig_action.sa_flags = SA_SIGINFO;
	// 	sigaction( signo, &sig_action, nullptr );
	// 
	// 	char szPath[MAX_PATH] = { 0 };
	// 	snprintf( szPath, MAX_PATH, "%s/dump", base::getCurrentWorkPath() );
	// 
	// 	base::createDir( szPath );
	// 	chdir( szPath );
	// 
	// 	raise( signo );
	// }
#endif
}

namespace base
{
	void initProcessExceptionHander()
	{
#ifdef _WIN32
		g_pExceptionBuf = new char[_MAX_EXCEPTION_BUF_SIZE];
		g_pExtraMemory = new char[_MAX_EXTRA_MEMORY];
#endif

#ifdef _WIN32
		// 关闭在Debug下ASSEST失败弹对话框特性
#ifdef _DEBUG
		_CrtSetReportMode(_CRT_ASSERT, 0);
		_CrtSetReportMode(_CRT_ERROR, 0);
#endif
		g_pGenWinDump = new CGenWinDump();
		if (!g_pGenWinDump->loadSymbol())
			fprintf(stderr, "load pdb fail\n");

		SetErrorMode(SEM_NOGPFAULTERRORBOX);
		SetUnhandledExceptionFilter(exceptionHandler);
		disableSetUnhandledExceptionFilter();

		// CtrlC屏蔽
		//SetConsoleCtrlHandler(breakCtrlC, true);

#ifdef _DEBUG
		// abort不弹出对话框
		_set_abort_behavior(0, _WRITE_ABORT_MSG);
		_set_abort_behavior(-1, _CALL_REPORTFAULT);
#endif

		// 这个在linux下没有对应的功能，一般如果是非法指针直接段错误，最终会产生一个dump
		_set_invalid_parameter_handler(invalidParameterHandler);
#else
#endif
	}

	void uninitProcessExceptionHander()
	{
#ifdef _WIN32
		delete[] g_pExtraMemory;
		delete[] g_pExceptionBuf;

		delete g_pGenWinDump;
#endif
	}

	void initThreadExceptionHander()
	{
#ifdef _WIN32
		// 设置在栈溢出时后在异常处理器中的使用的栈大小，这个大小是在正常的栈大小中切下来的
		ULONG nStack = 64 * 4096;
		BOOL bSuccess = SetThreadStackGuarantee(&nStack);
#endif
	}

	size_t getStack(uint32_t nBegin, uint32_t nEnd, void** pStack, size_t nMaxSize)
	{
		if (pStack == nullptr)
			return 0;

#ifdef _WIN32
		return g_pGenWinDump->getStack(nBegin, nEnd, pStack, (uint32_t)nMaxSize);
#else
		if (nEnd > _MAX_STACK_SIZE)
			return 0;

		// 使用这一系列函数必须在编译的时候加上-rdynamic选项
		void* pAddress[_MAX_STACK_SIZE] = { nullptr };
		size_t nSize = backtrace(pAddress, nEnd);
		size_t i = nBegin;
		for (; i < nSize; ++i)
		{
			pStack[i - nBegin] = pAddress[i];
			if (i - nBegin >= nMaxSize)
				break;
		}
		
		return i - nBegin;
#endif
	}

	size_t getStackInfo(uint32_t nBegin, uint32_t nEnd, char* szInfo, size_t nMaxSize)
	{
		if (szInfo == nullptr)
			return 0;

#ifdef _WIN32
		nBegin += 2;
		size_t nLen = g_pGenWinDump->getStackInfo(nBegin, nEnd, szInfo, nMaxSize);
#else
		if (nEnd > _MAX_STACK_SIZE)
			return 0;

		// 使用这一系列函数必须在编译的时候加上-rdynamic选项
		char szSymbol[4096] = { 0 };
		void* pAddress[_MAX_STACK_SIZE] = { nullptr };
		char** pSymbol = nullptr;
		size_t nSize = backtrace(pAddress, nEnd);
		pSymbol = (char**)backtrace_symbols(pAddress, nSize);
		size_t nSymbolSize = 0;
		for (size_t i = nBegin; i < nSize && pSymbol != nullptr; ++i)
		{
			uint32_t nSymbolLen = strlen(pSymbol[i]);
			if (nSymbolSize + nSymbolLen >= _countof(szSymbol))
				break;
			memcpy(szSymbol + nSymbolSize, "\t", 1);
			++nSymbolSize;
			memcpy(szSymbol + nSymbolSize, pSymbol[i], nSymbolLen);
			nSymbolSize += nSymbolLen;
			memcpy(szSymbol + nSymbolSize, "\r\n", 2);
			nSymbolSize += 2;
		}
		if (pSymbol != nullptr)
			free(pSymbol);

		memset(szInfo, 0, nMaxSize);
		uint32_t nLen = nSymbolSize < (nMaxSize - 1) ? nSymbolSize : (nMaxSize - 1);
		memcpy(szInfo, szSymbol, nLen);
#endif

		return nLen;
	}

	size_t getFunctionInfo(const void* pAddr, char* szInfo, size_t nMaxSize)
	{
		if (pAddr == nullptr || szInfo == nullptr)
			return 0;

#ifdef _WIN32
		return g_pGenWinDump->getStackInfo((void**)&pAddr, 1, szInfo, nMaxSize);
#else
		// 使用这一系列函数必须在编译的时候加上-rdynamic选项
		char** pSymbol = nullptr;
		pSymbol = (char**)backtrace_symbols((void*const*)&pAddr, 1);
		if (pSymbol != nullptr && pSymbol[0] != nullptr)
			base::function_util::strncpy(szInfo, nMaxSize, pSymbol[0], _TRUNCATE);
		if (pSymbol != nullptr)
			free(pSymbol);

		return strnlen(szInfo, nMaxSize);
#endif
	}

}

#ifdef _WIN32
#pragma warning(pop)
#endif