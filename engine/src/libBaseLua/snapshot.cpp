#ifndef _WIN32
#include <stdbool.h>
#endif

#include <stdio.h>
#include <string.h>

#include "lua_facade.h"

#include "libBaseCommon/function_util.h"

namespace base
{
	void mark_object(lua_State* pL, lua_State* pNewL, const void* parent, const char* desc);

	void mark_function_env(lua_State* pL, lua_State* pNewL, const void* t)
	{
		//lua_getfenv(pL, -1);
		if (lua_istable(pL, -1))
			mark_object(pL, pNewL, t, "[environment]");
		else
			lua_pop(pL, 1);
	}

#define TABLE 1
#define FUNCTION 2
#define SOURCE 3
#define THREAD 4
#define USERDATA 5
#define MARK 6

	bool ismarked(lua_State* pNewL, const void *p)
	{
		lua_rawgetp(pNewL, MARK, p);
		if (lua_isnil(pNewL, -1))
		{
			lua_pop(pNewL, 1);
			lua_pushboolean(pNewL, 1);
			lua_rawsetp(pNewL, MARK, p);
			return false;
		}
		lua_pop(pNewL, 1);
		return true;
	}

	const void* readobject(lua_State* pL, lua_State* pNewL, const void* parent, const char* desc)
	{
		int32_t t = lua_type(pL, -1);
		int32_t tidx = 0;
		switch (t)
		{
		case LUA_TTABLE:
			tidx = TABLE;
			break;
		case LUA_TFUNCTION:
			tidx = FUNCTION;
			break;
		case LUA_TTHREAD:
			tidx = THREAD;
			break;
		case LUA_TUSERDATA:
			tidx = USERDATA;
			break;
		default:
			return nullptr;
		}

		const void *p = lua_topointer(pL, -1);
		if (ismarked(pNewL, p))
		{
			lua_rawgetp(pNewL, tidx, p);
			if (!lua_isnil(pNewL, -1))
			{
				lua_pushstring(pNewL, desc);
				lua_rawsetp(pNewL, -2, parent);
			}
			lua_pop(pNewL, 1);
			lua_pop(pL, 1);
			return nullptr;
		}

		lua_newtable(pNewL);
		lua_pushstring(pNewL, desc);
		lua_rawsetp(pNewL, -2, parent);
		lua_rawsetp(pNewL, tidx, p);

		return p;
	}

	const char* keystring(lua_State* pL, int32_t index, char* buffer, size_t size)
	{
		int t = lua_type(pL, index);
		switch (t)
		{
		case LUA_TSTRING:
			return lua_tostring(pL, index);
		case LUA_TNUMBER:
			base::function_util::snprintf(buffer, size, "[%lg]", lua_tonumber(pL, index));
			break;
		case LUA_TBOOLEAN:
			base::function_util::snprintf(buffer, size, "[%s]", lua_toboolean(pL, index) ? "true" : "false");
			break;
		case LUA_TNIL:
			base::function_util::snprintf(buffer, size, "[nil]");
			break;
		default:
			base::function_util::snprintf(buffer, size, "[%s:%p]", lua_typename(pL, t), lua_topointer(pL, index));
			break;
		}
		return buffer;
	}

	void mark_table(lua_State* pL, lua_State* pNewL, const void* parent, const char* desc)
	{
		const void *t = readobject(pL, pNewL, parent, desc);
		if (t == nullptr)
			return;

		bool weakk = false;
		bool weakv = false;
		if (lua_getmetatable(pL, -1))
		{
			lua_pushliteral(pL, "__mode");
			lua_rawget(pL, -2);
			if (lua_isstring(pL, -1))
			{
				const char* mode = lua_tostring(pL, -1);
				if (strchr(mode, 'k'))
				{
					weakk = true;
				}
				if (strchr(mode, 'v'))
				{
					weakv = true;
				}
			}
			lua_pop(pL, 1);

			mark_table(pL, pNewL, t, "[metatable]");
		}

		lua_pushnil(pL);
		while (lua_next(pL, -2) != 0)
		{
			if (weakv)
			{
				lua_pop(pL, 1);
			}
			else
			{
				char temp[32];
				const char * desc = keystring(pL, -2, temp, _countof(temp));
				mark_object(pL, pNewL, t, desc);
			}
			if (!weakk)
			{
				lua_pushvalue(pL, -1);
				mark_object(pL, pNewL, t, "[key]");
			}
		}

		lua_pop(pL, 1);
	}

