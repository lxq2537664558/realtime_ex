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

// Ϊ�˱�֤��ʼ��˳�������ʱ��ֻ��ͨ���ṹ�徲̬�����ķ�ʽ��ȡ
static int64_t& getGmtProcessStartTimePoint()
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
			// �� x86-64 ƽ̨�ϣ�clock_gettime ����ϵͳ���ã�������vdso�������ģ���һ���������ÿ���
			// �ڱ�������ʱ����� -lrt ;��Ϊ��librt��ʵ����clock_gettime����
			// CLOCK_MONOTONIC:��ϵͳ������һ����ʼ��ʱ,����ϵͳʱ�䱻�û��ı��Ӱ��
			int64_t nTime = 0;
			struct timespec ts;
			if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
				nTime = ((int64_t)ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
			// ���治��ǿת�Ļ���������32λ����ͻ����49������
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
					// 				// ͨ��gettimeofday��ȡ��ʱ��ֵ�������ϵͳ����ʱ����û��������ȡ������Ȼ�Ǿɵ�ֵ���Ƚ�Σ�գ�����linux�ֲ���Ҳ˵��timezone��ò�Ҫ�ã�����ͳһ����
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
			ǧ������localtime ����ǲ�������ģ������þ�̬����������
			������������
			time_t time;
			tm* pTM1 = localtime( &time );
			tm* pTM2 = localtime( &time );
			����pTM1��pTM2��һ���ģ���Ϊָ��Ķ���һ��
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

		int64_t getGmtTimeByBuf(char* szBuf)
		{
			if (szBuf == nullptr)
				return -1;

			std::string szTime = szBuf;
			if (std::string::npos != szTime.find_first_not_of("0123456789-: "))
				return -1;

			uint32_t nCount = 0;
			std::string::size_type nPos = std::string::npos;
			while (std::string::npos != (nPos = szTime.find_first_of("-: ")))
			{
				++nCount;
				if (nCount >= 1 && nCount <= 2)
				{
					if ('-' != szTime[nPos])
						return -1;
				}
				else if (3 == nCount)
				{
					if (' ' != szTime[nPos])
						return -1;
				}
				else if (nCount >= 4)
				{
					if (':' != szTime[nPos])
						return -1;
				}
				szTime.erase(0, nPos + 1);
			}

			if (nCount != 5)
				return -1;

#ifdef _WIN32
			STime sTime;
			if (-1 == sscanf_s(szBuf, "%d-%d-%d %d:%d:%d", &sTime.nYear, &sTime.nMon, &sTime.nDay, &sTime.nHour, &sTime.nMin, &sTime.nSec))
				return -1;
#else
			STime sTime;
			if (-1 == sscanf(szBuf, "%d-%d-%d %d:%d:%d", &sTime.nYear, &sTime.nMon, &sTime.nDay, &sTime.nHour, &sTime.nMin, &sTime.nSec))
				return -1;
#endif
			return getGmtTimeByTM(sTime);
		}

		int64_t getLocalTimeByBuf(char* szBuf)
		{
			int64_t nTime = getGmtTimeByBuf(szBuf);
			if (nTime < 0)
				return nTime;

			return gmt2LocalTime(nTime);
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