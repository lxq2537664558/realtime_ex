#include "libBaseCommon/base_function.h"

#include "lua_facade.h"
#include "lua_debug.h"

#include <string>
#include <iostream>

namespace base
{
	CLuaFacade::CLuaFacade()
	{
		this->m_pMainLuaState = nullptr;
		this->m_pActiveLuaState = nullptr;
	}

	CLuaFacade::~CLuaFacade()
	{
		this->close();
	}

	static int lua_print(lua_State* pL)
	{
		int32_t n = lua_gettop(pL);  /* number of arguments */
		int32_t i;
		lua_getglobal(pL, "tostring");
		for (i = 1; i <= n; ++i)
		{
			const char *s;
			lua_pushvalue(pL, -1);  /* function to be called */
			lua_pushvalue(pL, i);   /* value to print */
			lua_call(pL, 1, 1);
			s = lua_tostring(pL, -1);  /* get result */
			if (s == nullptr)
				return luaL_error(pL, LUA_QL("tostring") " must return a string to "
				LUA_QL("print"));
			PrintInfo(s);
			//cout << s << endl;
			lua_pop(pL, 1);  /* pop result */
		}
		return 0;
	}

	int32_t traceback_count(lua_State* pL)
	{
		lua_Debug ar;
		int32_t index = 1;
		while (lua_getstack(pL, index, &ar))
		{
			index++;
		}
		lua_pushnumber(pL, index - 1);
		return 1;
	}

	void CLuaFacade::addObjectRef(void* pObject)
	{
		DebugAst(pObject != nullptr);

		int32_t tbl = lua_gettop(this->m_pMainLuaState);
		lua_getglobal(this->m_pMainLuaState, _REF_OBJECT_TBL_NAME);
		// 查看对象是否存在
		lua_pushlightuserdata(this->m_pMainLuaState, pObject);
		lua_rawget(this->m_pMainLuaState, -2);
		if (lua_isnil(this->m_pMainLuaState, -1))
		{
			lua_pop(this->m_pMainLuaState, -1);
			lua_getglobal(this->m_pMainLuaState, _WEAK_OBJECT_TBL_NAME);
			lua_pushlightuserdata(this->m_pMainLuaState, pObject);
			lua_rawget(this->m_pMainLuaState, -2);
			if (!lua_isuserdata(this->m_pMainLuaState, -1))
			{
				lua_settop(this->m_pMainLuaState, tbl);
				return;
			}

			SObjectWraper* pObjectWraper = (SObjectWraper*)lua_touserdata(this->m_pMainLuaState, -1);
			if (nullptr == pObjectWraper)
			{
				lua_settop(this->m_pMainLuaState, tbl);
				return;
			}
			if (pObjectWraper->pObject != pObject)
			{
				lua_settop(this->m_pMainLuaState, tbl);
				return;
			}
			if (!pObjectWraper->bGc)
			{
				lua_settop(this->m_pMainLuaState, tbl);
				return;
			}

			lua_getglobal(this->m_pMainLuaState, _REF_OBJECT_TBL_NAME);
			lua_pushlightuserdata(this->m_pMainLuaState, pObject);
			lua_pushvalue(this->m_pMainLuaState, -3);
			lua_rawset(this->m_pMainLuaState, -3);
			lua_pushvalue(this->m_pMainLuaState, -2);
		}
		else if (!lua_isuserdata(this->m_pMainLuaState, -1))
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}