	void mark_userdata(lua_State* pL, lua_State* pNewL, const void* parent, const char* desc)
	{
		const void* t = readobject(pL, pNewL, parent, desc);
		if (t == nullptr)
			return;

		if (lua_getmetatable(pL, -1))
		{
			mark_table(pL, pNewL, t, "[metatable]");
		}

		lua_getuservalue(pL, -1);
		if (lua_isnil(pL, -1))
		{
			lua_pop(pL, 2);
		}
		else
		{
			mark_table(pL, pNewL, t, "[uservalue]");
			lua_pop(pL, 1);
		}
	}

	void mark_function(lua_State* pL, lua_State* pNewL, const void* parent, const char* desc)
	{
		const void* t = readobject(pL, pNewL, parent, desc);
		if (t == nullptr)
			return;

		mark_function_env(pL, pNewL, t);

		int32_t i;
		for (i = 1;; ++i)
		{
			const char* name = lua_getupvalue(pL, -1, i);
			if (name == nullptr)
				break;
			mark_object(pL, pNewL, t, name[0] ? name : "[upvalue]");
		}

		if (lua_iscfunction(pL, -1))
		{
			if (i == 1)
			{
				// light c function
				lua_pushnil(pNewL);
				lua_rawsetp(pNewL, FUNCTION, t);
			}
			lua_pop(pL, 1);
		}
		else
		{
			lua_Debug ar;
			lua_getinfo(pL, ">S", &ar);
			luaL_Buffer b;
			luaL_buffinit(pNewL, &b);
			luaL_addstring(&b, ar.short_src);
			char tmp[16];
			base::function_util::snprintf(tmp, _countof(tmp), ":%d", ar.linedefined);
			luaL_addstring(&b, tmp);
			luaL_pushresult(&b);
			lua_rawsetp(pNewL, SOURCE, t);
		}
	}

	void mark_thread(lua_State* pL, lua_State* pNewL, const void* parent, const char* desc)
	{
		const void* t = readobject(pL, pNewL, parent, desc);
		if (t == nullptr)
			return;
		int32_t level = 0;
		lua_State* pcL = lua_tothread(pL, -1);
		if (pcL == pL)
		{
			level = 1;
		}
		lua_Debug ar;
		luaL_Buffer b;
		luaL_buffinit(pNewL, &b);
		while (lua_getstack(pcL, level, &ar))
		{
			char tmp[128];
			lua_getinfo(pcL, "Sl", &ar);
			luaL_addstring(&b, ar.short_src);
			if (ar.currentline >= 0)
			{
				char tmp[16];
				base::function_util::snprintf(tmp, _countof(tmp), ":%d ", ar.currentline);
				luaL_addstring(&b, tmp);
			}

			for (int32_t j = 1; j > -1; j -= 2)
			{
				for (int32_t i = j;; i += j)
				{
					const char* name = lua_getlocal(pcL, &ar, i);
					if (name == nullptr)
						break;
					base::function_util::snprintf(tmp, sizeof(tmp), "%s : %s:%d", name, ar.short_src, ar.currentline);
					mark_object(pcL, pNewL, t, tmp);
				}
			}
			++level;
		}
		luaL_pushresult(&b);
		lua_rawsetp(pNewL, SOURCE, t);
		lua_pop(pL, 1);
	}

