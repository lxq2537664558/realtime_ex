#include "stdafx.h"
#include "ticker.h"
#include "ticker_mgr_impl.h"
#include "debug_helper.h"

namespace base
{
	CTicker::CTicker()
		: m_pCoreContext(nullptr)
		, m_nIntervalTime(0)
		, m_bCoroutine(false)
	{
		this->m_callback = new std::function<void(uint64_t)>();
	}

	CTicker::~CTicker()
	{
		SAFE_DELETE(this->m_callback);

		if (this->m_pCoreContext == nullptr)
			return;

		reinterpret_cast<CCoreTickerNode*>(this->m_pCoreContext)->Value.pTickerMgr->unregisterTicker(this);
	}

	CTicker::CTicker(CTicker&& rhs)
	{
		this->m_nIntervalTime = rhs.m_nIntervalTime;
		this->m_nContext = rhs.m_nContext;
		this->m_bCoroutine = rhs.m_bCoroutine;
		*this->m_callback = *rhs.m_callback;
		this->m_pCoreContext = rhs.m_pCoreContext;
		if (this->m_pCoreContext != nullptr)
			reinterpret_cast<CCoreTickerNode*>(this->m_pCoreContext)->Value.pTicker = this;

		rhs.m_pCoreContext = nullptr;
	}

	CTicker& CTicker::operator = (CTicker&& rhs)
	{
		if (this == &rhs)
			return *this;

		if(this->m_pCoreContext != nullptr)
			reinterpret_cast<CCoreTickerNode*>(this->m_pCoreContext)->Value.pTickerMgr->unregisterTicker(this);

		this->m_nIntervalTime = rhs.m_nIntervalTime;
		this->m_nContext = rhs.m_nContext;
		this->m_bCoroutine = rhs.m_bCoroutine;
		*this->m_callback = *rhs.m_callback;
		this->m_pCoreContext = rhs.m_pCoreContext;
		if (this->m_pCoreContext != nullptr)
			reinterpret_cast<CCoreTickerNode*>(this->m_pCoreContext)->Value.pTicker = this;

		rhs.m_pCoreContext = nullptr;

		return *this;
	}

	int64_t CTicker::getIntervalTime() const
	{
		return this->m_nIntervalTime;
	}

	bool CTicker::isRegister() const
	{
		return this->m_pCoreContext != nullptr;
	}

	void CTicker::setCallback(const std::function<void(uint64_t)>& callback, bool bCoroutine /* = false */)
	{
		*this->m_callback = callback;
		this->m_bCoroutine = bCoroutine;
	}

	std::function<void(uint64_t)>& CTicker::getCallback()
	{
		return *this->m_callback;
	}

	bool CTicker::isCoroutine() const
	{
		return this->m_bCoroutine;
	}

	uint64_t CTicker::getContext() const
	{
		return this->m_nContext;
	}

	CTickerMgr::CTickerMgr(int64_t nTime, const std::function<void(CTicker*)>& callback)
	{
		this->m_pTickerMgrImpl = new CTickerMgrImpl(nTime, callback);
	}

	CTickerMgr::~CTickerMgr()
	{
	}

	int64_t CTickerMgr::getLogicTime() const
	{
		return this->m_pTickerMgrImpl->getLogicTime();
	}

	void CTickerMgr::update(int64_t nTime)
	{
		this->m_pTickerMgrImpl->update(nTime);
	}

	bool CTickerMgr::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		return this->m_pTickerMgrImpl->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CTickerMgr::unregisterTicker(CTicker* pTicker)
	{
		this->m_pTickerMgrImpl->unregisterTicker(pTicker);
	}
}