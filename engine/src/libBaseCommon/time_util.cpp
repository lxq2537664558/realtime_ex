#include "stdafx.h"
#include "time_util.h"
#include "function_util.h"

#include <time.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// 为了保证初始化顺序，下面的时间只能通过结构体静态变量的方式获取
namespace
{
	int64_t& getGmtProcessStartTimePoint()
	{
		struct SGmtProcessStartTimePoint
		{
			int64_t nGmtStartProcessTime;
			SGmtProcessStartTimePoint()
			{
				this->nGmtStartProcessTime = ((int64_t)time(nullptr)) * 1000 - base::time_util::getProcessPassTime() / 1000;
			}
		};

		static SGmtProcessStartTimePoint s_sGmtProcessStartTime;
		return s_sGmtProcessStartTime.nGmtStartProcessTime;
	}

	bool getTMByBuf(const char* szBuf, base::time_util::STime& sTime)
	{
		if (szBuf == nullptr)
			return false;

		std::string szTime = szBuf;
		if (std::string::npos != szTime.find_first_not_of("0123456789-: "))
			return false;

		uint32_t nCount = 0;
		std::string::size_type nPos = std::string::npos;
		while (std::string::npos != (nPos = szTime.find_first_of("-: ")))
		{
			++nCount;
			if (nCount >= 1 && nCount <= 2)
			{
				if ('-' != szTime[nPos])
					return false;
			}
			else if (3 == nCount)
			{
				if (' ' != szTime[nPos])
					return false;
			}
			else if (nCount >= 4)
			{
				if (':' != szTime[nPos])
					return false;
			}
			szTime.erase(0, nPos + 1);
		}

		if (nCount != 5)
			return false;

#ifdef _WIN32
		if (-1 == sscanf_s(szBuf, "%d-%d-%d %d:%d:%d", &sTime.nYear, &sTime.nMon, &sTime.nDay, &sTime.nHour, &sTime.nMin, &sTime.nSec))
			return false;
#else
		if (-1 == sscanf(szBuf, "%d-%d-%d %d:%d:%d", &sTime.nYear, &sTime.nMon, &sTime.nDay, &sTime.nHour, &sTime.nMin, &sTime.nSec))
			return false;
#endif
		return true;
	}
}

namespace base
{
	namespace time_util
	{
		int64_t getProcessPassTime()
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

					nFirstCheckTime = (counter.QuadPart * 1000000) / freq.QuadPart;
				}
			};

			static struct SFreq sFreq;

			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);

			int64_t nTime = (counter.QuadPart * 1000000) / sFreq.freq.QuadPart;

			return nTime - sFreq.nFirstCheckTime;
#else
			// 在 x86-64 平台上，clock_gettime 不是系统调用，而是用vdso机制做的，就一个函数调用开销
			// 在编译链接时需加上 -lrt ;因为在librt中实现了clock_gettime函数
			// CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
			int64_t nTime = 0;
			struct timespec ts;
			if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
				nTime = ((int64_t)ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
			// 上面不做强转的话如果编译成32位程序就会出现49天问题
			return nTime;
#endif
		}

		int64_t getGmtTime()
		{
			return getGmtProcessStartTimePoint() + getProcessPassTime() / 1000;
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
			// LocalTime = GmtTime - ZoneTime
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
			sTime.nWDay = ttm.tm_wday;
			sTime.nMon = ttm.tm_mon + 1;
			sTime.nYear = (uint16_t)(1900 + ttm.tm_year);
			sTime.nMSec = nTime % 1000;

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
			sTime.nWDay = ttm.tm_wday;
			sTime.nMon = ttm.tm_mon + 1;
			sTime.nYear = (uint16_t)(1900 + ttm.tm_year);
			sTime.nMSec = nTime % 1000;

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
			return gmt2LocalTime(((int64_t)tt) * 1000 + sTime.nMSec);
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
#ifdef _WIN32
			time_t tt = _mkgmtime(&ttm);
#else
			time_t tt = timegm(&ttm);
#endif
			return ((int64_t)tt) * 1000 + sTime.nMSec;
		}

		int64_t getGmtTimeByBuf(const char* szBuf)
		{
			STime sTime;
			if (!getTMByBuf(szBuf, sTime))
				return -1;

			return getGmtTimeByTM(sTime);
		}

		int64_t getLocalTimeByBuf(const char* szBuf)
		{
			STime sTime;
			if (!getTMByBuf(szBuf, sTime))
				return -1;

			return getLocalTimeByTM(sTime);
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
			return base::function_util::snprintf(szBuf, nBufSize, "%04d-%02d-%02d %02d:%02d:%02d",
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
			return base::function_util::snprintf(szBuf, nBufSize, "%04d-%02d-%02d %02d:%02d:%02d",
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
			getGmtProcessStartTimePoint() = nTime - getProcessPassTime() / 1000;
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
}