#include "libBaseLua/lua_facade.h"
#include "libBaseNetwork/network.h"

#include <iostream>
#include <list>
#include <string>
#include <functional>

base::CLuaFacade* g_pLua = nullptr;

std::function<void()> fun()
{
	std::string szBuf = "aaaaaa";

	return [=]()
	{ 
		PrintInfo( szBuf.c_str() );
	};
}

int main(int argc, wchar_t* argv[])
{
	base::initLog();
	base::initProcessExceptionHander();

	auto p = fun();
	p();

	SNetAddr sNetAddr;
	sNetAddr.nPort = 10086;
	base::crt::strncpy( sNetAddr.szIP, SNetAddr::__IP_Size, "0.0.0.0", _TRUNCATE );

	g_pLua = new base::CLuaFacade();
	g_pLua->open();

	g_pLua->startDebug( "0.0.0.0", 10086 );

	g_pLua->addSeachPath( "../../../engineTest/TestBaseLuaDebug" );
	
	g_pLua->loadFile( "test" );

	base::CLuaFunction<void> Lfun1 = g_pLua->createScriptFunction<void>( "fun" );

	while( true )
	{
		Lfun1();
		g_pLua->updateDebug();
		base::sleep(10);
	}

	Sleep( ~0 );
	return 0;
}