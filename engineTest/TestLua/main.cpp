extern "C"
{
#include "lua-5.1.5/lua.h"
#include "lua-5.1.5/lualib.h"
#include "lua-5.1.5/lauxlib.h"
}

int luv_dumpstack(lua_State* pL)
{
    int i, top;
    printf("-------------------\n");
    printf("dumpstack: L=%p\n", pL);
    top = lua_gettop(pL);
    printf("top: %d\n", top);
    for (i = 1; i <= top; ++i)
	{
        printf("[%d][%s]: %s\n", i, luaL_typename(pL, i), lua_tolstring(pL, i, NULL));
        //lua_pop(pL, 1);
    }
    printf("-------------------\n");
    return top;
}

static int test_yield(lua_State* pL)
{
    printf("stack before yield\n");
    luv_dumpstack(pL);
    lua_pushinteger(pL, 1);
    lua_pushinteger(pL, 2);
    lua_pushinteger(pL, 3);
    lua_pushinteger(pL, 4);
    lua_pushinteger(pL, 5);
    return lua_yield(pL, 2);
}

static int test_resume(lua_State* pL)
{
    lua_State* pNewL = lua_newthread(pL);
    lua_pushinteger(pNewL, 11);
    lua_pushinteger(pNewL, 12);
    lua_pushcfunction(pNewL, test_yield);
    lua_pushinteger(pNewL, 13);
    lua_pushinteger(pNewL, 14);
    lua_pushinteger(pNewL, 15);
    printf("stack before resume\n");
    luv_dumpstack(pNewL);
    printf("resume: %d\n", lua_resume(pNewL, 3));
    printf("stack after resume\n");
    luv_dumpstack(pNewL);
    lua_pushinteger(pNewL, 24);
    lua_pushinteger(pNewL, 25);
    printf("stack before second resume\n");
    luv_dumpstack(pNewL);
    /* XXX notice even we pass 2, all values in stack (4,5,24,25)
     * will passed to coroutine */
	
    printf("resume: %d\n", lua_resume(pNewL, 2));
    printf("stack after second resume\n");
    luv_dumpstack(pNewL);
    return 0;
}

static int test_fun1(lua_State* pL)
{
	return 1;
}

static int test_fun(lua_State* pL)
{
	lua_pushinteger(pL, 1);
	lua_pushcfunction(pL, test_fun1);
	lua_pushinteger(pL, 2);
	lua_call(pL, 1, 1);
	lua_pushinteger(pL, 3);
	lua_pushinteger(pL, 4);
	lua_pushinteger(pL, 5);
	printf("stack before back\n");
	luv_dumpstack(pL);
	return 1;
}

#include <direct.h>

int main(void)
{
	int a = 0xf;
	a &= ~(1<<0);
	int gg = int();
	char szBuf[256] = { 0 };
	getcwd(szBuf, 256);
    lua_State* pL = luaL_newstate();
	luaL_openlibs(pL);
	luaL_dofile(pL, "test.lua");
	void* pData = lua_newuserdata(pL, 100);
	while(true)
		lua_gc(pL, LUA_GCCOLLECT, 0);
    lua_pushcfunction(pL, test_resume);
	lua_pushinteger(pL, 1);
    lua_call(pL, 1, 1);
	luv_dumpstack(pL);
	lua_close(pL);
    return 0;
}