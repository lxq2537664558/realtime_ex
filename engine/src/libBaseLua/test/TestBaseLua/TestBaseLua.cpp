// TestBaseLua.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <string>
#include <iostream>
#include "libBaseLua/lua_facade.h"
#include "libBaseLua/namespace_binder.h"
#include "libBaseLua/lua_function.h"
#include "libBaseLua/class_binder.h"
#include "libBaseLua/base_lua.h"

#include "gtest/gtest.h"

base::CLuaFacade* g_pLua = nullptr;

class CAA
{
public:
	uint32_t m_nA;
	
	static uint32_t s_nC;

public:
	CAA(uint32_t a)
	{
		this->m_nA = a;
		std::cout << "CAA::CAA()" << std::endl;
	}

	~CAA()
	{
		std::cout << "CAA::~CAA()" << std::endl;
	}

	void funA(uint32_t nA)
	{
		this->m_nA = nA;
	}
};

class CBB : public CAA
{
public:
	uint32_t	m_nB;
public:
	CBB(uint32_t a, uint32_t b) : CAA(a)
	{
		this->m_nB = b;
	}

	void funB(uint32_t nB)
	{
		this->m_nB = nB;
	}
};

uint32_t CAA::s_nC = 900;

CBB* getBB()
{
	return new CBB(0, 0);
}

uint32_t g_nBB = 999;

TEST(LuaFunctionTest, LuaTest)
{
	base::CLuaFunction sLuaFunction(g_pLua, "test_function1");
	uint32_t nRet = 0;
	sLuaFunction.callR(nRet, 100, 200);
	EXPECT_EQ(300, nRet);
}

TEST(ClassFunctionTest2, LuaTest)
{
	base::CLuaFunction sLuaFunction(g_pLua, "test_function2");
	CAA* nRet = nullptr;
	CAA* pAA = new CAA(100);
	sLuaFunction.callR(nRet, pAA);

	EXPECT_EQ(true, pAA->m_nA == 222);
	EXPECT_EQ(true, nRet == pAA);
}

TEST(ClassFunctionTest3, LuaTest)
{
	base::CLuaFunction sLuaFunction(g_pLua, "test_function3");
	CAA* nRet = nullptr;
	CAA* pAA = new CAA(100);
	sLuaFunction.callR(nRet, pAA);

	EXPECT_EQ(true, pAA->m_nA == 333);
	EXPECT_EQ(true, nRet == pAA);
}

TEST(ClassFunctionTest4, LuaTest)
{
	base::CLuaFunction sLuaFunction(g_pLua, "test_function4");
	CBB* nRet = nullptr;
	CBB* pBB = new CBB(100, 100);
	sLuaFunction.callR(nRet, pBB);

	EXPECT_EQ(true, pBB->m_nA == 444);
	EXPECT_EQ(true, nRet == pBB);
}

TEST(ClassFunctionTest5, LuaTest)
{
	base::CLuaFunction sLuaFunction(g_pLua, "test_function5");
	CBB* nRet = nullptr;
	CBB* pBB = new CBB(100, 100);
	sLuaFunction.callR(nRet, pBB);

	EXPECT_EQ(true, pBB->m_nB == 555);
	EXPECT_EQ(true, nRet == pBB);
}

TEST(ClassFunctionTest6, LuaTest)
{
	base::CLuaFunction sLuaFunction(g_pLua, "test_function6");
	CBB* nRet = nullptr;
	sLuaFunction.callR(nRet);

	EXPECT_EQ(true, nRet->m_nA == 666);
}

TEST(ClassFunctionTest7, LuaTest)
{
	base::CLuaFunction sLuaFunction(g_pLua, "test_function7");
	CBB* nRet = nullptr;
	sLuaFunction.callR(nRet);

	EXPECT_EQ(true, nRet->m_nB == g_nBB);
}

class LuaEnvironment : public testing::Environment
{
public:
	virtual void SetUp()
	{
		base::initLog(false, false, "");
		base::initProfiling(true);
		base::initProcessExceptionHander();
		g_pLua = new base::CLuaFacade();
		g_pLua->open();
		g_pLua->startDebug("0.0.0.0", 12345);
		
		g_pLua->addSeachPath("../lua_src");

		base::CNamespaceBinder sNamespaceBinder(g_pLua);

		sNamespaceBinder.begin("cc");
		sNamespaceBinder.registerClass<CAA>("CAA", base::CClassBinder<CAA>::createObject<uint32_t>)
			.registerFunction("funA", &CAA::funA)
			.registerMember("a", &CAA::m_nA)
			.registerStaticMember("c", &CAA::s_nC)
			.endClass();

		sNamespaceBinder.registerClass<CBB>("CBB", "cc.CAA", base::CClassBinder<CBB>::createObject<uint32_t, uint32_t>)
			.registerFunction("funB", &CBB::funB)
			.registerMember("b", &CBB::m_nB)
			.endClass();

		sNamespaceBinder.registerFunction("getBB", &getBB);
		sNamespaceBinder.registerConstData("nBB", g_nBB);

		g_pLua->loadFile("test1");
	}
	virtual void TearDown()
	{
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	testing::AddGlobalTestEnvironment(new LuaEnvironment);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}