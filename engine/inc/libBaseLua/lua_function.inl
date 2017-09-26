namespace base
{
	template<class ...Args>
	bool base::CLuaFunction::call(Args... args)
	{
		static_assert(sizeof...(Args) < 20, "not support 20 arg");

		DebugAstEx(this->m_pLuaFacade != NULL, false);
		DebugAstEx(this->m_nRef != LUA_REFNIL, false);

		lua_State* pL = this->m_pLuaFacade->getActiveLuaState();

		SStackCheck sStackCheck(pL);
		
		lua_pushcclosure(pL, onError, 0);

		int32_t nErrorIdx = lua_gettop(pL);
		lua_rawgeti(pL, LUA_REGISTRYINDEX, this->m_nRef);

		SPushArgs::push(pL, args...);

		// 坚决不能调用多余20个参数的lua函数，不然lua栈直接崩溃
		int32_t nRet = lua_pcall(pL, sizeof...(Args), 0, nErrorIdx);
		if (nRet != 0)
		{
			lua_pop(pL, 1);
		}

		lua_pop(pL, 1);

		return nRet == 0;
	}

	template<class RT, class ...Args>
	bool base::CLuaFunction::callR(RT& ret, Args... args)
	{
		static_assert(sizeof...(Args) < 20, "not support 20 arg");

		DebugAstEx(this->m_pLuaFacade != NULL, false);
		DebugAstEx(this->m_nRef != LUA_REFNIL, false);

		lua_State* pL = this->m_pLuaFacade->getActiveLuaState();

		SStackCheck sStackCheck(pL);

		lua_pushcclosure(pL, onError, 0);

		int32_t nErrorIdx = lua_gettop(pL);
		lua_rawgeti(pL, LUA_REGISTRYINDEX, this->m_nRef);

		SPushArgs::push(pL, args...);

		bool ok = true;
		// 坚决不能调用多余20个参数的lua函数，不然lua栈直接崩溃
		int32_t nRet = lua_pcall(pL, sizeof...(Args), 1, nErrorIdx);
		if (nRet == 0)
		{
			if (!read2Cpp<RT>(pL, -1, ret))
			{
				PrintWarning("read lua function ret error");
				ok = false;
			}
		}
		else
		{
			lua_pop(pL, 1);
			ok = false;
		}

		lua_pop(pL, 2);

		return ok;
	}

	template<class ...Args>
	bool CLuaFunction::call(CLuaFacade* pLuaFacade, const char* szFunName, Args... args)
	{
		static_assert(sizeof...(Args) < 20, "not support 20 arg");

		DebugAstEx(szFunName != nullptr && pLuaFacade != nullptr, false);

		lua_State* pL = pLuaFacade->getActiveLuaState();

		SStackCheck sStackCheck(pL);
		lua_getglobal(pL, szFunName);
		if (!lua_isfunction(pL, -1))
		{
			PrintWarning("%s is not a lua function", szFunName);
			lua_pop(pL, 1);
			return false;
		}

		int32_t nIdx = lua_gettop(pL);

		lua_pushcclosure(pL, onError, 0);
		int32_t nErrorIdx = lua_gettop(pL);

		lua_pushvalue(pL, nIdx);

		SPushArgs::push(pL, args...);

		// 坚决不能调用多余20个参数的lua函数，不然lua栈直接崩溃
		int32_t nRet = lua_pcall(pL, sizeof...(Args), 0, nErrorIdx);
		if (nRet != 0)
		{
			lua_pop(pL, 1);
		}

		lua_pop(pL, 2);

		return nRet == 0;
	}

	template<class RT, class ...Args>
	bool CLuaFunction::callR(CLuaFacade* pLuaFacade, const char* szFunName, RT& ret, Args... args)
	{
		static_assert(sizeof...(Args) < 20, "not support 20 arg");

		DebugAstEx(szFunName != nullptr && pLuaFacade != nullptr, false);

		lua_State* pL = pLuaFacade->getActiveLuaState();

		SStackCheck sStackCheck(pL);
		lua_getglobal(pL, szFunName);
		if (!lua_isfunction(pL, -1))
		{
			PrintWarning("%s is not a lua function", szFunName);
			lua_pop(pL, 1);
			return false;
		}

		int32_t nIdx = lua_gettop(pL);

		lua_pushcclosure(pL, onError, 0);
		int32_t nErrorIdx = lua_gettop(pL);

		lua_pushvalue(pL, nIdx);

		SPushArgs::push(pL, args...);

		bool ok = true;
		// 坚决不能调用多余20个参数的lua函数，不然lua栈直接崩溃
		int32_t nRet = lua_pcall(pL, sizeof...(Args), 1, nErrorIdx);
		if (nRet == 0)
		{
			if (!read2Cpp<RT>(pL, -1, ret))
			{
				PrintWarning("read lua function ret error");
				ok = false;
			}
		}
		else
		{
			lua_pop(pL, 1);
			ok = false;
		}

		lua_pop(pL, 3);

		return ok;
	}
}