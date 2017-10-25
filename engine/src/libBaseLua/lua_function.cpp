#include "lua_function.h"
#include "base_lua.h"

#include "libBaseCommon/debug_helper.h"

namespace base
{
	CLuaFunction::CLuaFunction(CLuaFacade* pLuaFacade, const char* szName)
		: m_pLuaFacade(nullptr)
		, m_nRef(LUA_REFNIL)
	{
		DebugAst(pLuaFacade != nullptr && szName != nullptr);

		lua_State* pL = pLuaFacade->getState();
		SStackCheck sStackCheck(pL);
		lua_getglobal(pL, szName);
		if (lua_isfunction(pL, -1))
		{
			this->m_nRef = luaL_ref(pL, LUA_REGISTRYINDEX);
			this->m_pLuaFacade = pLuaFacade;
		}
		else
		{
			lua_pop(pL, 1);
			///< top = 2
			this->m_nRef = LUA_REFNIL;
			this->m_pLuaFacade = nullptr;
			
			///< top = 1
			PrintWarning("%s is not a lua function", szName);
		}
	}


	CLuaFunction::CLuaFunction(CLuaFacade* pLuaFacade, int32_t nRef)
		: m_pLuaFacade(pLuaFacade)
		, m_nRef(nRef)
	{

	}

	CLuaFunction::CLuaFunction()
		: m_pLuaFacade(nullptr)
		, m_nRef(LUA_REFNIL)
	{

	}

	CLuaFunction::CLuaFunction(const CLuaFunction&lhs)
		: m_pLuaFacade(nullptr)
		, m_nRef(LUA_REFNIL)
	{
		if (lhs.m_pLuaFacade != nullptr)
		{
			lua_State* pL = lhs.m_pLuaFacade->getState();

			lua_rawgeti(pL, LUA_REGISTRYINDEX, lhs.m_nRef);
			if (lua_isfunction(pL, -1))
			{
				this->m_nRef = luaL_ref(pL, LUA_REGISTRYINDEX);
				this->m_pLuaFacade = lhs.m_pLuaFacade;
			}
		}
	}

	CLuaFunction& CLuaFunction::operator = (const CLuaFunction& lhs)
	{
		if (this == &lhs)
			return *this;

		if (this->m_nRef != LUA_REFNIL && this->m_pLuaFacade != nullptr)
		{
			luaL_unref(this->m_pLuaFacade->getState(), LUA_REGISTRYINDEX, this->m_nRef);
			this->m_nRef = LUA_REFNIL;
			this->m_pLuaFacade = nullptr;
		}

		if (lhs.m_pLuaFacade != nullptr)
		{
			lua_State* pL = lhs.m_pLuaFacade->getState();

			lua_rawgeti(pL, LUA_REGISTRYINDEX, lhs.m_nRef);
			if (lua_isfunction(pL, -1))
			{
				this->m_nRef = luaL_ref(pL, LUA_REGISTRYINDEX);
				this->m_pLuaFacade = lhs.m_pLuaFacade;
			}
		}

		return *this;
	}

	CLuaFunction::CLuaFunction(CLuaFunction&& lhs)
	{
		this->m_nRef = lhs.m_nRef;
		this->m_pLuaFacade = lhs.m_pLuaFacade;

		lhs.m_nRef = LUA_REFNIL;
		lhs.m_pLuaFacade = nullptr;
	}

	CLuaFunction& CLuaFunction::operator = (CLuaFunction&& lhs)
	{
		this->m_nRef = lhs.m_nRef;
		this->m_pLuaFacade = lhs.m_pLuaFacade;

		lhs.m_nRef = LUA_REFNIL;
		lhs.m_pLuaFacade = nullptr;

		return *this;
	}

	CLuaFunction::~CLuaFunction()
	{
		if (this->m_nRef != -1 && this->m_pLuaFacade != nullptr)
			luaL_unref(this->m_pLuaFacade->getState(), LUA_REGISTRYINDEX, this->m_nRef);

		this->m_pLuaFacade = nullptr;
		this->m_nRef = LUA_REFNIL;
	}

}