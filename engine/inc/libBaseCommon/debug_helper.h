#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "base_function.h"
#include "base_exception.h"
#include "exception_handler.h"
#include "logger.h"


#define SAFE_DELETE(ptr) { if(ptr != nullptr) { delete ptr; ptr = nullptr; } }
#define SAFE_DELETE_ARRAY(ptr) { if(ptr != nullptr) { delete[] ptr; ptr = nullptr; } }
#define SAFE_RELEASE(ptr) { if(ptr != nullptr) { ptr->release(); ptr = nullptr; } }

// WindowsÏÂ__debugbreak()´úÌæint 3
#if defined _DEBUG && defined _WIN32
#	define BREAK_POINT do{ __debugbreak(); } while(0)
#else
#	define BREAK_POINT
#endif

#ifdef _DEBUG
#	define DebugAbort(Msg)			do{ base::saveErrorLog("DebugAbort(%s) in %s, %d", #Msg, __FILE__, __LINE__); BREAK_POINT;  } while(0)
#else
#	define DebugAbort(Msg)			do{ base::saveErrorLog("DebugAbort(%s) in %s, %d", #Msg, __FILE__, __LINE__); exit( 0 );  } while(0)
#endif

#define	DebugThrow( Exp )	throw base::CBaseException(Exp)

#define	DebugAst(Exp)			do{ if(!(Exp)) { base::saveErrorLog("DebugAst(%s) in %s, %d", #Exp, __FILE__, __LINE__); BREAK_POINT; return; } } while(0)
#define	DebugAstEx(Exp, Ret)	do{ if(!(Exp)) { base::saveErrorLog("DebugAstEx(%s) in %s, %d", #Exp, __FILE__, __LINE__); BREAK_POINT; return Ret; } } while(0)

#define	IF_NOT(Exp) if(!(Exp) ? (base::saveErrorLog("IF_NOT(%s) in %s, %d", #Exp, __FILE__, __LINE__), 1) : 0)
#define	IF_OK(Exp)  if((Exp) ? 1 : (base::saveErrorLog("IF_OK(%s) in %s, %d", #Exp, __FILE__, __LINE__), 0))