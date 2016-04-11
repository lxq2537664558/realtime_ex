#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "base_common.h"

namespace base
{
	/**
	@brief: »ù´¡Òì³£Àà
	*/
	class __BASE_COMMON_API__ CBaseException
	{
	public:
#ifdef _WIN32
		CBaseException(EXCEPTION_POINTERS* pExp);
#endif
		CBaseException(const char* szInfo);
		CBaseException(const CBaseException& rhs);
		const CBaseException& operator = (const CBaseException& rhs);

		~CBaseException();

		const char*			getInfo() const;
		static const char*	getName(uint32_t nExceptionID);

	private:
#ifdef _WIN32
		EXCEPTION_POINTERS* m_pExp;
#endif
		char*				m_szInfo;
	};

}