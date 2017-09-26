namespace base
{
	template<class T, class F>
	CClassBinder<T> CNamespaceBinder::registerClassBase(const char* szClassName, const char* szSuperClassName, F fn)
	{
		if (szClassName == nullptr || fn == nullptr)
			throw std::logic_error("invalid register class arg");

		lua_State* pL = this->m_pLuaFacade->getState();

		int32_t nTop = lua_gettop(pL);
		lua_pushstring(pL, szClassName);
		lua_gettable(pL, nTop);
		if (!lua_isnil(pL, -1))
			throw std::logic_error("dup register class");

		lua_pop(pL, 1);

		SClassName<T>::setNamespaceName(this->m_szNamespaceName);
		SClassName<T>::setName(szClassName);

		// 创建这个class的metatable
		lua_newtable(pL);
		lua_pushstring(pL, szClassName);
		lua_pushvalue(pL, -2);
		lua_rawset(pL, nTop);

		lua_pushvalue(pL, -1);
		lua_setmetatable(pL, -1);

		int32_t nMT = lua_gettop(pL);

		// 将基类注册的函数跟变量拷贝到子类中。
		if (szSuperClassName != nullptr)
		{
			CTokenParser token;
			if (!token.parse(szSuperClassName, "."))
				throw std::logic_error("invalid super name");

			if (token.getElementCount() < 1)
				throw std::logic_error("invalid super name");

			lua_pushinteger(pL, LUA_RIDX_GLOBALS);
			lua_rawget(pL, LUA_REGISTRYINDEX);

			char szBuf[256] = { 0 };
			for (size_t i = 0; i < token.getElementCount() - 1; ++i)
			{
				token.getStringElement(i, szBuf, _countof(szBuf));

				lua_pushstring(pL, szBuf);
				lua_rawget(pL, -2);
			}

			token.getStringElement(token.getElementCount() - 1, szBuf, _countof(szBuf));
			lua_pushstring(pL, szBuf);
			lua_rawget(pL, -2);
			if (!lua_istable(pL, -1))
				throw std::logic_error("super class not register");

			lua_pushnil(pL);
			while (lua_next(pL, -2))
			{
				const char* szKeyName = lua_tostring(pL, -2);
				if (szKeyName == nullptr)
				{
					lua_pop(pL, 1);
					continue;
				}
				if (strcmp(szKeyName, _CLASS_FUNCTION) == 0 || strcmp(szKeyName, _CLASS_PROPERTY) == 0)
				{
					lua_newtable(pL);
					lua_pushvalue(pL, -3);
					lua_pushvalue(pL, -2);
					lua_rawset(pL, nMT);

					int32_t nNewIndex = lua_gettop(pL);
					lua_pushnil(pL);
					while (lua_next(pL, -3))
					{
						lua_pushvalue(pL, -2);
						lua_pushvalue(pL, -2);
						lua_rawset(pL, nNewIndex);
						lua_pop(pL, 1);
					}
					lua_pop(pL, 1);
				}
				lua_pop(pL, 1);
			}
			lua_pop(pL, 1);
			lua_pop(pL, 1);
			lua_pop(pL, 1);
		}

		// 设置类类型
		lua_pushstring(pL, "__ctype");
		lua_pushinteger(pL, 1);
		lua_rawset(pL, -3);

		// 设置类名
		lua_pushstring(pL, "__cname");
		lua_pushstring(pL, szClassName);
		lua_rawset(pL, -3);

		// 设置创建对象函数
		lua_pushstring(pL, "new");
		lua_pushcclosure(pL, fn, 0);
		lua_rawset(pL, -3);

		// metatable.__index = __index_proxy
		lua_pushstring(pL, "__index");
		lua_pushcclosure(pL, &__index_proxy, 0);
		lua_rawset(pL, -3);

		// metatable.__newindex = __newindex_proxy
		lua_pushstring(pL, "__newindex");
		lua_pushcclosure(pL, &__newindex_proxy, 0);
		lua_rawset(pL, -3);

		// metatable.__gc = CClassBinder<T>::deleteObject
		lua_pushstring(pL, "__gc");
		lua_pushcclosure(pL, &CClassBinder<T>::deleteObject, 0);
		lua_rawset(pL, -3);

		if (szSuperClassName == nullptr)
		{
			// 设置类成员表
			lua_pushstring(pL, _CLASS_PROPERTY);
			lua_newtable(pL);
			lua_rawset(pL, -3);

			// 设置类函数表
			lua_pushstring(pL, _CLASS_FUNCTION);
			lua_newtable(pL);
			lua_rawset(pL, -3);
		}

		return CClassBinder<T>(*this);
	}

	template<class T, class F>
	CClassBinder<T> CNamespaceBinder::registerClass(const char* szClassName, F fn)
	{
		if (szClassName == nullptr)
			throw std::logic_error("invalid register class arg");

		return this->registerClassBase<T, F>(szClassName, nullptr, fn);
	}

	template<class T, class F>
	CClassBinder<T> CNamespaceBinder::registerClass(const char* szClassName, const char* szSuperClassName, F fn)
	{
		if (szClassName == nullptr || szSuperClassName == nullptr)
			throw std::logic_error("invalid register class arg");

		return this->registerClassBase<T, F>(szClassName, szSuperClassName, fn);
	}

	template<class RT, class ...Args>
	void CNamespaceBinder::registerFunction(const char* szName, RT(*fn)(Args...))
	{
		if (fn == nullptr || szName == nullptr)
			throw std::logic_error("invalid register function arg");

		lua_State* pL = this->m_pLuaFacade->getState();

		int32_t nTop = lua_gettop(pL);

		lua_pushstring(pL, szName);
		lua_rawget(pL, nTop);
		if (!lua_isnil(pL, -1))
			throw std::logic_error("dup register function");
		
		lua_pop(pL, 1);

		SStackCheck sStackCheck(pL);

		SNormalFunctionWrapper<RT, Args...>* pNormalFunctionWrapper = new SNormalFunctionWrapper<RT, Args...>();
		pNormalFunctionWrapper->pf = fn;

		lua_pushstring(pL, szName);
		lua_pushlightuserdata(pL, pNormalFunctionWrapper);
		lua_pushcclosure(pL, &__normal_invoke_proxy<RT, Args...>, 1);
		lua_rawset(pL, nTop);	// { key:szName, value:callByLua }
	}

	template<class T>
	void CNamespaceBinder::registerConstData(const char* szName, const T& val)
	{
		if (szName == nullptr)
			throw std::logic_error("invalid register const data arg");

		lua_State* pL = this->m_pLuaFacade->getState();

		int32_t nTop = lua_gettop(pL);
		lua_pushstring(pL, szName);
		lua_rawget(pL, nTop);
		if (!lua_isnil(pL, -1))
			throw std::logic_error("dup register const data");

		lua_pop(pL, 1);

		lua_pushstring(pL, szName);
		push2Lua(pL, val);
		lua_rawset(pL, nTop);
	}
}