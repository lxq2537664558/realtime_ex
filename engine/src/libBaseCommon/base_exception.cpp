#include "stdafx.h"
#include "base_exception.h"
#include "base_function.h"
#include "debug_helper.h"

namespace base
{
#ifdef _WIN32
	CBaseException::CBaseException(EXCEPTION_POINTERS* pExp)
		: m_pExp(pExp)
		, m_szInfo(nullptr)
	{
	}
#endif

	CBaseException::CBaseException(const char* szInfo)
	{
#ifdef _WIN32
		this->m_pExp = nullptr;
#endif
		if (szInfo != nullptr)
		{
			size_t nLen = base::crt::strnlen(szInfo, -1);
			this->m_szInfo = new char[nLen + 1];
			base::crt::strncpy(this->m_szInfo, nLen + 1, szInfo, _TRUNCATE);
		}
	}

	CBaseException::CBaseException(const CBaseException& rhs)
	{
#ifdef _WIN32
		this->m_pExp = nullptr;
#endif
		if (rhs.m_szInfo != nullptr)
		{
			size_t nLen = base::crt::strnlen(rhs.m_szInfo, -1);
			this->m_szInfo = new char[nLen + 1];
			base::crt::strncpy(this->m_szInfo, nLen + 1, rhs.m_szInfo, _TRUNCATE);
		}
	}

	const CBaseException& CBaseException::operator = (const CBaseException& rhs)
	{
		if (this == &rhs)
			return *this;

#ifdef _WIN32
		this->m_pExp = nullptr;
#endif
		SAFE_DELETE_ARRAY(this->m_szInfo);

		if (rhs.m_szInfo != nullptr)
		{
			size_t nLen = base::crt::strnlen(rhs.m_szInfo, -1);
			this->m_szInfo = new char[nLen + 1];
			base::crt::strncpy(this->m_szInfo, nLen + 1, rhs.m_szInfo, _TRUNCATE);
		}

		return *this;
	}

	CBaseException::~CBaseException()
	{
		SAFE_DELETE_ARRAY(this->m_szInfo);
	}

	const char* CBaseException::getInfo() const
	{
		return this->m_szInfo;
	}

	const char* CBaseException::getName(uint32_t nExceptionCode)
	{
#ifdef _WIN32

		static char szBuf[] = { '0', 'x', '0', '0', '0', '0', '0', '0', '0', '0', '0' };

		switch (nExceptionCode)
		{
		case STATUS_NO_MEMORY:	return "No Memory";
		case EXCEPTION_ACCESS_VIOLATION:	return "Access Violation";
		case EXCEPTION_DATATYPE_MISALIGNMENT:	return "Datatype Misalignment";
		case EXCEPTION_BREAKPOINT:	return "Breakpoint";
		case EXCEPTION_SINGLE_STEP:	return "Single Step";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:	return "Array Bounds Exceeded";
		case EXCEPTION_FLT_DENORMAL_OPERAND:	return "Float Denormal Operand";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:	return "Float Divide by Zero";
		case EXCEPTION_FLT_INEXACT_RESULT:	return "Float Inexact Result";
		case EXCEPTION_FLT_INVALID_OPERATION:	return "Float Invalid Operation";
		case EXCEPTION_FLT_OVERFLOW:	return "Float Overflow";
		case EXCEPTION_FLT_STACK_CHECK:	return "Float Stack Check";
		case EXCEPTION_FLT_UNDERFLOW:	return "Float Underflow";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:	return "Integer Divide by Zero";
		case EXCEPTION_INT_OVERFLOW:	return "Integer Overflow";
		case EXCEPTION_PRIV_INSTRUCTION:	return "Privileged Instruction";
		case EXCEPTION_IN_PAGE_ERROR:	return "In Page Error";
		case EXCEPTION_ILLEGAL_INSTRUCTION:	return "Illegal Instruction";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:	return "Noncontinuable Exception";
		case EXCEPTION_STACK_OVERFLOW:	return "Stack Overflow";
		case EXCEPTION_INVALID_DISPOSITION:	return "Invalid Disposition";
		case EXCEPTION_GUARD_PAGE:	return "Guard Page";
		case EXCEPTION_INVALID_HANDLE:	return "Invalid Handle";
		case 0xE06D7363:	return "Microsoft C++ Exception";
		default:
			{
				base::crt::itoa(nExceptionCode, &szBuf[2], _countof(szBuf) - 2, 16);
				return szBuf;
			}
		};

#else
		return "";
#endif
	}

}