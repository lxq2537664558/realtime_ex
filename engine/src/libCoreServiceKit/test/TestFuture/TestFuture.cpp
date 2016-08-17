// TestBaseLua.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "libCoreServiceKit/promise.h"

TEST(FutureTest1, FutureTest)
{
	core::CPromise<int32_t> p;
	core::CPromise<std::string> p2;
	auto f = p.getFuture();
	f.then_r([&p2](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 100);
		return p2.getFuture();
	}).then([](std::string nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == "hello");
	});

	p.setValue(100);
	p2.setValue("hello");
}

TEST(FutureTest2, FutureTest)
{
	auto f = core::CFuture<int32_t>::createFuture(100);
	f.then_r([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 100);
		return core::CFuture<std::string>::createFuture("hello");
	}).then([](std::string nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == "hello");
	});
}

TEST(FutureTest3, FutureTest)
{
	core::CPromise<std::string> p;
	auto f = core::CFuture<int32_t>::createFuture(100);
	f.then_r([&p](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 100);
		return p.getFuture();
	}).then([](std::string nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == "hello");
	});

	p.setValue("hello");
}

TEST(FutureTest4, FutureTest)
{
	core::CPromise<int32_t> p;
	auto f = p.getFuture();
	f.then_r([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 100);
		return core::CFuture<std::string>::createFuture("hello");
	}).then_r([](std::string nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == "hello");
		return core::CFuture<int32_t>::createFuture(800);
	}).then([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 800);
	});

	p.setValue(100);
}

TEST(FutureTest5, FutureTest)
{
	core::CPromise<int32_t> p;
	core::CPromise<int32_t> p2;
	auto f = p.getFuture();
	f.then_r([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 100);
		return core::CFuture<std::string>::createFuture("hello");
	}).then_r([&p2](std::string nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == "hello");
		return p2.getFuture();
	}).then([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 800);
	});

	p.setValue(100);
	p2.setValue(800);
}

TEST(FutureTest6, FutureTest)
{
	core::CPromise<int32_t> p1;
	core::CPromise<int32_t> p2;
	auto f1 = p1.getFuture();
	auto f2 = p2.getFuture();
	std::vector<core::CFuture<int32_t>> vecFuture;
	vecFuture.push_back(f1);
	vecFuture.push_back(f2);

	core::CFuture<int32_t>::collect(vecFuture, [](const std::vector<core::CFuture<int32_t>>& vecFuture)
	{
		int32_t nValue = 0;
		EXPECT_EQ(true, vecFuture[0].getValue(nValue));
		EXPECT_EQ(true, nValue == 100);
		EXPECT_EQ(true, vecFuture[1].getValue(nValue));
		EXPECT_EQ(true, nValue == 200);
	});
	p1.setValue(100);
	p2.setValue(200);
}

TEST(FutureTest7, FutureTest)
{
	core::CPromise<int32_t> p1;
	core::CPromise<int32_t> p2;
	auto f1 = p1.getFuture();
	auto f2 = p2.getFuture();
	std::vector<core::CFuture<int32_t>> vecFuture;
	vecFuture.push_back(f1);
	vecFuture.push_back(f2);

	core::CFuture<int32_t>::collect_r(vecFuture, [](const std::vector<core::CFuture<int32_t>>& vecFuture)
	{
		int32_t nValue = 0;
		EXPECT_EQ(true, vecFuture[0].getValue(nValue));
		EXPECT_EQ(true, nValue == 100);
		EXPECT_EQ(true, vecFuture[1].getValue(nValue));
		EXPECT_EQ(true, nValue == 200);

		return core::CFuture<int32_t>::createFuture(800);
	}).then([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 800);
	});
	p1.setValue(100);
	p2.setValue(200);
}

TEST(FutureTest8, FutureTest)
{
	core::CPromise<int32_t> p;
	core::CPromise<int32_t> p2;
	auto f = p.getFuture();
	f.then_r([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 100);
		return core::CFuture<std::string>::createFuture("hello");
	}).then_r([&p2](std::string nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == "hello");
		return core::CFuture<int32_t>();
	}).then([](int32_t nVal, uint32_t nErrorCode)
	{
		EXPECT_EQ(true, nVal == 800);
	});

	p.setValue(100);
	p2.setValue(800);
}

int _tmain(int argc, _TCHAR* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}