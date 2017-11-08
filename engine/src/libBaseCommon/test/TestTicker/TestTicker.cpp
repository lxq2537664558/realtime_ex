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
	vecTicker.resize(5000);
	int64_t nCount = 0;
	for (int64_t i = 0; i < vecTicker.size(); ++i)
	{
		CTicker* pTicker = new CTicker();
		vecTicker[i] = pTicker;
		pTicker->setCallback([&](uint64_t nContext)
		{
			nCount++;
		});

		pTickerMgr->registerTicker(pTicker, 5, 5, i);
	}

	int64_t nBeginTime = base::time_util::getGmtTime();
	for (int64_t i = 0; i <= 10000; ++i)
	{
		pTickerMgr->update(i);
	}
	int64_t nEndTime = base::time_util::getGmtTime();

	printf("time: %I64d count: %I64d \n", (nEndTime - nBeginTime), nCount);

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
	vecTicker.resize(5000);
	int64_t nCount = 0;
	for (int64_t i = 0; i < vecTicker.size(); ++i)
	{
		CTicker* pTicker = new CTicker();
		vecTicker[i] = pTicker;
		pTicker->setCallback([&](uint64_t nContext)
		{
			nCount++;
		});

		pTickerMgr->registerTicker(pTicker, 5, 5, i);
	}

	int64_t nBeginTime = base::time_util::getGmtTime();
	for (int64_t i = 0; i <= 10000; ++i)
	{
		pTickerMgr->update(i);
	}
	int64_t nEndTime = base::time_util::getGmtTime();

	printf("time: %I64d count: %I64d \n", (nEndTime - nBeginTime), nCount);

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