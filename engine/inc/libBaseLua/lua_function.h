#pragma once

#include "lua_facade.h"
#include "libBaseCommon/debug_helper.h"

namespace base
{
	class __BASE_LUA_API__ CLuaFunction
	{
	public:
		CLuaFunction(CLuaFacade* pLuaFacade, const char* szName);
		CLuaFunction(CLuaFacade* pLuaFacade, int32_t nRef);
		CLuaFunction();
		~CLuaFunction();

		CLuaFunction(const CLuaFunction&lhs);
		CLuaFunction(CLuaFunction&& lhs);

		CLuaFunction& operator = (const CLuaFunction& lhs);
		CLuaFunction& operator = (CLuaFunction&& lhs);

		template<class ...Args>
		inline bool			call(Args... args);

		template<class RT, class ...Args>
		inline bool			callR(RT& ret, Args... args);

		template<class ...Args>
		static inline bool	call(CLuaFacade* pLuaFacade, const char* szFunName, Args... args);

		template<class RT, class ...Args>
		static inline bool	callR(CLuaFacade* pLuaFacade, const char* szFunName, RT& ret, Args... args);

	private:
		CLuaFacade*	m_pLuaFacade;
		int32_t		m_nRef;
	};
}