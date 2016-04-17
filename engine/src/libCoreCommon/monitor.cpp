#include "stdafx.h"
#include "monitor.h"

#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/base_function.h"
#include "libBaseCommon/debug_helper.h"

#include <atomic>

class CMonitor :
	public base::IRunnable
{
public:
	CMonitor();
	virtual ~CMonitor();

	bool			init();
	void			uninit();
	void			pushMessageMonitor(uint32_t nID);
	void			pushTickerMonitor(void* pAddr);
	void			popMonitor();

private:
	virtual bool	onInit() { return true; }
	virtual void	onDestroy() { }
	virtual bool	onProcess();

private:
	uint32_t				m_nID;
	void*					m_pAddr;
	std::atomic<uint32_t>	m_nVersion;
	uint32_t				m_nCheckVersion;
	base::CThreadBase*		m_pThreadBase;
};

CMonitor::CMonitor()
	: m_nID(0)
	, m_pAddr(nullptr)
	, m_nVersion(0)
	, m_nCheckVersion(0)
	, m_pThreadBase(nullptr)
{
}

CMonitor::~CMonitor()
{

}

bool CMonitor::init()
{
	this->m_pThreadBase = base::CThreadBase::createNew(this);
	if (nullptr == this->m_pThreadBase)
		return false;

	return true;
}

void CMonitor::uninit()
{
	if (this->m_pThreadBase == nullptr)
		return;

	this->m_pThreadBase->quit();
	this->m_pThreadBase->join();
	this->m_pThreadBase->release();
	this->m_pThreadBase = nullptr;
}

void CMonitor::pushMessageMonitor(uint32_t nID)
{
	this->m_nID = nID;
	this->m_pAddr = nullptr;

	this->m_nVersion.store(this->m_nVersion + 1, std::memory_order_release);
}

void CMonitor::pushTickerMonitor(void* pAddr)
{
	this->m_pAddr = pAddr;
	this->m_nID = 0;

	this->m_nVersion.store(this->m_nVersion + 1, std::memory_order_release);
}

void CMonitor::popMonitor()
{
	this->m_nID = 0;
	this->m_pAddr = nullptr;
}

bool CMonitor::onProcess()
{
	if (this->m_nVersion.load(std::memory_order_acquire) == this->m_nCheckVersion)
	{
		if (this->m_nID != 0)
		{
			PrintWarning("dispatch message dead loop message id: %d", this->m_nID);
		}
		else if (this->m_pAddr != nullptr)
		{
			char szAddr[256] = { 0 };
			base::getFunctionInfo(this->m_pAddr, szAddr, _countof(szAddr));
			PrintWarning("update ticker dead loop callback addr: %s", szAddr);
		}
	}
	else
	{
		this->m_nCheckVersion = this->m_nVersion;
	}

	base::sleep(1000);

	return true;
}

static CMonitor* g_pMonitor = nullptr;

namespace core
{
	bool initMonitor()
	{
		DebugAstEx(g_pMonitor == nullptr, false);

		g_pMonitor = new CMonitor();

		return g_pMonitor->init();
	}

	void uninitMonitor()
	{
		if (g_pMonitor != nullptr)
		{
			g_pMonitor->uninit();
			SAFE_DELETE(g_pMonitor);
		}
	}

	void pushMessageMonitor(uint32_t nID)
	{
		g_pMonitor->pushMessageMonitor(nID);
	}

	void pushTickerMonitor(void* pAddr)
	{
		g_pMonitor->pushTickerMonitor(pAddr);
	}

	void popMonitor()
	{
		g_pMonitor->popMonitor();
	}
}