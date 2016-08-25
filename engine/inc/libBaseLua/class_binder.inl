
namespace base
{
	template<class T>
	CClassBinder<T>::CClassBinder(CNamespaceBinder& sNamespaceBinder)
		: m_sNamespaceBinder(sNamespaceBinder)
		, m_nStackTop(-1)
	{
		lua_State* pL = this->m_sNamespaceBinder.getLuaFacade()->getState();

		this->m_nStackTop = lua_gettop(pL);
		--this->m_nStackTop;
		if (this->m_nStackTop < 0)
			throw std::logic_error("invalid class stack");
	}

	template<class T>
	CClassBinder<T>::~CClassBinder()
	{
	}

	template<class T>
	CNamespaceBinder& CClassBinder<T>::endClass()
	{
		lua_State* pL = this->m_sNamespaceBinder.getLuaFacade()->getState();

		lua_settop(pL, this->m_nStackTop);

		return this->m_sNamespaceBinder;
	}

	template<class T>
	int32_t CClassBinder<T>::deleteObject(lua_State* pL)
	{
		SObjectWraper* pObjectWraper = reinterpret_cast<SObjectWraper*>(lua_touserdata(pL, 1));
		if (!pObjectWraper->bGc)
		{
			pObjectWraper->pObject = nullptr;
			return 0;
		}
		lua_pushstring(pL, _FACADE_NAME);
		lua_rawget(pL, LUA_REGISTRYINDEX);
		CLuaFacade* pLuaFacade = reinterpret_cast<CLuaFacade*>(lua_touserdata(pL, -1));
		lua_pop(pL, 1);
		DebugAstEx(pLuaFacade != nullptr, 0);

		pLuaFacade->setActiveLuaState(pL);

		delete reinterpret_cast<T*>(pObjectWraper->pObject);
		pObjectWraper->pObject = nullptr;

		pLuaFacade->setActiveLuaState(nullptr);
		return 0;
	}

	template<class T>
	template<class ...Args>
	int32_t CClassBinder<T>::createObject(lua_State* pL)
	{
		SFunctionBaseWrapper<void, Args...> sFunctionBaseWrapper;
		sFunctionBaseWrapper.invoke = [&](Args... args)
		{
			T* pObject = new T(args...);
			lua_pushstring(pL, _WEAK_OBJECT_TBL_NAME);
			lua_rawget(pL, LUA_REGISTRYINDEX);
			int32_t nWeak = lua_gettop(pL);
			// 查看对象是否存在
			lua_pushlightuserdata(pL, pObject);
			lua_rawget(pL, -2);
			if (lua_isnil(pL, -1))
			{
				lua_pop(pL, 1);

				// 创建对象的包裹内存块
				SObjectWraper* pObjectWraper = reinterpret_cast<SObjectWraper*>(lua_newuserdata(pL, sizeof(SObjectWraper)));
				pObjectWraper->pObject = pObject;
				pObjectWraper->bGc = true;
				pObjectWraper->nRefCount = 0;
				// 设置对象的metatable
				getMetatable(pL, SClassName<T>::getNamespaceName(), SClassName<T>::getName());
				lua_setmetatable(pL, -2);

				lua_pushlightuserdata(pL, pObject);
				lua_pushvalue(pL, -2);
				// 将对象设置进弱表中 weak_object_table[pObject] = pObjectWraper
				lua_rawset(pL, nWeak);
				lua_replace(pL, nWeak);
				lua_settop(pL, nWeak);
			}
			else
			{
				// 可能对象已经被C++层销毁，但lua的gc尚未执行，所以一个导出到lua的对象在析构时需要主动的调用下面的unBindObject，去释放弱表中的引用
				lua_settop(pL, nWeak - 1);
				lua_pushnil(pL);
				DebugAst(!"object is exist");
			}
		};

		lua_pushstring(pL, _FACADE_NAME);
		lua_rawget(pL, LUA_REGISTRYINDEX);
		CLuaFacade* pLuaFacade = reinterpret_cast<CLuaFacade*>(lua_touserdata(pL, -1));
		lua_pop(pL, 1);
		DebugAstEx(pLuaFacade != nullptr, 0);

		pLuaFacade->setActiveLuaState(pL);

		int32_t nIndex = 1;
		SParseArgs<sizeof...(Args), void, Args...>::template parse<Args...>(pL, nIndex, sFunctionBaseWrapper);

		pLuaFacade->setActiveLuaState(nullptr);

		return 1;
	}

