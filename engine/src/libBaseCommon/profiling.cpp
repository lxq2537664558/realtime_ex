#include "stdafx.h"
#include "profiling.h"
#include "debug_helper.h"
#include "noncopyable.h"
#include "base_time.h"
#include "exception_handler.h"

#include <unordered_map>

class CProfilingMgr :
	public base::noncopyable
{
public:
	CProfilingMgr();
	~CProfilingMgr();

	void profilingBeginByLabel(const char* szLabel);
	void endProfilingByLabel(const char* szLabel);
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

	std::unordered_map<const char*, SProfilerInfo>	m_mapProfilingInfoByLabel;
	std::unordered_map<const void*, SProfilerInfo>	m_mapProfilingInfoByAddr;
	int64_t											m_nProfilingMgrSpend;
};

CProfilingMgr::CProfilingMgr()
	: m_nProfilingMgrSpend(0)
{
}

CProfilingMgr::~CProfilingMgr()
{

}

void CProfilingMgr::profilingBeginByLabel(const char* szLabel)
{
	if (nullptr == szLabel)
		return;

	int64_t nBeginTime = base::getGmtTime();
	auto iter = this->m_mapProfilingInfoByLabel.find(szLabel);
	if (iter != this->m_mapProfilingInfoByLabel.end())
	{
		SProfilerInfo& sProfilerInfo = iter->second;
		sProfilerInfo.nPreTime = base::getGmtTime();
	}
	else
	{
		SProfilerInfo& sProfilerInfo = this->m_mapProfilingInfoByLabel[szLabel];
		sProfilerInfo.nPreTime = base::getGmtTime();
		sProfilerInfo.nCount = 0;
		sProfilerInfo.nTotalTime = 0;
		sProfilerInfo.nMaxTime = 0;
		sProfilerInfo.nMinTime = INVALID_32BIT;
	}
	int64_t nEndTime = base::getGmtTime();

	this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
}

void CProfilingMgr::endProfilingByLabel(const char* szLabel)
{
	if (nullptr == szLabel)
		return;

	int64_t nBeginTime = base::getGmtTime();

	auto iter = this->m_mapProfilingInfoByLabel.find(szLabel);
	if (iter == this->m_mapProfilingInfoByLabel.end())
	{
		PrintWarning("profiling error label: %s", szLabel);

		int64_t nEndTime = base::getGmtTime();

		this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
		return;
	}
	SProfilerInfo& sProfilerInfo = iter->second;
	int32_t nDetla = int32_t(base::getGmtTime() - sProfilerInfo.nPreTime);
	if (nDetla < sProfilerInfo.nMinTime)
		sProfilerInfo.nMinTime = nDetla;
	if (nDetla > sProfilerInfo.nMaxTime)
		sProfilerInfo.nMaxTime = nDetla;
	sProfilerInfo.nTotalTime += nDetla;
	++sProfilerInfo.nCount;

	int64_t nEndTime = base::getGmtTime();

	this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
}

void CProfilingMgr::profilingBeginByAddr(const void* pAddr)
{
	if (nullptr == pAddr)
		return;

	int64_t nBeginTime = base::getGmtTime();
	auto iter = this->m_mapProfilingInfoByAddr.find(pAddr);
	if (iter != this->m_mapProfilingInfoByAddr.end())
	{
		SProfilerInfo& sProfilerInfo = iter->second;
		sProfilerInfo.nPreTime = base::getGmtTime();
	}
	else
	{
		SProfilerInfo& sProfilerInfo = this->m_mapProfilingInfoByAddr[pAddr];
		sProfilerInfo.nPreTime = base::getGmtTime();
		sProfilerInfo.nCount = 0;
		sProfilerInfo.nTotalTime = 0;
		sProfilerInfo.nMaxTime = 0;
		sProfilerInfo.nMinTime = INVALID_32BIT;
	}
	int64_t nEndTime = base::getGmtTime();

	this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
}