	void mark_object(lua_State* pL, lua_State* pNewL, const void* parent, const char* desc)
	{
		int32_t t = lua_type(pL, -1);
		switch (t)
		{
		case LUA_TTABLE:
			mark_table(pL, pNewL, parent, desc);
			break;
		case LUA_TUSERDATA:
			mark_userdata(pL, pNewL, parent, desc);
			break;
		case LUA_TFUNCTION:
			mark_function(pL, pNewL, parent, desc);
			break;
		case LUA_TTHREAD:
			mark_thread(pL, pNewL, parent, desc);
			break;
		default:
			lua_pop(pL, 1);
			break;
		}
	}

	int32_t count_table(lua_State* pL, int32_t index)
	{
		int32_t n = 0;
		lua_pushnil(pL);
		while (lua_next(pL, index) != 0)
		{
			++n;
			lua_pop(pL, 1);
		}
		return n;
	}

	void gen_table_desc(lua_State* pNewL, luaL_Buffer* b, const void* parent, const char* desc)
	{
		char tmp[32];
		size_t l = base::function_util::snprintf(tmp, _countof(tmp), "%p : ", parent);
		luaL_addlstring(b, tmp, l);
		luaL_addstring(b, desc);
		luaL_addchar(b, '\n');
	}

	void pdesc(lua_State* pL, lua_State* pNewL, int32_t index, const char* name)
	{
		lua_pushnil(pNewL);
		while (lua_next(pNewL, index) != 0)
		{
			luaL_Buffer b;
			luaL_buffinit(pL, &b);
			const void* key = lua_touserdata(pNewL, -2);
			if (index == FUNCTION)
			{
				lua_rawgetp(pNewL, SOURCE, key);
				if (lua_isnil(pNewL, -1))
				{
					luaL_addstring(&b, "cfunction\n");
				}
				else
				{
					size_t l = 0;
					const char* s = lua_tolstring(pNewL, -1, &l);
					luaL_addlstring(&b, s, l);
					luaL_addchar(&b, '\n');
				}
				lua_pop(pNewL, 1);
			}
			else if (index == THREAD)
			{
				lua_rawgetp(pNewL, SOURCE, key);
				size_t l = 0;
				const char* s = lua_tolstring(pNewL, -1, &l);
				luaL_addlstring(&b, s, l);
				luaL_addchar(&b, '\n');
				lua_pop(pNewL, 1);
			}
			else
			{
				luaL_addstring(&b, name);
				luaL_addchar(&b, '\n');
			}
			lua_pushnil(pNewL);
			while (lua_next(pNewL, -2) != 0)
			{
				const void* parent = lua_touserdata(pNewL, -2);
				const char* desc = luaL_checkstring(pNewL, -1);
				gen_table_desc(pNewL, &b, parent, desc);
				lua_pop(pNewL, 1);
			}
			luaL_pushresult(&b);
			lua_rawsetp(pL, -2, key);
			lua_pop(pNewL, 1);
		}
	}

	void gen_result(lua_State* pL, lua_State* pNewL)
	{
		int32_t count = 0;
		count += count_table(pNewL, TABLE);
		count += count_table(pNewL, FUNCTION);
		count += count_table(pNewL, USERDATA);
		count += count_table(pNewL, THREAD);
		lua_createtable(pL, 0, count);
		pdesc(pL, pNewL, TABLE, "table");
		pdesc(pL, pNewL, USERDATA, "userdata");
		pdesc(pL, pNewL, FUNCTION, "function");
		pdesc(pL, pNewL, THREAD, "thread");
	}

	int32_t snapshot(lua_State* pL)
	{
		lua_State* pNewL = luaL_newstate();
		for (int32_t i = 0; i < MARK; ++i)
		{
			lua_newtable(pNewL);
		}
		lua_pushvalue(pL, LUA_REGISTRYINDEX);
		mark_table(pL, pNewL, nullptr, "[registry]");
		gen_result(pL, pNewL);
		lua_close(pNewL);
		return 1;
	}
}