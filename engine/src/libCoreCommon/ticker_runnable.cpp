#include "stdafx.h"
#include "ticker_runnable.h"
#include "core_common.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>
#include <thread>

// 放这里为了调试或者看dump的时候方便
core::CTickerRunnable*	g_pTickerRunnable;

#define _CYCLE_TIME 10


namespace core
{
	CTickerRunnable::CTickerRunnable()
		: m_pTickerMgr(nullptr)
		, m_pThreadBase(nullptr)
	{
	}

	CTickerRunnable::~CTickerRunnable()
	{
		SAFE_DELETE(this->m_pTickerMgr);
		SAFE_RELEASE(this->m_pThreadBase);
	}

	CTickerRunnable* CTickerRunnable::Inst()
	{
		if (g_pTickerRunnable == nullptr)
			g_pTickerRunnable = new core::CTickerRunnable();

		return g_pTickerRunnable;
	}

	bool CTickerRunnable::init()
	{
		this->m_pTickerMgr = new core::CTickerMgr();
		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
	}

	void CTickerRunnable::join()
	{
		this->m_pThreadBase->join();
	}

	void CTickerRunnable::release()
	{
		delete g_pTickerRunnable;
		g_pTickerRunnable = nullptr;
	}

	bool CTickerRunnable::onInit()
	{
		return true;
	}

	void CTickerRunnable::onDestroy()
	{

	}

	bool CTickerRunnable::onProcess()
	{
		int64_t nCurTime = base::getGmtTime();
		this->m_pTickerMgr->update(nCurTime);
		int64_t nEndTime = base::getGmtTime();
		int64_t nDeltaTime = nEndTime - nCurTime;
		if (_CYCLE_TIME - nDeltaTime > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(_CYCLE_TIME - nDeltaTime));

		return true;
	}
}