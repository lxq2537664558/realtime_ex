#include "stdafx.h"
#include "base_time.h"
#include "base_function.h"

#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

static int64_t getProcessPassTime()
{
#ifdef _WIN32
	struct SFreq
	{
		LARGE_INTEGER	freq;
		int64_t			nFirstCheckTime;
		SFreq()
		{
			if (!QueryPerformanceFrequency(&freq))
			{
				fprintf(stderr, "cur machine cout support QueryPerformanceFrequency\n");
			}

			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);

			nFirstCheckTime = (counter.QuadPart * 1000) / freq.QuadPart;
		}
	};

	static struct SFreq sFreq;

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	int64_t nTime = (counter.QuadPart * 1000) / sFreq.freq.QuadPart;

	return nTime - sFreq.nFirstCheckTime;
#else
	// 在 x86-64 平台上，clock_gettime 不是系统调用，而是用vdso机制做的，就一个函数调用开销
	// 在编译链接时需加上 -lrt ;因为在librt中实现了clock_gettime函数
	// CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
	int64_t nTime = 0;
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		nTime = ((int64_t)ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
	///< 上面不做强转的话如果编译成32位程序就会出现49天问题
	return nTime;
#endif
}

// 为了保证初始化顺序，下面的时间只能通过结构体静态变量的方式获取
static int64_t& getGmtProcessStartTimePoint()
{
	struct SGmtProcessStartTimePoint
	{
		int64_t nGmtStartProcessTime;
		SGmtProcessStartTimePoint()
		{
			this->nGmtStartProcessTime = ((int64_t)time(nullptr)) * 1000 - getProcessPassTime();
		}
	};

	static SGmtProcessStartTimePoint s_sGmtProcessStartTime;
	return s_sGmtProcessStartTime.nGmtStartProcessTime;
}

namespace base
{
	int64_t getGmtTime()
	{
		return getGmtProcessStartTimePoint() + getProcessPassTime();
	}

	int32_t getZoneTime()
	{
		struct SZoneTime
		{
			int32_t nZoneTime;

			SZoneTime()
			{
				// #ifdef _WIN32
				// 				TIME_ZONE_INFORMATION tzi;
				// 				::GetTimeZoneInformation( &tzi );
				// 				this->nZoneTime = tzi.Bias*60*1000;
				// #else
				// 				// 通过gettimeofday获取的时区值，如果在系统设置时区后没有重启，取到的依然是旧的值，比较危险，并且linux手册上也说，timezone最好不要用，现在统一计算
				// 				struct timezone tzone;
				// 				gettimeofday( nullptr, &tzone );
				// 				this->nZoneTime = tzone.tz_minuteswest*60*1000;
				// #endif

#ifdef _WIN32
				_tzset();
				long nTimeZone = 0;
				_get_timezone(&nTimeZone);
				this->nZoneTime = nTimeZone * 1000;
#else
				tzset();
				this->nZoneTime = timezone * 1000;
#endif
			}
		};

		static struct SZoneTime sZoneTime;

		return sZoneTime.nZoneTime;
	}

	int64_t getLocalTime()
	{
		// GmtTime = LocalTime + ZoneTime
		return getGmtTime() - getZoneTime();
	}

	STime getLocalTimeTM(int64_t nTime)
	{
		/*
		千万别调用localtime 这个是不可重入的，里面用静态变量保存了
		如果这样的情况
		time_t time;
		tm* pTM1 = localtime( &time );
		tm* pTM2 = localtime( &time );
		这样pTM1跟pTM2是一样的，因为指向的对象一致
		*/
		if (nTime <= 0)
			nTime = getGmtTime();
		else
			nTime = local2GmtTime(nTime);

		time_t tt = (time_t)(nTime / 1000);
		struct tm ttm;
#ifdef _WIN32
		localtime_s(&ttm, &tt);
#else
		localtime_r(&tt, &ttm);
#endif
		STime sTime;
		sTime.nSec = ttm.tm_sec;
		sTime.nMin = ttm.tm_min;
		sTime.nHour = ttm.tm_hour;
		sTime.nDay = ttm.tm_mday;
		sTime.nWDay = ttm.tm_wday + 1;
		sTime.nMon = ttm.tm_mon + 1;
		sTime.nYear = (uint16_t)(1900 + ttm.tm_year);

		return sTime;
	}

