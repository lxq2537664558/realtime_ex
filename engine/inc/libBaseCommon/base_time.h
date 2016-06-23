#pragma once

#include "base_common.h"

namespace base
{
	struct STime
	{
		uint32_t nYear;	// 年	[0, -)
		uint32_t nMon;	// 月	[1,12]
		uint32_t nDay;	// 日	[1,31]
		uint32_t nWDay;	// 周日	[0, 6](周天开始)
		uint32_t nHour;	// 时	[0,23]
		uint32_t nMin;	// 分	[0,59]
		uint32_t nSec;	// 秒	[0,59]
	};

	/**
	@brief: 获取当前的格林威治时间，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	getGmtTime();
	/**
	@brief: 获取格林威治格式化的时间
	*/
	__BASE_COMMON_API__ STime	getGmtTimeTM(int64_t nTime = 0);
	/**
	@brief: 根据格式化的本地时间获取数值时间，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	getGmtTimeByTM(const STime& sTime);
	/**
	@brief: 根据字符串格式的时间获取数值时间(字符串格式：2010-01-01 01:01:01)，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	getGmtTimeByBuf(char* szBuf);
	/**
	@brief: 获取本地时间，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	getLocalTime();
	/**
	@brief: 获取本地格式化的时间
	*/
	__BASE_COMMON_API__ STime	getLocalTimeTM(int64_t nTime = 0);
	/**
	@brief: 根据格式化的本地时间获取数值时间，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	getLocalTimeByTM(const STime& sTime);
	/**
	@brief: 根据字符串格式的时间获取数值时间(字符串格式：2010-01-01 01:01:01)，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	getLocalTimeByBuf(char* szBuf);
	/**
	@brief: 获取当前时区，单位毫秒
	*/
	__BASE_COMMON_API__ int32_t	getZoneTime();
	/**
	@brief: 格林威治时间转本地时间，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	gmt2LocalTime(int64_t nTime);
	/**
	@brief: 本地时间转格林威治时间，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	local2GmtTime(int64_t nTime);
	/**
	@brief: 格式化本地时间(字符串格式：2010-01-01 01:01:01)
	*/
	__BASE_COMMON_API__ size_t	formatLocalTime(char* szBuf, size_t nBufSize, int64_t nTime = 0);
	/**
	@brief: 格式化格林威治时间(字符串格式：2010-01-01 01:01:01)
	*/
	__BASE_COMMON_API__ size_t	formatGmtTime(char* szBuf, size_t nBufSize, int64_t nTime = 0);
	/**
	@brief: 获取当前时间到指定时间之间的时间间隔，单位毫秒
	*/
	__BASE_COMMON_API__ int64_t	getLocalSpanTime(const STime& sTime);
	/**
	@brief: 设置格林威治时间
	*/
	__BASE_COMMON_API__ void	setGmtTime(int64_t nTime);
	/**
	@brief: 获取进程启动到现在的时间（高精度，微妙）
	*/
	__BASE_COMMON_API__ int64_t	getProcessPassTime();


	/**
	@brief: 间隔时间计算器
	*/
	class __BASE_COMMON_API__ CTimeElapse
	{
	private:
		int64_t	m_nBeginTime;

	public:
		CTimeElapse();
		~CTimeElapse();

		void  beginTime();
		int64_t getElapse() const;
	};
}