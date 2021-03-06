#include "stdafx.h"
#include "profiling.h"
#include "debug_helper.h"
#include "noncopyable.h"
#include "time_util.h"
#include "exception_handler.h"
#include "thread_base.h"

#include <unordered_map>

namespace
{
	bool g_bEnableProfiling = true;

	class CProfilingMgr :
		public base::noncopyable
	{
	public:
		CProfilingMgr();
		~CProfilingMgr();

		void profilingBeginByLabel(const char* szLabel, uint32_t nContext);
		void endProfilingByLabel(const char* szLabel, uint32_t nContext);
		void profilingBeginByAddr(const void* pAddr);
		void endProfilingByAddr(const void* pAddr);
		void profiling(int64_t nTotalTime);

	private:
		struct SProfilerInfo
		{
			int64_t		nPreTime;
			int64_t		nTotalTime;
			int32_t		nMaxTime;
			int32_t		nMinTime;
			uint32_t	nCount;
		};

		std::unordered_map<const char*, std::unordered_map<uint32_t, SProfilerInfo>>	m_mapProfilingInfoByLabel;
		std::unordered_map<const void*, SProfilerInfo>									m_mapProfilingInfoByAddr;
		int64_t																			m_nProfilingMgrSpend;
	};

	CProfilingMgr::CProfilingMgr()
		: m_nProfilingMgrSpend(0)
	{
	}

	CProfilingMgr::~CProfilingMgr()
	{

	}

	void CProfilingMgr::profilingBeginByLabel(const char* szLabel, uint32_t nContext)
	{
		if (nullptr == szLabel || !g_bEnableProfiling)
			return;

		int64_t nBeginTime = base::time_util::getProcessPassTime();
		auto iter = this->m_mapProfilingInfoByLabel.find(szLabel);
		if (iter != this->m_mapProfilingInfoByLabel.end())
		{
			auto& mapProfilingInfo = iter->second;
			auto iterInfo = mapProfilingInfo.find(nContext);
			if (iterInfo == mapProfilingInfo.end())
			{
				SProfilerInfo& sProfilerInfo = mapProfilingInfo[nContext];
				sProfilerInfo.nPreTime = base::time_util::getProcessPassTime();
				sProfilerInfo.nCount = 0;
				sProfilerInfo.nTotalTime = 0;
				sProfilerInfo.nMaxTime = 0;
				sProfilerInfo.nMinTime = INT32_MAX;
			}
			else
			{
				SProfilerInfo& sProfilerInfo = iterInfo->second;
				sProfilerInfo.nPreTime = base::time_util::getProcessPassTime();
			}
		}
		else
		{
			auto& mapProfilingInfo = this->m_mapProfilingInfoByLabel[szLabel];
			SProfilerInfo& sProfilerInfo = mapProfilingInfo[nContext];
			sProfilerInfo.nPreTime = base::time_util::getProcessPassTime();
			sProfilerInfo.nCount = 0;
			sProfilerInfo.nTotalTime = 0;
			sProfilerInfo.nMaxTime = 0;
			sProfilerInfo.nMinTime = INT32_MAX;
		}
		int64_t nEndTime = base::time_util::getProcessPassTime();

		this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
	}

	void CProfilingMgr::endProfilingByLabel(const char* szLabel, uint32_t nContext)
	{
		if (nullptr == szLabel || !g_bEnableProfiling)
			return;

		int64_t nBeginTime = base::time_util::getProcessPassTime();

		auto iter = this->m_mapProfilingInfoByLabel.find(szLabel);
		if (iter == this->m_mapProfilingInfoByLabel.end())
		{
			PrintWarning("profiling error label: {}", szLabel);

			int64_t nEndTime = base::time_util::getProcessPassTime();

			this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
			return;
		}
		auto& mapProfilingInfo = iter->second;
		auto iterInfo = mapProfilingInfo.find(nContext);
		if (iterInfo == mapProfilingInfo.end())
		{
			PrintWarning("profiling error context: {}", nContext);

			int64_t nEndTime = base::time_util::getProcessPassTime();

			this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
			return;
		}
		SProfilerInfo& sProfilerInfo = iterInfo->second;
		int32_t nDetla = int32_t(base::time_util::getProcessPassTime() - sProfilerInfo.nPreTime);
		if (nDetla < sProfilerInfo.nMinTime)
			sProfilerInfo.nMinTime = nDetla;
		if (nDetla > sProfilerInfo.nMaxTime)
			sProfilerInfo.nMaxTime = nDetla;
		sProfilerInfo.nTotalTime += nDetla;
		++sProfilerInfo.nCount;

		int64_t nEndTime = base::time_util::getProcessPassTime();

		this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
	}

	void CProfilingMgr::profilingBeginByAddr(const void* pAddr)
	{
		if (nullptr == pAddr || !g_bEnableProfiling)
			return;

		int64_t nBeginTime = base::time_util::getProcessPassTime();
		auto iter = this->m_mapProfilingInfoByAddr.find(pAddr);
		if (iter != this->m_mapProfilingInfoByAddr.end())
		{
			SProfilerInfo& sProfilerInfo = iter->second;
			sProfilerInfo.nPreTime = base::time_util::getProcessPassTime();
		}
		else
		{
			SProfilerInfo& sProfilerInfo = this->m_mapProfilingInfoByAddr[pAddr];
			sProfilerInfo.nPreTime = base::time_util::getProcessPassTime();
			sProfilerInfo.nCount = 0;
			sProfilerInfo.nTotalTime = 0;
			sProfilerInfo.nMaxTime = 0;
			sProfilerInfo.nMinTime = INT32_MAX;
		}
		int64_t nEndTime = base::time_util::getProcessPassTime();

		this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
	}