	STime getGmtTimeTM(int64_t nTime)
	{
		if (nTime <= 0)
			nTime = getGmtTime();

		time_t tt = (time_t)(nTime / 1000);
		struct tm ttm;
#ifdef _WIN32
		gmtime_s(&ttm, &tt);
#else
		gmtime_r(&tt, &ttm);
#endif
		STime sTime;
		sTime.nSec = ttm.tm_sec;
		sTime.nMin = ttm.tm_min;
		sTime.nHour = ttm.tm_hour;
		sTime.nDay = ttm.tm_mday;
		sTime.nWDay = ttm.tm_wday + 1;
		sTime.nMon = ttm.tm_mon + 1;
		sTime.nYear = (uint16_t)(1900 + ttm.tm_year);

		return sTime;
	}

	int64_t getLocalTimeByTM(const STime& sTime)
	{
		struct tm ttm;
		ttm.tm_sec = sTime.nSec;
		ttm.tm_min = sTime.nMin;
		ttm.tm_hour = sTime.nHour;
		ttm.tm_mday = sTime.nDay;
		ttm.tm_mon = sTime.nMon - 1;
		ttm.tm_year = sTime.nYear - 1900;
		ttm.tm_yday = 0;
		ttm.tm_wday = 0;
		ttm.tm_isdst = 0;

		time_t tt = mktime(&ttm);
		int64_t nTime = (int64_t)tt * 1000;

		return gmt2LocalTime(nTime);
	}

	int64_t getGmtTimeByTM(const STime& sTime)
	{
		struct tm ttm;
		ttm.tm_sec = sTime.nSec;
		ttm.tm_min = sTime.nMin;
		ttm.tm_hour = sTime.nHour;
		ttm.tm_mday = sTime.nDay;
		ttm.tm_mon = sTime.nMon - 1;
		ttm.tm_year = sTime.nYear - 1900;
		ttm.tm_yday = 0;
		ttm.tm_wday = 0;
		ttm.tm_isdst = 0;

		time_t tt = mktime(&ttm);
		return ((int64_t)tt) * 1000;
	}

	int64_t gmt2LocalTime(int64_t nTime)
	{
		// LocalTime = GmtTime - ZoneTime
		return nTime - getZoneTime();
	}

	int64_t local2GmtTime(int64_t nTime)
	{
		// GmtTime = LocalTime + ZoneTime
		return nTime + getZoneTime();
	}

	size_t formatLocalTime(char* szBuf, size_t nBufSize, int64_t nTime /* = 0 */)
	{
		if (szBuf == nullptr)
			return 0;

		if (nTime <= 0)
			nTime = getGmtTime();
		else
			nTime = local2GmtTime(nTime);

		time_t tt = (time_t)(nTime / 1000);
		struct tm ttm;
#ifdef _WIN32
		localtime_s(&ttm, static_cast<time_t*>(&tt));
#else
		localtime_r(static_cast<time_t*>(&tt), &ttm);
#endif
		return base::crt::snprintf(szBuf, nBufSize, "%04d-%02d-%02d %02d:%02d:%02d",
			ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday, ttm.tm_hour, ttm.tm_min, ttm.tm_sec);
	}

	size_t formatGmtTime(char* szBuf, size_t nBufSize, int64_t nTime)
	{
		if (szBuf == nullptr)
			return 0;

		if (nTime <= 0)
			nTime = getGmtTime();

		time_t tt = (time_t)(nTime / 1000);
		struct tm ttm;
#ifdef _WIN32
		gmtime_s(&ttm, static_cast<time_t*>(&tt));
#else
		gmtime_r(static_cast<time_t*>(&tt), &ttm);
#endif
		return base::crt::snprintf(szBuf, nBufSize, "%04d-%02d-%02d %02d:%02d:%02d",
			ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday, ttm.tm_hour, ttm.tm_min, ttm.tm_sec);
	}

	int64_t getLocalSpanTime(const STime& sTime)
	{
		int64_t nEndTime = getLocalTimeByTM(sTime);
		int64_t nCurTime = getLocalTime();
		if (nEndTime > nCurTime)
			return nEndTime - nCurTime;
		return 0;
	}

	void setGmtTime(int64_t nTime)
	{
		getGmtProcessStartTimePoint() = nTime - getProcessPassTime();
	}


	CTimeElapse::CTimeElapse()
	{
		this->beginTime();
	}

	CTimeElapse::~CTimeElapse()
	{

	}

	void CTimeElapse::beginTime()
	{
		this->m_nBeginTime = getGmtTime();
	}

	int64_t CTimeElapse::getElapse() const
	{
		return getGmtTime() - this->m_nBeginTime;
	}
}