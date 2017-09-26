#pragma once

#include "lua_facade.h"

#include "libBaseCommon/noncopyable.h"

namespace base
{
	template<class T>
	class CClassBinder;

	class __BASE_LUA_API__ CNamespaceBinder :
		public noncopyable
	{
	public:
		CNamespaceBinder(CLuaFacade* pLuaFacade);
		~CNamespaceBinder();

		void					begin(const char* szName);
		void					end();

		template<class T, class F>
		CClassBinder<T>			registerClass(const char* szClassName, F fn);

		template<class T, class F>
		CClassBinder<T>			registerClass(const char* szClassName, const char* szSuperClassName, F fn);
		
		template<class RT, class ...Args>
		inline void				registerFunction(const char* szName, RT(*fn)(Args...));
		
		template<class T>
		inline void				registerConstData(const char* szName, const T& val);

		CLuaFacade*				getLuaFacade() const;

	private:
		template<class T, class F>
		CClassBinder<T>			registerClassBase(const char* szClassName, const char* szSuperClassName, F fn);

	private:
		CLuaFacade*	m_pLuaFacade;
		char*		m_szNamespaceName;
		int32_t		m_nStackTop;
	};
}