		SObjectWraper* pObjectWraper = (SObjectWraper*)lua_touserdata(this->m_pMainLuaState, -1);
		if (nullptr == pObjectWraper)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		if (pObjectWraper->pObject != pObject)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		// 生命周期由C++控制的对象做引用计数没有意义
		if (!pObjectWraper->bGc)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}

		++pObjectWraper->nRefCount;
		lua_pushlightuserdata(this->m_pMainLuaState, pObject);
		lua_pushvalue(this->m_pMainLuaState, -2);

		lua_rawset(this->m_pMainLuaState, -3);
		lua_settop(this->m_pMainLuaState, tbl);
	}

	void CLuaFacade::delObjectRef(void* pObject)
	{
		DebugAst(pObject != nullptr);

		int32_t tbl = lua_gettop(this->m_pMainLuaState);
		lua_getglobal(this->m_pMainLuaState, _REF_OBJECT_TBL_NAME);
		// 查看对象是否存在
		lua_pushlightuserdata(this->m_pMainLuaState, pObject);
		lua_rawget(this->m_pMainLuaState, -2);
		if (!lua_isuserdata(this->m_pMainLuaState, -1))
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		SObjectWraper* pObjectWraper = (SObjectWraper*)lua_touserdata(this->m_pMainLuaState, -1);
		if (nullptr == pObjectWraper)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		if (pObjectWraper->pObject != pObject)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		// 生命周期由C++控制的对象做引用计数没有意义
		if (!pObjectWraper->bGc)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}

		--pObjectWraper->nRefCount;
		lua_pushlightuserdata(this->m_pMainLuaState, pObject);
		if (pObjectWraper->nRefCount <= 0)
			lua_pushnil(this->m_pMainLuaState);
		else
			lua_pushvalue(this->m_pMainLuaState, -2);

		lua_rawset(this->m_pMainLuaState, -4);

		lua_settop(this->m_pMainLuaState, tbl);
	}

	void CLuaFacade::unbindObject(void* pObject)
	{
		DebugAst(pObject != nullptr);

		int32_t tbl = lua_gettop(this->m_pMainLuaState);
		lua_getglobal(this->m_pMainLuaState, _WEAK_OBJECT_TBL_NAME);
		// 查看对象是否存在
		lua_pushlightuserdata(this->m_pMainLuaState, pObject);
		lua_rawget(this->m_pMainLuaState, -2);
		if (!lua_isuserdata(this->m_pMainLuaState, -1))
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		SObjectWraper* pObjectWraper = (SObjectWraper*)lua_touserdata(this->m_pMainLuaState, -1);
		if (nullptr == pObjectWraper)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		if (pObjectWraper->pObject != pObject)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}
		// 生命周期由C++控制的对象做引用计数没有意义
		if (pObjectWraper->bGc)
		{
			lua_settop(this->m_pMainLuaState, tbl);
			return;
		}

		lua_pop(this->m_pMainLuaState, 1);

		pObjectWraper->pObject = nullptr;
		lua_settop(this->m_pMainLuaState, tbl);
	}

	bool CLuaFacade::open()
	{
		DebugAstEx(this->m_pMainLuaState == nullptr, false);

		this->m_pMainLuaState = lua_open();
		DebugAstEx(this->m_pMainLuaState != nullptr, false);
		luaL_openlibs(this->m_pMainLuaState);
		luaL_Reg zFuncs[] =
		{
			{ "print", lua_print },
			{ "traceback_count", traceback_count },
			{ nullptr, nullptr }
		};
		luaL_register(this->m_pMainLuaState, "_G", zFuncs);

		lua_pushlightuserdata(this->m_pMainLuaState, this);
		lua_setglobal(this->m_pMainLuaState, _FACADE_NAME);

		//initLuaHelper( this );

		// 创建 ref_object_table
		lua_newtable(this->m_pMainLuaState);
		lua_setglobal(this->m_pMainLuaState, _REF_OBJECT_TBL_NAME);

		lua_pushliteral(this->m_pMainLuaState, "__mode");
		lua_pushstring(this->m_pMainLuaState, "k");
		lua_rawset(this->m_pMainLuaState, -3);

		// 创建 weak_object_table
		lua_newtable(this->m_pMainLuaState);
		lua_pushvalue(this->m_pMainLuaState, -1);

		lua_setglobal(this->m_pMainLuaState, _WEAK_OBJECT_TBL_NAME);

		// weak_object_table.mt = weak_object_table
		lua_pushvalue(this->m_pMainLuaState, -1);
		lua_setmetatable(this->m_pMainLuaState, -2);

		// weak_object_table.__mode = "v"
		lua_pushliteral(this->m_pMainLuaState, "__mode");
		lua_pushstring(this->m_pMainLuaState, "v");
		lua_rawset(this->m_pMainLuaState, -3);

		// pop weak_object_table
		lua_pop(this->m_pMainLuaState, 1);

		this->m_pActiveLuaState = nullptr;
		return true;
	}

	void CLuaFacade::close()
	{
		if (this->m_pMainLuaState != nullptr)
		{
			lua_close(this->m_pMainLuaState);
			this->m_pMainLuaState = nullptr;
		}
	}

	void CLuaFacade::setActiveLuaState(lua_State* pL)
	{
		this->m_pActiveLuaState = pL;
	}

	bool CLuaFacade::loadFile(const char* szName)
	{
		DebugAstEx(szName != nullptr, false);

		lua_getglobal(this->m_pMainLuaState, "package");
		lua_getfield(this->m_pMainLuaState, -1, "loaded");
		lua_pushstring(this->m_pMainLuaState, szName);
		lua_rawget(this->m_pMainLuaState, -2);
		if (!lua_isnil(this->m_pMainLuaState, -1))
		{
			lua_pushstring(this->m_pMainLuaState, szName);
			lua_pushnil(this->m_pMainLuaState);
			lua_rawset(this->m_pMainLuaState, -4);
		}
		lua_pop(this->m_pMainLuaState, 3);

		std::string szRequire = "require \'" + std::string(szName) + "\'";

		return this->runString(szRequire.c_str(), NULL);
	}

	bool CLuaFacade::runString(const char* szBuf, const char* szBufName)
	{
		DebugAstEx(szBuf != nullptr, false);

		lua_pushcclosure(this->m_pMainLuaState, lua_helper::onError, 0);
		int32_t nErrIdx = lua_gettop(this->m_pMainLuaState);
		bool bRet = false;
		if (luaL_loadbuffer(this->m_pMainLuaState, szBuf, strlen(szBuf), szBufName) == 0)
		{
			if (lua_pcall(this->m_pMainLuaState, 0, 0, nErrIdx))
			{
				lua_pop(this->m_pMainLuaState, 1);
			}
			else
			{
				bRet = true;
			}
		}
		else
		{
			PrintWarning("luaL_loadbuffer error: %s", lua_tostring(this->m_pMainLuaState, -1));
			lua_pop(this->m_pMainLuaState, 1);
		}

		lua_pop(this->m_pMainLuaState, 1);

		return bRet;
	}

	void CLuaFacade::addSeachPath(const char* szPath)
	{
		DebugAst(szPath != nullptr);

		lua_getglobal(this->m_pMainLuaState, "package");
		lua_getfield(this->m_pMainLuaState, -1, "path");
		const char* szCurPath = lua_tostring(this->m_pMainLuaState, -1);
		lua_pushfstring(this->m_pMainLuaState, "%s;%s/?.lua", szCurPath, szPath);
		lua_setfield(this->m_pMainLuaState, -3, "path");
		lua_pop(this->m_pMainLuaState, 2);
	}

	bool CLuaFacade::startDebug(const char* szIP, uint16_t nPort)
	{
		return startLuaDebug(this->m_pMainLuaState, szIP, nPort);
	}

	void CLuaFacade::updateDebug()
	{
		updateLuaDebug(this->m_pMainLuaState);
	}

	lua_State* CLuaFacade::getActiveLuaState() const
	{
		if (this->m_pActiveLuaState == nullptr)
			return this->m_pMainLuaState;

		return this->m_pActiveLuaState;
	}

	namespace lua_helper
	{
		int32_t onError(lua_State* pL)
		{
			PrintWarning("lua error : %s", lua_tostring(pL, -1));

			printStack(pL, 0);

			return 0;
		}
	}
}