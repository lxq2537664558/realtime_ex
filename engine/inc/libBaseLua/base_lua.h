#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/base_function.h"
#include "libBaseCommon/token_parser.h"

#include "lua-5.3.3/lua.hpp"

#ifdef _WIN32
#	ifdef __BUILD_BASE_LUA_DLL__
#		define __BASE_LUA_API__ __declspec(dllexport)
#	else
#		define __BASE_LUA_API__ __declspec(dllimport)
#	endif
#else
#	define __BASE_LUA_API__
#endif

namespace base
{
	/*

	����ע����̣�
	һ������ָ����һ��SFunctionWrapper�����������Ϊ��__invoke_proxy�����ıհ���upvalue��
	�����ȫ�ֺ����Ͱ�����հ��ŵ�global table�У��Ժ�����ΪKEY���հ�ΪVALUE��
	����ǳ�Ա�����Ͱ�����հ��ŵ���� mt�У��Ժ�����ΪKEY���հ�ΪVALUE��

	��Ա����ע����̣�
	��Ա�����ڶ����е�ƫ����SMemberOffsetInfo�����������Ϊ��__property_proxy�����ıհ���upvalue
	Ȼ���Ա�����ΪKEY���հ�ΪVALUE�ŵ�__property���У�__property�������mt���Ԫ�أ�

	__index���̣� 
	1. ȡ��mt����������mt�в���
	2. ����Ǻ�������ʾ���Ԫ���ǳ�Ա����������
	3. ������������ų�nil���ľͷ��ش���
	4. �����nil��ȡ��__property����������__property����
	5. ����Ǻ������ͱ�ʾ�ǳ�Ա�������õ������������ȡ��Ա����
	6. ���ش���


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
#define _CLASS_PROPERTY			"__property"
#define _CLASS_FUNCTION			"__function"

	struct SObjectWraper
	{
		void*	pObject;	// �����Ķ���
		bool	bGc;		// true ��ʾ������������������ȫ����lua���Ƶģ�lua������lua���٣���false ��ʾ���������������ڲ�����lua���Ƶģ�һ���Ƿ���ֵ������ʲô�ģ����ֶ���Ӧ����lua�б��棬����ö����ɾ���ͻ���ֻ���
		int32_t	nRefCount;	// ��������������lua���ƵĶ��󣬸ñ�����ʾ��C++�������ü������������ڲ�����lua�����������ڵĶ��󣬸ñ���û������
	};

	template<class T>
	struct SClassName
	{
		static inline void setName(const char* szClassName)
		{
			size_t nLen = crt::strnlen(szClassName, _TRUNCATE);
			s_szClassName = new char[nLen + 1];
			crt::strcpy(s_szClassName, nLen + 1, szClassName);
		}

		static inline void setNamespaceName(const char* szNamespaceName)
		{
			size_t nLen = crt::strnlen(szNamespaceName, _TRUNCATE);
			s_szNamespaceName = new char[nLen + 1];
			crt::strcpy(s_szNamespaceName, nLen + 1, szNamespaceName);
		}

		static inline const char* getName()
		{
			return s_szClassName;
		}

		static inline const char* getNamespaceName()
		{
			return s_szNamespaceName;
		}

		static char* s_szClassName;
		static char* s_szNamespaceName;
	};

	template<class T>
	char* SClassName<T>::s_szClassName;
	template<class T>
	char* SClassName<T>::s_szNamespaceName;

	inline void getMetatable(lua_State* pL, const char* szNamespaceName, const char* szName)
	{
		CTokenParser token;
		token.parse(szNamespaceName, '.');

		int32_t nTop = lua_gettop(pL);

		lua_pushinteger(pL, LUA_RIDX_GLOBALS);
		lua_rawget(pL, LUA_REGISTRYINDEX);
		char szBuf[256] = { 0 };
		for (size_t i = 0; i < token.getElementCount(); ++i)
		{
			token.getStringElement(i, szBuf, _countof(szBuf));

			lua_pushstring(pL, szBuf);
			lua_rawget(pL, -2);
		}

		lua_pushstring(pL, szName);
		lua_rawget(pL, -2);

		lua_replace(pL, nTop + 1);
		lua_settop(pL, nTop + 1);
	}

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

	inline int32_t onError(lua_State* pL)
	{
		PrintWarning("lua error : %s", lua_tostring(pL, -1));

		printStack(pL, 0);

		return 0;
	}

	// ��֧�ַ�ָ����󴫵�
	template<class T>
	struct SLua2CppType
	{
		inline static T convert(lua_State* pL, int32_t nIndex)
		{
			static_assert(false, "unsupport ref and value");
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
			static_assert(false, "unsupport ref and value");
		}
	};

	template<class T>
	struct Convert2LuaType<const T*>
	{
		inline static void convertType(lua_State* pL, const T* val)
		{
			lua_pushstring(pL, _WEAK_OBJECT_TBL_NAME);
			lua_rawget(pL, LUA_REGISTRYINDEX);
			int32_t tbl = lua_gettop(pL);
			// ��ȡ�����еİ�������
			lua_pushlightuserdata(pL, (void*)val);
			lua_rawget(pL, -2);
			if (lua_isnil(pL, -1))
			{
				lua_pop(pL, 1);

				// ��������İ����ڴ��
				SObjectWraper* pObjectWraper = reinterpret_cast<SObjectWraper*>(lua_newuserdata(pL, sizeof(SObjectWraper)));

				pObjectWraper->pObject = (void*)val;
				pObjectWraper->bGc = false;
				pObjectWraper->nRefCount = 0;
				getMetatable(pL, SClassName<T>::getNamespaceName(), SClassName<T>::getName());
				// ���ö����metatable
				lua_setmetatable(pL, -2);

				lua_pushlightuserdata(pL, (void*)val);
				lua_pushvalue(pL, -2);
				// ���������ý������� weak_object_table[pObject] = pObjectWraper
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
		static_assert(false, "not suppert val object");
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
		int32_t a = lua_gettop(pL);
		lua_getmetatable(pL, 1);
		if (!lua_istable(pL, -1))
			return 0;

		lua_pushstring(pL, _CLASS_FUNCTION);
		lua_rawget(pL, -2);
		if (!lua_istable(pL, -1))
			return 0;

		lua_pushvalue(pL, 2);
		lua_rawget(pL, -2);
		// ��Ա����
		if (lua_isfunction(pL, -1))
			return 1;

		if (!lua_isnil(pL, -1))
			return 0;

		lua_pop(pL, 2);
		lua_pushstring(pL, _CLASS_PROPERTY);
		lua_rawget(pL, -2);
		if (!lua_istable(pL, -1))
			return 0;

		lua_pushvalue(pL, 2);
		lua_rawget(pL, -2);
		// ��Ա����
		if (!lua_isfunction(pL, -1))
			return 0;
		
		// ��������������ã���������lua��ȥ���ã���Ϊ��Ҫ����set����get
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

		lua_pushstring(pL, _CLASS_PROPERTY);
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

	template<class RT, class ...Args>
	struct SNormalFunctionWrapper
	{
		typedef RT(*FUN_TYPE)(Args...);

		FUN_TYPE	pf;
	};

	template<class T, class RT, class ...Args>
	struct SClassFunctionWrapper
	{
		typedef RT(T::*FUN_TYPE)(Args...);

		FUN_TYPE	pf;
	};

	template<class RT, class ...Args>
	struct SFunctionBaseWrapper
	{
		std::function<RT(Args...)>	invoke;
	};

	struct SPushArgs
	{
		template<class T, class...NowArgs>
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

	template<int32_t ARG_COUNT, class RT, class ...Args>
	struct SParseArgs;
	
	template<class RT, class ...Args>
	struct SParseNowArg
	{
		template<class T, class ...RemainArgs, class ...NowArgs>
		static int32_t parse(lua_State* pL, int32_t& nIndex, SFunctionBaseWrapper<RT, Args...>& sFunctionBaseWrapper, NowArgs&&... args)
		{
			bool bError = false;
			T value = read2Cpp<T>(pL, nIndex++, bError);
			DebugAstEx(!bError, 0);
			
			return SParseArgs<sizeof...(RemainArgs), RT, Args...>::template parse<RemainArgs...>(pL, nIndex, sFunctionBaseWrapper, args..., value);
		}
	};

	template<int32_t ARG_COUNT, class RT, class ...Args>
	struct SParseArgs
	{
		template<class ...RemainArgs, class ...NowArgs>
		static int32_t parse(lua_State* pL, int32_t& nIndex, SFunctionBaseWrapper<RT, Args...>& sFunctionBaseWrapper, NowArgs&&... args)
		{
			return SParseNowArg<RT, Args...>::template parse<RemainArgs...>(pL, nIndex, sFunctionBaseWrapper, args...);
		}
	};

	template<class RT, class ...Args>
	struct SParseArgs<0, RT, Args...>
	{
		template<class ...NowArgs>
		static int32_t parse(lua_State* pL, int32_t& nIndex, SFunctionBaseWrapper<RT, Args...>& sFunctionBaseWrapper, NowArgs&&... args)
		{
			RT ret = sFunctionBaseWrapper.invoke(args...);
			
			push2Lua(pL, ret);
			
			return 1;
		}
	};

	template<class ...Args>
	struct SParseArgs<0, void, Args...>
	{
		template<class ...NowArgs>
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

	class CLuaFacade;
	template<class RT, class ...Args>
	int32_t __normal_invoke_proxy(lua_State* pL)
	{
		// ��lua����C++�������Ĳ�������ʱ���ڶ�ȡջ�ϵĺ���ʱ�᷵��nil����BaseScript��ᱨ��
		// ��lua����C++�������Ĳ�������ʱ��ֱ�Ӻ���
		// ��lua����C++�������Ĳ������ʹ���ʱ���ڶ�ȡջ�ϵĺ���ʱBaseScript��ᱨ��
		// ������ܵ��ö���20��������C++��������Ȼluaջֱ�ӱ���
		PROFILING_GUARD(__normal_invoke_proxy)
		lua_pushstring(pL, _FACADE_NAME);
		lua_rawget(pL, LUA_REGISTRYINDEX);
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
	
	template<class T, class RT, class ...Args>
	int32_t __class_invoke_proxy(lua_State* pL)
	{
		// ��lua����C++�������Ĳ�������ʱ���ڶ�ȡջ�ϵĺ���ʱ�᷵��nil����libBaseLua�ᱨ��
		// ��lua����C++�������Ĳ�������ʱ��ֱ�Ӻ���
		// ��lua����C++�������Ĳ������ʹ���ʱ���ڶ�ȡջ�ϵĺ���ʱlibBaseLua�ᱨ��
		// ������ܵ��ö���20��������C++��������Ȼluaջֱ�ӱ���
		PROFILING_GUARD(__class_invoke_proxy)
		lua_pushstring(pL, _FACADE_NAME);
		lua_rawget(pL, LUA_REGISTRYINDEX);
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
}