	template<class T>
	template<class RT, class ...Args>
	CClassBinder<T>& CClassBinder<T>::registerFunction(const char* szName, RT(T::*fn)(Args...))
	{
		if (fn == nullptr || szName == nullptr)
			throw std::logic_error("invalid register class function arg");

		lua_State* pL = this->m_sNamespaceBinder.getLuaFacade()->getState();

		int32_t nTop = lua_gettop(pL);

		lua_pushstring(pL, _CLASS_FUNCTION);
		lua_rawget(pL, nTop);

		lua_pushstring(pL, szName);
		lua_rawget(pL, -2);
		if (!lua_isnil(pL, -1))
			throw std::logic_error("dup register class function");

		lua_pop(pL, 1);

		SClassFunctionWrapper<T, RT, Args...>* pClassFunctionWrapper = new SClassFunctionWrapper<T, RT, Args...>();
		pClassFunctionWrapper->pf = fn;

		lua_pushstring(pL, szName);
		lua_pushlightuserdata(pL, pClassFunctionWrapper);
		lua_pushcclosure(pL, &__class_invoke_proxy<T, RT, Args...>, 1);
		lua_rawset(pL, -3);

		lua_pop(pL, 1);

		return *this;
	}

	template<class T>
	template<class RT, class ...Args>
	CClassBinder<T>& CClassBinder<T>::registerFunction(const char* szName, RT(T::*fn)(Args...) const)
	{
		if (fn == nullptr || szName == nullptr)
			throw std::logic_error("invalid register class function arg");

		lua_State* pL = this->m_sNamespaceBinder.getLuaFacade()->getState();

		int32_t nTop = lua_gettop(pL);

		lua_pushstring(pL, _CLASS_FUNCTION);
		lua_rawget(pL, nTop);

		lua_pushstring(pL, szName);
		lua_rawget(pL, -2);
		if (!lua_isnil(pL, -1))
			throw std::logic_error("dup register class function");

		lua_pop(pL, 1);

		SClassFunctionWrapper<T, RT, Args...>* pClassFunctionWrapper = new SClassFunctionWrapper<T, RT, Args...>();
		pClassFunctionWrapper->pf = reinterpret_cast<typename SClassFunctionWrapper<T, RT, Args...>::FUN_TYPE>(fn);

		lua_pushstring(pL, szName);
		lua_pushlightuserdata(pL, pClassFunctionWrapper);
		lua_pushcclosure(pL, &__class_invoke_proxy<T, RT, Args...>, 1);
		lua_rawset(pL, -3);

		lua_pop(pL, 1);

		return *this;
	}

	template<class T>
	template<class M>
	CClassBinder<T>& CClassBinder<T>::registerMember(const char* szName, const M T::* pMember)
	{
		if (szName == nullptr || pMember == nullptr)
			throw std::logic_error("invalid register class member arg");

		lua_State* pL = this->m_sNamespaceBinder.getLuaFacade()->getState();

		int32_t nTop = lua_gettop(pL);

		lua_pushstring(pL, _CLASS_PROPERTY);
		lua_rawget(pL, nTop);

		lua_pushstring(pL, szName);
		lua_rawget(pL, -2);
		if (!lua_isnil(pL, -1))
			throw std::logic_error("dup register class member");

		lua_pop(pL, 1);

		SMemberOffsetInfo<T, M>* pMemberOffsetInfo = new SMemberOffsetInfo<T, M>();
		pMemberOffsetInfo->mp = pMember;
		lua_pushstring(pL, szName);
		lua_pushlightuserdata(pL, pMemberOffsetInfo);
		lua_pushcclosure(pL, &__property_proxy<T, M>, 1);
		lua_rawset(pL, -3);

		lua_pop(pL, 1);

		return *this;
	}

	template<class T>
	template<class M>
	CClassBinder<T>& CClassBinder<T>::registerStaticMember(const char* szName, M* pMember)
	{
		if (szName == nullptr || pMember == nullptr)
			throw std::logic_error("invalid register static class member arg");

		lua_State* pL = this->m_sNamespaceBinder.getLuaFacade()->getState();

		int32_t nTop = lua_gettop(pL);

		lua_pushstring(pL, _CLASS_PROPERTY);
		lua_rawget(pL, nTop);

		lua_pushstring(pL, szName);
		lua_rawget(pL, -2);
		if (!lua_isnil(pL, -1))
			throw std::logic_error("dup register class member");

		lua_pop(pL, 1);

		lua_pushstring(pL, szName);
		lua_pushlightuserdata(pL, pMember);
		lua_pushcclosure(pL, &__static_property_proxy<M>, 1);
		lua_rawset(pL, -3);

		lua_pop(pL, 1);

		return *this;
	}
}