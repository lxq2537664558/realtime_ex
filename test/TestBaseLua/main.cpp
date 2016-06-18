#include "libBaseLua/lua_facade.h"

#include <string>
#include <iostream>
using namespace std;

base::CLuaFacade* pLua = nullptr;

class CAA
{
public:
	int a;
	int b;
	char c;
	string d;

	static int s;

public:
	CAA( int32_t b)
	{
		this->a = b;
		cout << "CAA::CAA()" << endl;
	}

	~CAA()
	{
		cout << "CAA::~CAA()" << endl;
		pLua->unbindObject(this);
	}

	void fun0()
	{
		cout << "fun0" << endl;
		//return 0;
	}

	const CAA* fun1( CAA* pAA )
	{
		cout << "fun1" << endl;
		pAA->a = 100;
		return pAA;
	}

	void fun2( CAA* pAA1, CAA* pAA2 )
	{
		cout << "fun2" << endl;
		//return 0;
	}

	void fun3( int a, int b, int c ) const
	{
		cout << "fun3 " << a << " " << b << " " << c << endl;
		pLua->call("lua_fun2", 666);
		//return 0;
	}

	void fun4( int a, int b, int c, int d )
	{
		cout << "fun4" << endl;
		//return 0;
	}

	void fun5( int a, int b, int c, int d, int e )
	{
		cout << "fun5" << endl;
		//return 0;
	}

	void fun6( int a, int b, int c, int d, int e, int f )
	{
		cout << "fun6" << endl;
		//return 0;
	}

	void fun7( int a, int b, int c, int d, int e, int f, int g )
	{
		cout << "fun7" << endl;
		//return 0;
	}

	void fun8( int a, int b, int c, int d, int e, int f, int g, int h )
	{
		cout << "fun8" << endl;
		//return 0;
	}

	void fun9( int a, int b, int c, int d, int e, int f, int g, int h, int i )
	{
		cout << "fun9" << endl;
		//return 0;
	}
};

int CAA::s = 900;

void fun0()
{
	cout << "fun0" << endl;
}

void fun1( int a )
{
	cout << "fun1" << endl;
}

void fun2( int a, int b )
{
	cout << "fun2" << endl;
}

void fun3( int a, int b, int c )
{
	cout << "fun3" << endl;
}

void fun4( int a, int b, int c, int d )
{
	cout << "fun4" << endl;
}

void fun5( int a, int b, int c, int d, int e )
{
	cout << "fun5" << endl;
}

void fun6( int a, int b, int c, int d, int e, int f )
{
	cout << "fun6" << endl;
}

void fun7( int a, int b, int c, int d, int e, int f, int g )
{
	cout << "fun7" << endl;
}

void fun8( int a, int b, int c, int d, int e, int f, int g, int h )
{
	cout << "fun8" << endl;
}

void fun9( int a, int b, int c, int d, int e, int f, int g, int h, int i )
{
	cout << "fun9" << endl;
}


int64_t funXX( int8_t a, uint8_t b, int16_t c, uint16_t d, int32_t e, uint32_t f, int64_t g, uint64_t h, float i )
{
	return 1000;
}

uint64_t funXXX( int64_t a )
{
	double b = 0xffffffff;
	int32_t c = (int32_t)b;
	uint32_t d = (int32_t)b;
	return 30000;
}

void fun64( int64_t id )
{
	pLua->call( "fun", 888 );
}

void funAA(CAA* pAA)
{
	delete pAA;
}

CAA* getAA()
{
	return new CAA( 100 );
}

int main(int argc, wchar_t* argv[])
{
	base::initLog( false );
	base::initProfiling(true);
	base::initProcessExceptionHander();
	pLua = new base::CLuaFacade();
	pLua->open();
	pLua->startDebug("0.0.0.0", 12345);
	pLua->addSeachPath( "../../../test/TestBaseLua" );

	
	pLua->registerClass<CAA>("CAA", base::lua_helper::createObject<CAA, int32_t>);
	pLua->registerClassFunction("fun0", &CAA::fun0);
	pLua->registerClassFunction("fun1", &CAA::fun1);
	pLua->registerClassFunction("fun2", &CAA::fun2);
	pLua->registerClassFunction("fun3", &CAA::fun3);
	pLua->registerClassFunction("fun4", &CAA::fun4);
	pLua->registerClassFunction("fun5", &CAA::fun5);
	pLua->registerClassFunction("fun6", &CAA::fun6);
	pLua->registerClassFunction("fun7", &CAA::fun7);
	pLua->registerClassFunction("fun8", &CAA::fun8);
	pLua->registerClassFunction("fun9", &CAA::fun9);
	pLua->registerClassMember("a", &CAA::a);
	pLua->registerClassStaticMember("CAA", "s", &CAA::s);

	pLua->registerFunction("funXX", &funXX);
	pLua->registerFunction("fun64", &fun64);
	pLua->registerFunction("funXXX", &funXXX);
	pLua->registerFunction("funAA", &funAA);
	pLua->registerFunction("getAA", &getAA);
	pLua->loadFile("test");

	while (true)
	{
		pLua->updateDebug();
		uint32_t ret;
		pLua->callR("lua_fun", ret, 100);
		Sleep(1);
	}
	Sleep(~0);
	return 0;
}