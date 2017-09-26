// TestBaseLua.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <string>
#include <iostream>

#include "gtest/gtest.h"

#include "libBaseCommon/logger.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/exception_handler.h"
#include "libBaseCommon/time_util.h"
#include <thread>

TEST(CommonTest, TimeTest)
{
	int64_t nTime = time(nullptr);

	int64_t nGmtTime = base::time_util::getGmtTime();
	EXPECT_EQ(true, (nGmtTime - nTime * 1000) < 1000);

	base::time_util::STime sGmtTime = base::time_util::getGmtTimeTM(nGmtTime);
	int64_t nGmtTime1 = base::time_util::getGmtTimeByTM(sGmtTime);
	EXPECT_EQ(true, nGmtTime1 == nGmtTime);

	sGmtTime.nYear = 2017;
	sGmtTime.nMon = 9;
	sGmtTime.nDay = 18;
	sGmtTime.nHour = 10;
	sGmtTime.nMin = 18;
	sGmtTime.nSec = 18;
	sGmtTime.nMSec = 0;
	nGmtTime = base::time_util::getGmtTimeByTM(sGmtTime);
	nGmtTime1 = base::time_util::getGmtTimeByBuf("2017-09-18 10:18:18");
	EXPECT_EQ(true, nGmtTime1 == nGmtTime);

	int32_t nTimeZone = base::time_util::getZoneTime();
	int64_t nLocalTime = base::time_util::getLocalTime();
	nTime = time(nullptr);
	EXPECT_EQ(true, (nLocalTime - (nTime - nTimeZone) * 1000) < 1000);

	base::time_util::STime sLocalTime = base::time_util::getLocalTimeTM(nLocalTime);
	int64_t nLocalTime1 = base::time_util::getLocalTimeByTM(sLocalTime);
	EXPECT_EQ(true, nLocalTime1 == nLocalTime);

	sLocalTime.nYear = 2017;
	sLocalTime.nMon = 9;
	sLocalTime.nDay = 18;
	sLocalTime.nHour = 10;
	sLocalTime.nMin = 18;
	sLocalTime.nSec = 18;
	sLocalTime.nMSec = 0;
	nLocalTime = base::time_util::getLocalTimeByTM(sLocalTime);
	nLocalTime1 = base::time_util::getLocalTimeByBuf("2017-09-18 10:18:18");
	EXPECT_EQ(true, nLocalTime1 == nLocalTime);

	sGmtTime.nYear = 2017;
	sGmtTime.nMon = 9;
	sGmtTime.nDay = 18;
	sGmtTime.nHour = 10;
	sGmtTime.nMin = 18;
	sGmtTime.nSec = 18;
	sGmtTime.nMSec = 0;
	nGmtTime = base::time_util::getGmtTimeByTM(sGmtTime);
	base::time_util::setGmtTime(nGmtTime);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	int64_t nGmtTime2 = base::time_util::getGmtTime();
	EXPECT_EQ(true, nGmtTime2 > nGmtTime);
}

class CommonEnvironment : public testing::Environment
{
public:
	virtual void SetUp()
	{
		base::log::init(false, false, "");
		base::profiling::init(true);
		base::initProcessExceptionHander();
		
	}

	virtual void TearDown()
	{
	}
};

int32_t main(int32_t argc, char* argv[])
{
	testing::AddGlobalTestEnvironment(new CommonEnvironment);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}