void CProfilingMgr::endProfilingByAddr(const void* pAddr)
{
	if (nullptr == pAddr)
		return;

	int64_t nBeginTime = base::getGmtTime();

	auto iter = this->m_mapProfilingInfoByAddr.find(pAddr);
	if (iter == this->m_mapProfilingInfoByAddr.end())
	{
		PrintWarning("profiling error addr: 0x%x", pAddr);

		int64_t nEndTime = base::getGmtTime();

		this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
		return;
	}
	SProfilerInfo& sProfilerInfo = iter->second;
	int32_t nDetla = int32_t(base::getGmtTime() - sProfilerInfo.nPreTime);
	if (nDetla < sProfilerInfo.nMinTime)
		sProfilerInfo.nMinTime = nDetla;
	if (nDetla > sProfilerInfo.nMaxTime)
		sProfilerInfo.nMaxTime = nDetla;
	sProfilerInfo.nTotalTime += nDetla;
	++sProfilerInfo.nCount;

	int64_t nEndTime = base::getGmtTime();

	this->m_nProfilingMgrSpend += (nEndTime - nBeginTime);
}

void CProfilingMgr::profiling(int64_t nTotalTime)
{
	if (nTotalTime <= 0)
		return;

	for (auto iter = this->m_mapProfilingInfoByLabel.begin(); iter != this->m_mapProfilingInfoByLabel.end(); ++iter)
	{
		SProfilerInfo& sProfilerInfo = iter->second;
		float fPercentage = 100 * (float)(sProfilerInfo.nTotalTime / (float)nTotalTime);
		float fAvgTime = 0.0f;
		if (sProfilerInfo.nCount != 0)
			fAvgTime = (float)(sProfilerInfo.nTotalTime / (float)sProfilerInfo.nCount);
		base::saveLogEx("Profiling", false, "label: %s min_time: %d max_time: %d avg_time: %f total_time: " INT64FMT " percentage: %f hit_count %d", iter->first, sProfilerInfo.nMinTime, sProfilerInfo.nMaxTime, fAvgTime, sProfilerInfo.nTotalTime, fPercentage, sProfilerInfo.nCount);
		sProfilerInfo.nPreTime = 0;
		sProfilerInfo.nCount = 0;
		sProfilerInfo.nTotalTime = 0;
		sProfilerInfo.nMaxTime = 0;
		sProfilerInfo.nMinTime = INVALID_32BIT;
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
		base::saveLogEx("Profiling", false, "addr: %s min_time: %d max_time: %d avg_time: %f total_time: " INT64FMT " percentage: %f hit_count %d", szAddr, sProfilerInfo.nMinTime, sProfilerInfo.nMaxTime, fAvgTime, sProfilerInfo.nTotalTime, fPercentage, sProfilerInfo.nCount);
		sProfilerInfo.nPreTime = 0;
		sProfilerInfo.nCount = 0;
		sProfilerInfo.nTotalTime = 0;
		sProfilerInfo.nMaxTime = 0;
		sProfilerInfo.nMinTime = INVALID_32BIT;
	}
	this->m_mapProfilingInfoByLabel.clear();
	this->m_mapProfilingInfoByAddr.clear();
	base::saveLogEx("Profiling", false, "profiling spend: %f", this->m_nProfilingMgrSpend / (float)nTotalTime);
	this->m_nProfilingMgrSpend = 0;
}

static CProfilingMgr* g_pProfilingMgr = nullptr;

namespace base
{
	bool initProfiling()
	{
		if (g_pProfilingMgr != nullptr)
			return false;

		g_pProfilingMgr = new CProfilingMgr();
		return true;
	}

	void uninitProfiling()
	{
		SAFE_DELETE(g_pProfilingMgr);
	}

	void profilingBeginByLabel(const char* szLabel)
	{
		g_pProfilingMgr->profilingBeginByLabel(szLabel);
	}

	void profilingEndByLabel(const char* szLabel)
	{
		g_pProfilingMgr->endProfilingByLabel(szLabel);
	}

	void profilingBeginByAddr(const void* pAddr)
	{
		g_pProfilingMgr->profilingBeginByAddr(pAddr);
	}

	void profilingEndByAddr(const void* pAddr)
	{
		g_pProfilingMgr->endProfilingByAddr(pAddr);
	}

	void profiling(int64_t nTotalTime)
	{
		g_pProfilingMgr->profiling(nTotalTime);
	}
}