	void CProfilingMgr::endProfilingByAddr(const void* pAddr)
	{
		if (nullptr == pAddr || !g_bEnableProfiling)
			return;

		int64_t nBeginTime = base::time_util::getProcessPassTime();

		auto iter = this->m_mapProfilingInfoByAddr.find(pAddr);
		if (iter == this->m_mapProfilingInfoByAddr.end())
		{
			int64_t nEndTime = base::time_util::getProcessPassTime();

			this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
			return;
		}
		SProfilerInfo& sProfilerInfo = iter->second;
		int32_t nDetla = int32_t(base::time_util::getProcessPassTime() - sProfilerInfo.nPreTime);
		if (nDetla < sProfilerInfo.nMinTime)
			sProfilerInfo.nMinTime = nDetla;
		if (nDetla > sProfilerInfo.nMaxTime)
			sProfilerInfo.nMaxTime = nDetla;
		sProfilerInfo.nTotalTime += nDetla;
		++sProfilerInfo.nCount;

		int64_t nEndTime = base::time_util::getProcessPassTime();

		this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
	}

	void CProfilingMgr::profiling(int64_t nTotalTime)
	{
		if (nTotalTime <= 0 || !g_bEnableProfiling)
			return;

		base::log::saveEx("Profiling", false, "thread_id: %d", base::CThreadBase::getCurrentID());
		for (auto iter = this->m_mapProfilingInfoByLabel.begin(); iter != this->m_mapProfilingInfoByLabel.end(); ++iter)
		{
			auto& mapProfilingInfo = iter->second;
			for (auto iterInfo = mapProfilingInfo.begin(); iterInfo != mapProfilingInfo.end(); ++iterInfo)
			{
				uint32_t nContext = iterInfo->first;
				SProfilerInfo& sProfilerInfo = iterInfo->second;
				float fPercentage = 100 * (float)(sProfilerInfo.nTotalTime / (float)nTotalTime);
				float fAvgTime = 0.0f;
				if (sProfilerInfo.nCount != 0)
					fAvgTime = (float)(sProfilerInfo.nTotalTime / (float)sProfilerInfo.nCount);
				base::log::saveEx("Profiling", false, "label: %s context: %u min_time: %d max_time: %d avg_time: %f total_time: " INT64FMT " percentage: %f hit_count %d", iter->first, nContext, sProfilerInfo.nMinTime, sProfilerInfo.nMaxTime, fAvgTime, sProfilerInfo.nTotalTime, fPercentage, sProfilerInfo.nCount);
				sProfilerInfo.nPreTime = 0;
				sProfilerInfo.nCount = 0;
				sProfilerInfo.nTotalTime = 0;
				sProfilerInfo.nMaxTime = 0;
				sProfilerInfo.nMinTime = INT32_MAX;
			}
		}

		char szAddr[256] = { 0 };
		for (auto iter = this->m_mapProfilingInfoByAddr.begin(); iter != this->m_mapProfilingInfoByAddr.end(); ++iter)
		{
			SProfilerInfo& sProfilerInfo = iter->second;
			float fPercentage = 100 * (float)(sProfilerInfo.nTotalTime / (float)nTotalTime);
			float fAvgTime = 0.0f;
			if (sProfilerInfo.nCount != 0)
				fAvgTime = (float)(sProfilerInfo.nTotalTime / (float)sProfilerInfo.nCount);


			base::getFunctionInfo(iter->first, szAddr, _countof(szAddr));
			base::log::saveEx("Profiling", false, "addr: %s min_time: %d max_time: %d avg_time: %f total_time: " INT64FMT " percentage: %f hit_count %d", szAddr, sProfilerInfo.nMinTime, sProfilerInfo.nMaxTime, fAvgTime, sProfilerInfo.nTotalTime, fPercentage, sProfilerInfo.nCount);
			sProfilerInfo.nPreTime = 0;
			sProfilerInfo.nCount = 0;
			sProfilerInfo.nTotalTime = 0;
			sProfilerInfo.nMaxTime = 0;
			sProfilerInfo.nMinTime = INT32_MAX;
		}
		this->m_mapProfilingInfoByLabel.clear();
		this->m_mapProfilingInfoByAddr.clear();
		base::log::saveEx("Profiling", false, "profiling spend: %f", this->m_nProfilingMgrSpend / (float)nTotalTime);
		this->m_nProfilingMgrSpend = 0;
	}

	CProfilingMgr* getProfilingMgr()
	{
		static thread_local CProfilingMgr s_Inst;

		return &s_Inst;
	}
}

namespace base
{
	namespace profiling
	{
		bool init(bool bEnable)
		{
			g_bEnableProfiling = bEnable;
			return true;
		}

		void enable(bool bEnable)
		{
			g_bEnableProfiling = bEnable;
		}

		void uninit()
		{

		}

		void beginByLabel(const char* szLabel, uint32_t nContext)
		{
			getProfilingMgr()->profilingBeginByLabel(szLabel, nContext);
		}

		void endByLabel(const char* szLabel, uint32_t nContext)
		{
			getProfilingMgr()->endProfilingByLabel(szLabel, nContext);
		}

		void beginByAddr(const void* pAddr)
		{
			getProfilingMgr()->profilingBeginByAddr(pAddr);
		}

		void endByAddr(const void* pAddr)
		{
			getProfilingMgr()->endProfilingByAddr(pAddr);
		}

		void update(int64_t nTotalTime)
		{
			getProfilingMgr()->profiling(nTotalTime);
		}
	}
}