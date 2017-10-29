// TestBaseLua.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "libCoreCommon/promise.h"
#include "libCoreCommon/when_all.h"
#include <utility>

TEST(FutureTest1, FutureTest)
{
	core::CPromise<int32_t> p;
	core::CPromise<std::string> p2;
	auto f = p.getFuture();
	f.then_r([&p2](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 100);
		return p2.getFuture();
	}).then([](const std::string* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == "hello");
	});

	p.setValue(std::make_shared<int32_t>(100), 0);
	p2.setValue(std::make_shared<std::string>("hello"), 0);
}

TEST(FutureTest2, FutureTest)
{
	auto f = core::createFuture(std::make_shared<int32_t>(100));
	f.then_r([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 100);
		return core::createFuture(std::make_shared<std::string>("hello"));
	}).then([](const std::string* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == "hello");
	});
}

TEST(FutureTest3, FutureTest)
{
	core::CPromise<std::string> p;
	auto f = core::createFuture(std::make_shared<int32_t>(100));
	f.then_r([&p](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 100);
		return p.getFuture();
	}).then([](const std::string* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == "hello");
	});

	p.setValue(std::make_shared<std::string>("hello"), 0);
}

TEST(FutureTest4, FutureTest)
{
	core::CPromise<int32_t> p;
	auto f = p.getFuture();
	f.then_r([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 100);
		return core::createFuture(std::make_shared<std::string>("hello"));
	}).then_r([](const std::string* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == "hello");
		return core::createFuture(std::make_shared<int32_t>(800));
	}).then([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 800);
	});

	p.setValue(std::make_shared<int32_t>(100), 0);
}

TEST(FutureTest5, FutureTest)
{
	core::CPromise<int32_t> p;
	core::CPromise<int32_t> p2;
	auto f = p.getFuture();
	f.then_r([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 100);
		return core::createFuture(std::make_shared<std::string>("hello"));
	}).then_r([&p2](const std::string* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == "hello");
		return p2.getFuture();
	}).then([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 800);
	});

	p.setValue(std::make_shared<int32_t>(100), 0);
	p2.setValue(std::make_shared<int32_t>(800), 0);
}

TEST(FutureTest6, FutureTest)
{
	core::CPromise<int32_t> p1;
	core::CPromise<int32_t> p2;
	auto f1 = p1.getFuture();
	auto f2 = p2.getFuture();

	auto sFuture = core::whenAll(f1, f2);

	sFuture.then([](const std::tuple<core::CFuture<int32_t>, core::CFuture<int32_t>>* all, uint32_t nErrorCode)
	{
		std::tuple<core::CFuture<int32_t>, core::CFuture<int32_t>>& sAll = const_cast<std::tuple<core::CFuture<int32_t>, core::CFuture<int32_t>>&>(*all);

		const int32_t* nValue = std::get<0>(sAll).getValue();
		EXPECT_EQ(true, *nValue == 100);
		nValue = std::get<1>(sAll).getValue();
		EXPECT_EQ(true, *nValue == 200);
	});

	p1.setValue(std::make_shared<int32_t>(100), 0);
	p2.setValue(std::make_shared<int32_t>(200), 0);
}

TEST(FutureTest7, FutureTest)
{
	core::CPromise<int32_t> p1;
	core::CPromise<int32_t> p2;
	auto f1 = p1.getFuture();
	auto f2 = p2.getFuture();

	auto sFuture = core::whenAll(f1, f2);

	sFuture.then_r([](const std::tuple<core::CFuture<int32_t>, core::CFuture<int32_t>>* all, uint32_t nErrorCode)
	{
		std::tuple<core::CFuture<int32_t>, core::CFuture<int32_t>>& sAll = const_cast<std::tuple<core::CFuture<int32_t>, core::CFuture<int32_t>>&>(*all);

		const int32_t* nValue = std::get<0>(sAll).getValue();
		EXPECT_EQ(true, *nValue == 100);
		nValue = std::get<1>(sAll).getValue();
		EXPECT_EQ(true, *nValue == 200);

		return core::createFuture(std::make_shared<int32_t>(800));
	}).then([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 800);
	});

	p1.setValue(std::make_shared<int32_t>(100), 0);
	p2.setValue(std::make_shared<int32_t>(200), 0);
}

TEST(FutureTest8, FutureTest)
{
	core::CPromise<int32_t> p;
	core::CPromise<int32_t> p2;
	auto f = p.getFuture();
	f.then_r([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 100);
		return core::createFuture(std::make_shared<std::string>("hello"));
	}).then_r([&p2](const std::string* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == "hello");
		return p2.getFuture();
	}).then([](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 800);
	});

	p.setValue(std::make_shared<int32_t>(100), 0);
	p2.setValue(std::make_shared<int32_t>(800), 0);
}

TEST(FutureTest9, FutureTest)
{
	core::CPromise<int32_t> p;
	core::CPromise<std::string> p2;
	auto f1 = p.getFuture();
	p.setValue(std::make_shared<int32_t>(100), 0);
	auto f2 = f1.then_r([&p2](const int32_t* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == 100);
		return p2.getFuture();
	});
	f2.then([](const std::string* nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, *nVal == "hello");
	});
	p2.setValue(std::make_shared<std::string>("hello"), 0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}