// TestBaseLua.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include <utility>
#include "libBaseCommon/ticker.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"

using namespace base;

// 测试时间推进正确性
TEST(TickFunctionTest1, TickTest)
{
	std::vector<CTicker*> vecTicker;
	int64_t nBeginTime = 1509680061793;
	int64_t nRunTime = 10000;
	int64_t nCount = 0;
	CTickerMgr* pTickerMgr = new CTickerMgr(nBeginTime, nullptr);
	vecTicker.reserve(nRunTime);
	for (int64_t i = 0; i < nRunTime; ++i)
	{
		CTicker* pTicker = new CTicker();
		vecTicker.push_back(pTicker);
		pTicker->setCallback([&](uint64_t nContex) 
		{
			EXPECT_EQ(nCount, nContex);

			nCount++;
		});

		pTickerMgr->registerTicker(pTicker, i, 0, i);
	}

	for (int64_t i = nBeginTime; i <= nBeginTime + nRunTime; ++i)
	{
		pTickerMgr->update(i);
	}

	EXPECT_EQ(nCount, nRunTime);

	for (int64_t i = 0; i < nRunTime; ++i)
	{
		SAFE_DELETE(vecTicker[i]);
	}
	SAFE_DELETE(pTickerMgr);
}

// 注册反注册测试
TEST(TickFunctionTest2, TickTest)
{
	std::vector<CTicker*> vecTicker;
	
	CTickerMgr* pTickerMgr = new CTickerMgr(0, nullptr);
	vecTicker.resize(100);
	int64_t nCount = 0;
	for (int64_t i = 0; i < vecTicker.size(); ++i)
	{
		CTicker* pTicker = new CTicker();
		vecTicker[i] = pTicker;
		pTicker->setCallback([&](uint64_t nContext)
		{
			if (nContext == 20)
			{
				pTickerMgr->unregisterTicker(vecTicker[19]);
				pTickerMgr->unregisterTicker(vecTicker[21]);
			}
			else if (nContext == 30)
			{
				pTickerMgr->unregisterTicker(vecTicker[30]);
			}

			nCount++;
		});

		pTickerMgr->registerTicker(pTicker, 100, 100, i);
	}

	for (int64_t i = 0; i <= 200; ++i)
	{
		pTickerMgr->update(i);
	}

	EXPECT_EQ(nCount, 196);

	for (int64_t i = 0; i < vecTicker.size(); ++i)
	{
		SAFE_DELETE(vecTicker[i]);
	}
	SAFE_DELETE(pTickerMgr);
}

TEST(TickFunctionTest3, TickTest)
{
	std::vector<CTicker*> vecTicker;

	CTickerMgr* pTickerMgr = new CTickerMgr(0, nullptr);
	vecTicker.resize(2);
	int64_t nCount = 0;
	
	CTicker* pTicker0 = new CTicker();
	vecTicker[0] = pTicker0;
	pTicker0->setCallback([&](uint64_t nContext)
	{
		nCount++;
	});

	pTickerMgr->registerTicker(pTicker0, 5, 5, 0);

	CTicker* pTicker1 = new CTicker();
	vecTicker[1] = pTicker1;
	*pTicker1 = std::move(*pTicker0);

	for (int64_t i = 0; i <= 20; ++i)
	{
		pTickerMgr->update(i);
	}
	
	EXPECT_EQ(nCount, 4);
	
	for (int64_t i = 0; i < vecTicker.size(); ++i)
	{
		SAFE_DELETE(vecTicker[i]);
	}
	SAFE_DELETE(pTickerMgr);
}

TEST(TickFunctionTest4, TickTest)
{
	std::vector<CTicker*> vecTicker;

	CTickerMgr* pTickerMgr = new CTickerMgr(0, nullptr);
	vecTicker.resize(2);
	int64_t nCount = 0;

	CTicker* pTicker0 = new CTicker();
	vecTicker[0] = pTicker0;
	pTicker0->setCallback([&](uint64_t nContext)
	{
		nCount++;
	});

	pTickerMgr->registerTicker(pTicker0, 5, 5, 0);

	CTicker* pTicker1 = new CTicker();
	vecTicker[1] = pTicker1;
	pTicker1->setCallback([&](uint64_t nContext)
	{
		nCount++;
	});

	pTickerMgr->registerTicker(pTicker1, 10, 10, 0);
	*pTicker1 = std::move(*pTicker0);

	for (int64_t i = 0; i <= 20; ++i)
	{
		pTickerMgr->update(i);
	}

	EXPECT_EQ(nCount, 4);

	for (int64_t i = 0; i < vecTicker.size(); ++i)
	{
		SAFE_DELETE(vecTicker[i]);
	}
	SAFE_DELETE(pTickerMgr);
}

TEST(TickFunctionTest5, TickTest)
{
	std::vector<CTicker*> vecTicker;

	CTickerMgr* pTickerMgr = new CTickerMgr(0, nullptr);
	vecTicker.resize(2);
	int64_t nCount = 0;

	CTicker* pTicker0 = new CTicker();
	vecTicker[0] = pTicker0;
	pTicker0->setCallback([&](uint64_t nContext)
	{
		nCount++;
		if (nCount == 1)
			*vecTicker[1] = std::move(*pTicker0);
	});

	pTickerMgr->registerTicker(pTicker0, 5, 5, 0);

	CTicker* pTicker1 = new CTicker();
	vecTicker[1] = pTicker1;
	pTicker1->setCallback([&](uint64_t nContext)
	{
		nCount++;
	});

	pTickerMgr->registerTicker(pTicker1, 10, 10, 0);

	for (int64_t i = 0; i <= 20; ++i)
	{
		pTickerMgr->update(i);
	}

	EXPECT_EQ(nCount, 4);

	for (int64_t i = 0; i < vecTicker.size(); ++i)
	{
		SAFE_DELETE(vecTicker[i]);
	}
	SAFE_DELETE(pTickerMgr);
}

class TickEnvironment : public testing::Environment
{
public:
	virtual void SetUp()
	{
		
	}
	virtual void TearDown()
	{
	}
};

int32_t main(int32_t argc, char* argv[])
{
	testing::AddGlobalTestEnvironment(new TickEnvironment);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}