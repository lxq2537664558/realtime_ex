#include "namespace_binder.h"
#include "lua_facade.h"

#include "libBaseCommon/token_parser.h"
#include "libBaseCommon/debug_helper.h"

namespace base
{
	CNamespaceBinder::CNamespaceBinder(CLuaFacade* pLuaFacade)
		: m_pLuaFacade(pLuaFacade)
		, m_nStackTop(-1)
		, m_szNamespaceName(nullptr)
	{
	}

	CNamespaceBinder::~CNamespaceBinder()
	{
		SAFE_DELETE_ARRAY(this->m_szNamespaceName);
		this->m_pLuaFacade = nullptr;
	}

	void CNamespaceBinder::begin(const char* szName)
	{
		if (szName == nullptr)
			throw std::logic_error("invalid namespace name arg");

		CTokenParser token;
		if (!token.parse(szName, "."))
			throw std::logic_error("invalid namespace name");

		lua_State* pL = this->m_pLuaFacade->getState();

		this->m_nStackTop = lua_gettop(pL);
		lua_pushinteger(pL, LUA_RIDX_GLOBALS);
		lua_rawget(pL, LUA_REGISTRYINDEX);
		char szBuf[256] = { 0 };
		for (size_t i = 0; i < token.getElementCount(); ++i)
		{
			token.getStringElement(i, szBuf, _countof(szBuf));

			lua_pushstring(pL, szBuf);
			lua_rawget(pL, -2);

			if (lua_isnil(pL, -1))
			{
				lua_pop(pL, 1);
				lua_newtable(pL);
				lua_pushstring(pL, szBuf);
				lua_pushvalue(pL, -2);
				lua_rawset(pL, -4);
			}
			else if (!lua_istable(pL, -1))
			{
				throw std::logic_error("namespace table error");
			}
		}

		SAFE_DELETE_ARRAY(this->m_szNamespaceName);
		size_t nLen = base::function_util::strnlen(szName, _TRUNCATE);
		this->m_szNamespaceName = new char[nLen + 1];
		base::function_util::strcpy(this->m_szNamespaceName, nLen + 1, szName);
	}

	void CNamespaceBinder::end()
	{
		if (this->m_nStackTop < 0)
			throw std::logic_error("invalid namespace stack");

		lua_State* pL = this->m_pLuaFacade->getState();

		lua_settop(pL, this->m_nStackTop);
		this->m_nStackTop = -1;
	}

	CLuaFacade* CNamespaceBinder::getLuaFacade() const
	{
		return this->m_pLuaFacade;
	}

}