#pragma once

#include "lua_facade.h"
#include "namespace_binder.h"

namespace base
{
	template<class T>
	class CClassBinder
	{
		friend class CNamespaceBinder;
	
	private:
		CClassBinder(CNamespaceBinder& sNamespaceBinder);

	public:
		~CClassBinder();

		CNamespaceBinder&		endClass();

		template<class RT, class ...Args>
		inline CClassBinder<T>&	registerFunction(const char* szName, RT(T::*fn)(Args...) const);
		template<class RT, class ...Args>
		inline CClassBinder<T>&	registerFunction(const char* szName, RT(T::*fn)(Args...));

		template<class M>
		inline CClassBinder<T>&	registerMember(const char* szName, const M T::* pMember);
		template<class M>
		inline CClassBinder<T>&	registerStaticMember(const char* szName, M* pMember);

		template<class ...Args>
		inline static int32_t	createObject(lua_State* pL);
		inline static int32_t	deleteObject(lua_State* pL);

	private:
		template<class F>
		inline void				registerClassBase(const char* szSuperName, F fn);

	private:
		CNamespaceBinder&	m_sNamespaceBinder;
		int32_t				m_nStackTop;
	};

}