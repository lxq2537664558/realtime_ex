
namespace base
{
	/*

	函数注册过程：
	一个函数指针由一个SFunctionWrapper对象包裹，作为以__invoke_proxy创建的闭包的upvalue。
	如果是全局函数就把这个闭包放到global table中（以函数名为KEY，闭包为VALUE）
	如果是成员函数就把这个闭包放到类的 mt中（以函数名为KEY，闭包为VALUE）

	成员变量注册过程：
	成员变量在对象中的偏移由SMemberOffsetInfo对象包裹，作为以__property_proxy创建的闭包的upvalue
	然后以变量名为KEY，闭包为VALUE放到__property表中（__property表是类的mt表的元素）

	__index过程： 
	1. 取出mt，用名字在mt中查找
	2. 如果是函数，表示这个元素是成员函数，返回
	3. 如果是其他（排除nil）的就返回错误
	4. 如果是nil就取出__property，用名字在__property查找
	5. 如果是函数，就表示是成员变量，用调用这个函数获取成员变量
	6. 返回错误


	mt
	{
		__index				= __index_proxy
		__newindex			= __newindex_proxy
		__property			= { }
		__gc				= deleteObject
		member_function1
		member_function2
		member_function3
		.
		.
		.
	}
	*/

#define _WEAK_OBJECT_TBL_NAME	"weak_object_table"
#define _REF_OBJECT_TBL_NAME	"ref_object_table"
#define _FACADE_NAME			"facade"


	struct SObjectWraper
	{
		void*	pObject;	// 真正的对象
		bool	bGc;		// true 表示这个对象的生命周期完全是由lua控制的（lua创建，lua销毁），false 表示这个对象的生命周期不是由lua控制的，一般是返回值，参数什么的，此种对象不应该在lua中保存，否则该对象的删除就会出现混乱
		int32_t	nRefCount;	// 对于生命周期由lua控制的对象，该变量标示在C++层面引用计数次数，对于不是由lua控制生命周期的对象，该变量没有意义
	};

	template<class T>
	struct SClassName
	{
		static inline void setName(const char* szClassName)
		{
			s_szClassName = szClassName;
		}
		static inline const char* getName()
		{
			return s_szClassName;
		}

		static const char* s_szClassName;
	};
	template<class T>
	const char* SClassName<T>::s_szClassName;

	struct SStackCheck
	{
		int32_t		m_nTop;
		lua_State*	m_pL;

		SStackCheck(lua_State* pL)
		{
			this->m_pL = pL;
			this->m_nTop = lua_gettop(this->m_pL);
		}

		~SStackCheck()
		{
			int32_t nTop = lua_gettop(this->m_pL);
			if (nTop != this->m_nTop)
			{
				PrintWarning("lua stack error");
			}
		}
	};

	inline void printStack(lua_State* pL, int32_t n)
	{
		SStackCheck sStackCheck(pL);

		lua_Debug ar;
		if (lua_getstack(pL, n, &ar))
		{
			lua_getinfo(pL, "Sln", &ar);

			if (ar.name != NULL)
				PrintInfo("\tstack[%d] -> line %d : %s()[%s : line %d]", n, ar.currentline, ar.name, ar.short_src, ar.linedefined);
			else
				PrintInfo("\tstack[%d] -> line %d : unknown[%s : line %d]", n, ar.currentline, ar.short_src, ar.linedefined);

			printStack(pL, n + 1);
		}
	}

	// 不支持非指针对象传递
	template<class T>
	struct SLua2CppType
	{
		inline static T convert(lua_State* pL, int32_t nIndex)
		{
#ifdef _WIN32
			// 下面代码故意语法错误的，不要去改
			err;
			// 上面代码故意语法错误的，不要去改
#else
			return T();
#endif
		}
	};

	template<class T>
	struct SLua2CppType<T*>
	{
		inline static T* convert(lua_State* pL, int32_t nIndex)
		{
			SObjectWraper* pObjectWraper = reinterpret_cast<SObjectWraper*>(lua_touserdata(pL, nIndex));
			DebugAstEx(pObjectWraper != nullptr, nullptr);
			return static_cast<T*>(pObjectWraper->pObject);
		}
	};

