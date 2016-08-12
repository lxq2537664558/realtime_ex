#pragma once
#include "lua_facade.h"
#include "namespace_binder.h"

namespace base
{
	template<typename T>
	class CClassBinder
	{
		friend class CNamespaceBinder;
	
	private:
		CClassBinder(CNamespaceBinder& sNamespaceBinder);

	public:
		~CClassBinder();

		CNamespaceBinder&		endClass();

		template<typename RT, typename ...Args>
		inline CClassBinder<T>&	registerFunction(const char* szName, RT(T::*fn)(Args...) const);
		template<typename RT, typename ...Args>
		inline CClassBinder<T>&	registerFunction(const char* szName, RT(T::*fn)(Args...));

		template<typename M>
		inline CClassBinder<T>&	registerMember(const char* szName, const M T::* pMember);
		template<typename M>
		inline CClassBinder<T>&	registerStaticMember(const char* szName, M* pMember);

		template<typename ...Args>
		inline static int32_t	createObject(lua_State* pL);
		inline static int32_t	deleteObject(lua_State* pL);

	private:
		template<typename F>
		inline void				registerClassBase(const char* szSuperName, F fn);

	private:
		CNamespaceBinder&	m_sNamespaceBinder;
		int32_t				m_nStackTop;
	};

}

#include "class_binder.inl"