	template<class T>
	inline T read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isuserdata(pL, nIndex))
		{
			bError = true;

			return nullptr;
		}

		return SLua2CppType<T>::convert(pL, nIndex);
	};

	template<> inline void read2Cpp(lua_State* pL, int32_t nIndex, bool& bError) {}

	template<> inline bool read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isboolean(pL, nIndex))
		{
			bError = true;

			return false;
		}
		return lua_toboolean(pL, nIndex) ? true : false;
	}
	template<> inline char* read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isstring(pL, nIndex))
		{
			bError = true;

			return nullptr;
		}
		return (char*)lua_tostring(pL, nIndex);
	}
	template<> inline const char* read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isstring(pL, nIndex))
		{
			bError = true;

			return nullptr;
		}
		return (const char*)lua_tostring(pL, nIndex);
	}
	template<> inline int8_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (int8_t)lua_tointeger(pL, nIndex);
	}
	template<> inline uint8_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (uint8_t)lua_tointeger(pL, nIndex);
	}
	template<> inline int16_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (int16_t)lua_tointeger(pL, nIndex);
	}
	template<> inline uint16_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (uint16_t)lua_tointeger(pL, nIndex);
	}
	template<> inline int32_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (int32_t)lua_tointeger(pL, nIndex);
	}
	template<> inline uint32_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (uint32_t)lua_tointeger(pL, nIndex);
	}
	template<> inline int64_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (int64_t)lua_tointeger(pL, nIndex);
	}
	template<> inline uint64_t read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isinteger(pL, nIndex))
		{
			bError = true;

			return 0;
		}
		return (uint64_t)lua_tointeger(pL, nIndex);
	}
	template<> inline float read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isnumber(pL, nIndex))
		{
			bError = true;

			return 0.0f;
		}
		return (float)lua_tonumber(pL, nIndex);
	}
	template<> inline double read2Cpp(lua_State* pL, int32_t nIndex, bool& bError)
	{
		bError = false;
		if (!lua_isnumber(pL, nIndex))
		{
			bError = true;

			return 0.0;
		}
		return (double)lua_tonumber(pL, nIndex);
	}

	template<class T>
	struct Convert2LuaType
	{
		inline static void convertType(lua_State* pL, T& val)
		{
			DebugAst("unsupport ref and value");
		}
	};

	template<class T>
	struct Convert2LuaType<const T*>
	{
		inline static void convertType(lua_State* pL, const T* val)
		{
			lua_getglobal(pL, _WEAK_OBJECT_TBL_NAME);
			int32_t tbl = lua_gettop(pL);
			///< 获取弱表中的包裹对象
			lua_pushlightuserdata(pL, (void*)val);
			lua_rawget(pL, -2);
			if (lua_isnil(pL, -1))
			{
				lua_pop(pL, 1);

				///< 创建对象的包裹内存块
				SObjectWraper* pObjectWraper = reinterpret_cast<SObjectWraper*>(lua_newuserdata(pL, sizeof(SObjectWraper)));

				pObjectWraper->pObject = (void*)val;
				pObjectWraper->bGc = false;
				pObjectWraper->nRefCount = 0;
				luaL_getmetatable(pL, SClassName<T>::getName());
				///< 设置对象的metatable
				lua_setmetatable(pL, -2);

				lua_pushlightuserdata(pL, (void*)val);
				lua_pushvalue(pL, -2);
				///< 将对象设置进弱表中 weak_object_table[pObject] = pObjectWraper
				lua_rawset(pL, tbl);
			}
			else
			{
				SObjectWraper* pObjectWraper = reinterpret_cast<SObjectWraper*>(lua_touserdata(pL, -1));
				if (pObjectWraper == nullptr)
					lua_pushnil(pL);
				if (pObjectWraper->pObject == nullptr)
					lua_pushnil(pL);
			}

			lua_replace(pL, tbl);
			lua_settop(pL, tbl);
		}
	};

	template<class T>
	struct Convert2LuaType<T*>
	{
		inline static void convertType(lua_State* pL, T* val)
		{
			Convert2LuaType<const T*>::convertType(pL, val);
		}
	};

	template<class T>
	inline void push2Lua(lua_State* pL, T val)
	{
#ifdef _WIN32
		///< 下面代码故意语法错误的，不要去改
		if
			///< 上面代码故意语法错误的，不要去改
#endif
	}

	template<class T>
	inline void push2Lua(lua_State* pL, T* val)
	{
		Convert2LuaType<T*>::convertType(pL, val);
	};

	template<> inline void push2Lua(lua_State* pL, int8_t val)		{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, uint8_t val)		{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, int16_t val)		{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, uint16_t val)	{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, int32_t val)		{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, uint32_t val)	{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, int64_t val)		{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, uint64_t val)	{ lua_pushinteger(pL, val); }
	template<> inline void push2Lua(lua_State* pL, float val)		{ lua_pushnumber(pL, val); }
	template<> inline void push2Lua(lua_State* pL, double val)		{ lua_pushnumber(pL, val); }
	template<> inline void push2Lua(lua_State* pL, char* val)		{ lua_pushstring(pL, val); }
	template<> inline void push2Lua(lua_State* pL, const char* val) { lua_pushstring(pL, val); }
	template<> inline void push2Lua(lua_State* pL, bool val)		{ lua_pushboolean(pL, val); }

	inline int32_t __index_proxy(lua_State* pL)
	{
		lua_getmetatable(pL, 1);
		if (!lua_istable(pL, -1))
			return 0;

		lua_pushvalue(pL, 2);
		lua_rawget(pL, -2);
		// 成员函数
		if (lua_isfunction(pL, -1))
			return 1;

		if (!lua_isnil(pL, -1))
			return 0;

		lua_pop(pL, 1);
		lua_pushstring(pL, "__property");
		lua_rawget(pL, -2);
		if (!lua_istable(pL, -1))
			return 0;

		lua_pushvalue(pL, 2);
		lua_rawget(pL, -2);
		// 成员变量
		if (!lua_isfunction(pL, -1))
			return 0;
		
		lua_pushvalue(pL, 1);
		lua_pushboolean(pL, false);
		lua_call(pL, 2, 1);

		return 1;
	}

	inline int32_t __newindex_proxy(lua_State* pL)
	{
		lua_getmetatable(pL, 1);
		if (!lua_istable(pL, -1))
			return 0;

		lua_pushstring(pL, "__property");
		lua_rawget(pL, -2);
		if (!lua_istable(pL, -1))
			return 0;

		lua_pushvalue(pL, 2);
		lua_rawget(pL, -2);
		if (!lua_isfunction(pL, -1))
			return 0;

		lua_pushvalue(pL, 1);
		lua_pushboolean(pL, true);
		lua_pushvalue(pL, 3);
		lua_call(pL, 3, 0);
		return 0;
	}

	template<typename RT, typename ...Args>
	struct SNormalFunctionWrapper
	{
		typedef RT(*FUN_TYPE)(Args...);

		FUN_TYPE	pf;
	};

	template<typename T, typename RT, typename ...Args>
	struct SClassFunctionWrapper
	{
		typedef RT(T::*FUN_TYPE)(Args...);

		FUN_TYPE	pf;
	};

	template<typename RT, typename ...Args>
	struct SFunctionBaseWrapper
	{
		std::function<RT(Args...)>	invoke;
	};

	struct SPushArgs
	{
		template<typename T, typename...NowArgs>
		static bool push(lua_State* pL, T& t, NowArgs&... args)
		{
			push2Lua(pL, t);

			return SPushArgs::push(pL, args...);
		}

		static bool push(lua_State* pL)
		{
			return true;
		}
	};

	template<int32_t ARG_COUNT, typename RT, typename ...Args>
	struct SParseArgs;
	
	template<typename RT, typename ...Args>
	struct SParseNowArg
	{
		template<typename T, typename ...RemainArgs, typename ...NowArgs>
		static int32_t parse(lua_State* pL, int32_t& nIndex, SFunctionBaseWrapper<RT, Args...>& sFunctionBaseWrapper, NowArgs&&... args)
		{
			bool bError = false;
			T value = read2Cpp<T>(pL, nIndex++, bError);
			DebugAstEx(!bError, 0);
			
			return SParseArgs<sizeof...(RemainArgs), RT, Args...>::template parse<RemainArgs...>(pL, nIndex, sFunctionBaseWrapper, args..., value);
		}
	};

	template<int32_t ARG_COUNT, typename RT, typename ...Args>
	struct SParseArgs
	{
		template<typename ...RemainArgs, typename ...NowArgs>
		static int32_t parse(lua_State* pL, int32_t& nIndex, SFunctionBaseWrapper<RT, Args...>& sFunctionBaseWrapper, NowArgs&&... args)
		{
			return SParseNowArg<RT, Args...>::template parse<RemainArgs...>(pL, nIndex, sFunctionBaseWrapper, args...);
		}
	};

	template<typename RT, typename ...Args>
	struct SParseArgs<0, RT, Args...>
	{
		template<typename ...NowArgs>
		static int32_t parse(lua_State* pL, int32_t& nIndex, SFunctionBaseWrapper<RT, Args...>& sFunctionBaseWrapper, NowArgs&&... args)
		{
			RT ret = sFunctionBaseWrapper.invoke(args...);
			
			push2Lua(pL, ret);
			
			return 1;
		}
	};

	template<typename ...Args>
	struct SParseArgs<0, void, Args...>
	{
		template<typename ...NowArgs>
		static int32_t parse(lua_State* pL, int32_t& nIndex, SFunctionBaseWrapper<void, Args...>& sFunctionBaseWrapper, NowArgs&&... args)
		{
			sFunctionBaseWrapper.invoke(args...);

			return 0;
		}
	};

	template<class T, class M>
	struct SMemberOffsetInfo
	{
		typedef const M T::*mp_t;
		mp_t mp;
	};

	template<typename RT, typename ...Args>
	int32_t __normal_invoke_proxy(lua_State* pL)
	{
		// 当lua调用C++函数给的参数不够时，在读取栈上的函数时会返回nil对象，BaseScript层会报错
		// 当lua调用C++函数给的参数多余时，直接忽略
		// 当lua调用C++函数给的参数类型错误时，在读取栈上的函数时BaseScript层会报错
		// 坚决不能调用多余20个参数的C++函数，不然lua栈直接崩溃
		PROFILING_GUARD(__normal_invoke_proxy)
		lua_getglobal(pL, _FACADE_NAME);
		CLuaFacade* pLuaFacade = reinterpret_cast<CLuaFacade*>(lua_touserdata(pL, -1));
		lua_pop(pL, 1);
		DebugAstEx(pLuaFacade != nullptr, 0);

		pLuaFacade->setActiveLuaState(pL);
		SNormalFunctionWrapper<RT, Args...>* pNormalFunctionWrapper = reinterpret_cast<SNormalFunctionWrapper<RT, Args...>*>(lua_touserdata(pL, lua_upvalueindex(1)));
		DebugAstEx(pNormalFunctionWrapper != nullptr, 0);

		SFunctionBaseWrapper<RT, Args...> sFunctionBaseWrapper;
		sFunctionBaseWrapper.invoke = pNormalFunctionWrapper->pf;

		int32_t nIndex = 1;
		int32_t nRet = SParseArgs<sizeof...(Args), RT, Args...>::template parse<Args...>(pL, nIndex, sFunctionBaseWrapper);
		
		pLuaFacade->setActiveLuaState(nullptr);

		return nRet;
	}
	
	template<typename T, typename RT, typename ...Args>
	int32_t __class_invoke_proxy(lua_State* pL)
	{
		// 当lua调用C++函数给的参数不够时，在读取栈上的函数时会返回nil对象，libBaseLua会报错
		// 当lua调用C++函数给的参数多余时，直接忽略
		// 当lua调用C++函数给的参数类型错误时，在读取栈上的函数时libBaseLua会报错
		// 坚决不能调用多余20个参数的C++函数，不然lua栈直接崩溃
		PROFILING_GUARD(__class_invoke_proxy)
		lua_getglobal(pL, _FACADE_NAME);
		CLuaFacade* pLuaFacade = reinterpret_cast<CLuaFacade*>(lua_touserdata(pL, -1));
		lua_pop(pL, 1);
		DebugAstEx(pLuaFacade != nullptr, 0);

		pLuaFacade->setActiveLuaState(pL);
		SClassFunctionWrapper<T, RT, Args...>* pClassFunctionWrapper = reinterpret_cast<SClassFunctionWrapper<T, RT, Args...>*>(lua_touserdata(pL, lua_upvalueindex(1)));
		DebugAstEx(pClassFunctionWrapper != nullptr, 0);

		bool bError = false;
		T* pObject = read2Cpp<T*>(pL, 1, bError);
		DebugAstEx(!bError && pObject != nullptr, 0);

		SFunctionBaseWrapper<RT, Args...> sFunctionBaseWrapper;
		sFunctionBaseWrapper.invoke = [&](Args... args)->RT
		{
			return (pObject->*pClassFunctionWrapper->pf)(args...);
		};

		int32_t nIndex = 2;
		int32_t nRet = SParseArgs<sizeof...(Args), RT, Args...>::template parse<Args...>(pL, nIndex, sFunctionBaseWrapper);
		pLuaFacade->setActiveLuaState(nullptr);

		return nRet;
	}

	template<class T, class M>
	inline int32_t __property_proxy(lua_State* pL)
	{
		bool bError = false;
		T* pObject = read2Cpp<T*>(pL, 1, bError);
		DebugAstEx(!bError && pObject != nullptr, 0);

		bool bWrite = read2Cpp<bool>(pL, 2, bError);
		DebugAstEx(!bError, 0);
		
		SMemberOffsetInfo<T, M>* pMemberOffsetInfo = reinterpret_cast<SMemberOffsetInfo<T, M>*>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (bWrite)
		{
			M m = read2Cpp<M>(pL, 3, bError);
			DebugAstEx(!bError, 0);
			pObject->*const_cast<M T::*>(pMemberOffsetInfo->mp) = m;
			return 0;
		}
		else
		{
			push2Lua(pL, pObject->*pMemberOffsetInfo->mp);
			return 1;
		}
	}

	template<class T>
	inline int32_t __static_property_proxy(lua_State* pL)
	{
		bool bError = false;
		bool bWrite = read2Cpp<bool>(pL, 2, bError);
		DebugAstEx(!bError, 0);
		T* pValue = reinterpret_cast<T*>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (bWrite)
		{
			T val = read2Cpp<T>(pL, 3, bError);
			DebugAstEx(!bError, 0);
			*pValue = val;
			return 0;
		}
		else
		{
			push2Lua(pL, *pValue);
			return 1;
		}
	}

	template<class T, class F>
	void CLuaFacade::registerClass(const char* szClassName, F pfFun)
	{
		DebugAst(szClassName != nullptr);
		DebugAst(SClassName<T>::getName() == nullptr);

		SStackCheck sStackCheck(this->m_pMainLuaState);

		lua_getglobal(this->m_pMainLuaState, szClassName);
		bool bExist = !lua_isnil(this->m_pMainLuaState, -1);
		lua_pop(this->m_pMainLuaState, 1);

		if (bExist)
		{
			PrintWarning("dup registerClass class_name: %s", szClassName);
			return;
		}

		SClassName<T>::setName(szClassName);

		char szCreateBuf[256] = { 0 };
		base::crt::snprintf(szCreateBuf, _countof(szCreateBuf), "%s_create", szClassName);
		// 设置创建对象函数
		lua_pushcclosure(this->m_pMainLuaState, pfFun, 0);
		lua_setglobal(this->m_pMainLuaState, szCreateBuf);

		// 创建这个class的metatable
		luaL_newmetatable(this->m_pMainLuaState, szClassName);

		// metatable.__index = __index
		lua_pushstring(this->m_pMainLuaState, "__index");
		lua_pushcclosure(this->m_pMainLuaState, &__index_proxy, 0);
		lua_rawset(this->m_pMainLuaState, -3);

		// metatable.__newindex = __newindex
		lua_pushstring(this->m_pMainLuaState, "__newindex");
		lua_pushcclosure(this->m_pMainLuaState, &__newindex_proxy, 0);
		lua_rawset(this->m_pMainLuaState, -3);

		// metatable.__gc = deleteObject<T>
		lua_pushstring(this->m_pMainLuaState, "__gc");
		lua_pushcclosure(this->m_pMainLuaState, &lua_helper::deleteObject<T>, 0);
		lua_rawset(this->m_pMainLuaState, -3);

		// pop metatable
		lua_pop(this->m_pMainLuaState, 1);

		char szBuf[256] = { 0 };
		base::crt::snprintf(szBuf, _countof(szBuf), "%s = {} function %s.new(...) return %s_create(...) end", szClassName, szClassName, szClassName);
		this->runString(szBuf, nullptr);
	}

	template<typename T, typename RT, typename ...Args>
	void CLuaFacade::registerClassFunction(const char* szName, RT(T::*pfFun)(Args...))
	{
		DebugAst(pfFun != nullptr && szName != nullptr);

		SStackCheck sStackCheck(this->m_pMainLuaState);

		luaL_getmetatable(this->m_pMainLuaState, SClassName<T>::getName());
		if (!lua_istable(this->m_pMainLuaState, -1))
		{
			PrintWarning("please register class %s", SClassName<T>::getName());
			lua_pop(this->m_pMainLuaState, 1);
			DebugAst(false);
			return;
		}

		lua_pushstring(this->m_pMainLuaState, szName);
		lua_rawget(this->m_pMainLuaState, -2);
		bool bExist = !lua_isnil(this->m_pMainLuaState, -1);
		lua_pop(this->m_pMainLuaState, 1);
		if (bExist)
		{
			PrintWarning("dup registerClassFunction function_name: %s", szName);
			return;
		}

		SClassFunctionWrapper<T, RT, Args...>* pClassFunctionWrapper = new SClassFunctionWrapper<T, RT, Args...>();

		pClassFunctionWrapper->pf = pfFun;
		
		lua_pushstring(this->m_pMainLuaState, szName);
		lua_pushlightuserdata(this->m_pMainLuaState, pClassFunctionWrapper);
		lua_pushcclosure(this->m_pMainLuaState, &__class_invoke_proxy<T, RT, Args...>, 1);
		lua_rawset(this->m_pMainLuaState, -3);

		lua_pop(this->m_pMainLuaState, 1);
	}
	
	template<typename T, typename RT, typename ...Args>
	void CLuaFacade::registerClassFunction(const char* szName, RT(T::*pfFun)(Args...) const)
	{
		DebugAst(pfFun != nullptr && szName != nullptr);

		SStackCheck sStackCheck(this->m_pMainLuaState);

		luaL_getmetatable(this->m_pMainLuaState, SClassName<T>::getName());
		if (!lua_istable(this->m_pMainLuaState, -1))
		{
			PrintWarning("please register class %s", SClassName<T>::getName());
			lua_pop(this->m_pMainLuaState, 1);
			DebugAst(false);
			return;
		}

		lua_pushstring(this->m_pMainLuaState, szName);
		lua_rawget(this->m_pMainLuaState, -2);
		bool bExist = !lua_isnil(this->m_pMainLuaState, -1);
		lua_pop(this->m_pMainLuaState, 1);
		if (bExist)
		{
			PrintWarning("dup registerClassFunction function_name: %s", szName);
			return;
		}

		SClassFunctionWrapper<T, RT, Args...>* pFunctionWrapper = new SClassFunctionWrapper<T, RT, Args...>();
		pFunctionWrapper->pf = reinterpret_cast<typename SClassFunctionWrapper<T, RT, Args...>::FUN_TYPE>(pfFun);
		
		lua_pushstring(this->m_pMainLuaState, szName);
		lua_pushlightuserdata(this->m_pMainLuaState, pFunctionWrapper);
		lua_pushcclosure(this->m_pMainLuaState, &__class_invoke_proxy<T, RT, Args...>, 1);
		lua_rawset(this->m_pMainLuaState, -3);

		lua_pop(this->m_pMainLuaState, 1);
	}

	template<typename RT, typename ...Args>
	void CLuaFacade::registerFunction(const char* szName, RT(*pfFun)(Args...))
	{
		DebugAst(pfFun != nullptr && szName != nullptr);
		
		lua_getglobal(this->m_pMainLuaState, szName);
		bool bExist = !lua_isnil(this->m_pMainLuaState, -1);
		lua_pop(this->m_pMainLuaState, 1);

		if (bExist)
		{
			PrintWarning("dup registerFunction function_name: %s", szName);
			return;
		}

		SStackCheck sStackCheck(this->m_pMainLuaState);

		SNormalFunctionWrapper<RT, Args...>* pNormalFunctionWrapper = new SNormalFunctionWrapper<RT, Args...>();
		pNormalFunctionWrapper->pf = pfFun;

		lua_pushlightuserdata(this->m_pMainLuaState, pNormalFunctionWrapper);
		lua_pushcclosure(this->m_pMainLuaState, &__normal_invoke_proxy<RT, Args...>, 1);
		lua_setglobal(this->m_pMainLuaState, szName);	///< { key:szName, value:CallByLua }
	}
	
	template<class T>
	void CLuaFacade::registerConstData(const char* szName, const T& val)
	{
		DebugAst(szName != nullptr);

		lua_getglobal(this->m_pMainLuaState, szName);
		bool bExist = !lua_isnil(this->m_pMainLuaState, -1);
		lua_pop(this->m_pMainLuaState, 1);

		if (bExist)
		{
			PrintWarning("dup registerConstData function_name: %s", szName);
			return;
		}

		lua_pushnumber(this->m_pMainLuaState, val);
		lua_setglobal(this->m_pMainLuaState, szName);
	}

	template<class T, class M>
	void CLuaFacade::registerClassMember(const char* szName, const M T::* pMember)
	{
		SStackCheck sStackCheck(this->m_pMainLuaState);
		luaL_getmetatable(this->m_pMainLuaState, SClassName<T>::getName());
		if (!lua_istable(this->m_pMainLuaState, -1))
		{
			PrintWarning("please register class %s", SClassName<T>::getName());
			lua_pop(this->m_pMainLuaState, 1);
			DebugAst(false);
			return;
		}

		lua_pushstring(this->m_pMainLuaState, "__property");
		lua_rawget(this->m_pMainLuaState, -2);
		if (lua_isnil(this->m_pMainLuaState, -1))
		{
			lua_pop(this->m_pMainLuaState, 1);
			lua_newtable(this->m_pMainLuaState);
			lua_pushstring(this->m_pMainLuaState, "__property");
			lua_pushvalue(this->m_pMainLuaState, -2);
			lua_rawset(this->m_pMainLuaState, -4);
		}
		else
		{
			if (!lua_istable(this->m_pMainLuaState, -1))
			{
				PrintWarning("unkonw class __property class %s", SClassName<T>::getName());
				return;
			}
		}

		lua_pushstring(this->m_pMainLuaState, szName);
		lua_rawget(this->m_pMainLuaState, -2);
		bool bExist = !lua_isnil(this->m_pMainLuaState, -1);
		lua_pop(this->m_pMainLuaState, 1);
		if (bExist)
		{
			PrintWarning("dup registerClassMember member_name: %s", szName);
			return;
		}

		SMemberOffsetInfo<T, M>* pMemberOffsetInfo = new SMemberOffsetInfo<T, M>();
		pMemberOffsetInfo->mp = pMember;
		lua_pushstring(this->m_pMainLuaState, szName);
		lua_pushlightuserdata(this->m_pMainLuaState, pMemberOffsetInfo);
		lua_pushcclosure(this->m_pMainLuaState, &__property_proxy<T, M>, 1);
		lua_rawset(this->m_pMainLuaState, -3);

		lua_pop(this->m_pMainLuaState, 2);
	}

	template<class M>
	void CLuaFacade::registerClassStaticMember(const char* szClassName, const char* szName, M* pMember)
	{
		DebugAst(pMember != nullptr && szClassName != nullptr);

		SStackCheck sStackCheck(this->m_pMainLuaState);
		luaL_getmetatable(this->m_pMainLuaState, szClassName);
		if (!lua_istable(this->m_pMainLuaState, -1))
		{
			PrintWarning("please register class %s", szClassName);
			lua_pop(this->m_pMainLuaState, 1);
			DebugAst(false);
			return;
		}

		lua_pushstring(this->m_pMainLuaState, "__property");
		lua_rawget(this->m_pMainLuaState, -2);
		if (lua_isnil(this->m_pMainLuaState, -1))
		{
			lua_pop(this->m_pMainLuaState, 1);
			lua_newtable(this->m_pMainLuaState);
			lua_pushstring(this->m_pMainLuaState, "__property");
			lua_pushvalue(this->m_pMainLuaState, -2);
			lua_rawset(this->m_pMainLuaState, -4);
		}
		else
		{
			if (!lua_istable(this->m_pMainLuaState, -1))
			{
				PrintWarning("unkonw class __property class %s", szClassName);
				return;
			}
		}

		lua_pushstring(this->m_pMainLuaState, szName);
		lua_rawget(this->m_pMainLuaState, -2);
		bool bExist = !lua_isnil(this->m_pMainLuaState, -1);
		lua_pop(this->m_pMainLuaState, 1);
		if (bExist)
		{
			PrintWarning("dup registerClassStaticMember member_name: %s", szName);
			return;
		}

		lua_pushstring(this->m_pMainLuaState, szName);
		
		
		lua_pushlightuserdata(this->m_pMainLuaState, pMember);
		lua_pushcclosure(this->m_pMainLuaState, &__static_property_proxy<M>, 1);
		lua_rawset(this->m_pMainLuaState, -3);

		lua_pop(this->m_pMainLuaState, 2);
	}

	namespace lua_helper
	{
		template<typename T, typename ...Args>
		int32_t createObject(lua_State* pL)
		{
			SFunctionBaseWrapper<void, Args...> sFunctionBaseWrapper;
			sFunctionBaseWrapper.invoke = [&](Args... args)
			{
				T* pObject = new T(args...);
				lua_getglobal(pL, _WEAK_OBJECT_TBL_NAME);
				int32_t tbl = lua_gettop(pL);
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
					///< 设置对象的metatable
					luaL_getmetatable(pL, SClassName<T>::getName());
					lua_setmetatable(pL, -2);

					lua_pushlightuserdata(pL, pObject);
					lua_pushvalue(pL, -2);
					///< 将对象设置进弱表中 weak_object_table[pObject] = pObjectWraper
					lua_rawset(pL, tbl);
					lua_replace(pL, tbl);
					lua_settop(pL, tbl);
				}
				else
				{
					// 可能对象已经被C++层销毁，但lua的gc尚未执行，所以一个导出到lua的对象在析构时需要主动的调用下面的unBindObject，去释放弱表中的引用
					lua_settop(pL, tbl - 1);
					lua_pushnil(pL);
					DebugAst(!"object is exist");
				}
			};

			lua_getglobal(pL, _FACADE_NAME);
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
		int32_t deleteObject(lua_State* pL)
		{
			SObjectWraper* pObjectWraper = reinterpret_cast<SObjectWraper*>(lua_touserdata(pL, 1));
			if (!pObjectWraper->bGc)
			{
				pObjectWraper->pObject = nullptr;
				return 0;
			}
			lua_getglobal(pL, _FACADE_NAME);
			CLuaFacade* pLuaFacade = reinterpret_cast<CLuaFacade*>(lua_touserdata(pL, -1));
			lua_pop(pL, 1);
			DebugAstEx(pLuaFacade != nullptr, 0);

			pLuaFacade->setActiveLuaState(pL);

			delete reinterpret_cast<T*>(pObjectWraper->pObject);
			pObjectWraper->pObject = nullptr;

			pLuaFacade->setActiveLuaState(nullptr);
			return 0;
		}
	}

	template<typename ...Args>
	bool CLuaFacade::call(const char* szFunName, Args... args)
	{
		DebugAstEx(szFunName != nullptr, false);

		lua_State* pL = this->getActiveLuaState();

		SStackCheck sStackCheck(pL);
		lua_getglobal(pL, szFunName);
		if (!lua_isfunction(pL, -1))
		{
			PrintWarning("%s is not a lua function", szFunName);
			lua_pop(pL, 1);
			return false;
		}

		int32_t nIdx = lua_gettop(pL);

		lua_pushcclosure(pL, lua_helper::onError, 0);
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

	template<typename RT, typename ...Args>
	bool CLuaFacade::callR(const char* szFunName, RT& ret, Args... args)
	{
		DebugAstEx(szFunName != nullptr, false);

		lua_State* pL = this->getActiveLuaState();

		SStackCheck sStackCheck(pL);
		lua_getglobal(pL, szFunName);
		if (!lua_isfunction(pL, -1))
		{
			PrintWarning("%s is not a lua function", szFunName);
			lua_pop(pL, 1);
			return false;
		}

		int32_t nIdx = lua_gettop(pL);

		lua_pushcclosure(pL, lua_helper::onError, 0);
		int32_t nErrorIdx = lua_gettop(pL);

		lua_pushvalue(pL, nIdx);

		SPushArgs::push(pL, args...);

		bool ok = true;
		// 坚决不能调用多余20个参数的lua函数，不然lua栈直接崩溃
		int32_t nRet = lua_pcall(pL, sizeof...(Args), 1, nErrorIdx);
		if (nRet == 0)
		{
			bool bError = false;
			ret = read2Cpp<RT>(pL, -1, bError);
			if (bError)
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