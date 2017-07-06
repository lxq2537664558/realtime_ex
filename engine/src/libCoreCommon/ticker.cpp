#include "stdafx.h"
#include "ticker.h"
#include "ticker_runnable.h"
#include "base_app.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{

	CTicker::CTicker()
		: m_pCoreContext(nullptr)
	{
	}

	CTicker::~CTicker()
	{
		CBaseApp::Inst()->unregisterTicker(this);
	}

	CTicker::CTicker(CTicker&& rhs)
	{
		this->m_pCoreContext.store(rhs.m_pCoreContext, std::memory_order_release);
		if (this->m_pCoreContext)
		reinterpret_cast<CCoreTickerInfo*>()
		this->m_nIntervalTime = rhs.m_nIntervalTime;
		this->m_nContext = rhs.m_nContext;
		this->m_callback = rhs.m_callback;

		rhs.m_pCoreContext = nullptr;
	}

	CTicker& CTicker::operator = (CTicker&& rhs)
	{
		if (this == &rhs)
			return *this;

		this->m_pTickerNode = rhs.m_pTickerNode;
		this->m_nIntervalTime = rhs.m_nIntervalTime;
		this->m_nContext = rhs.m_nContext;
		this->m_callback = rhs.m_callback;

		rhs.m_pTickerNode = nullptr;

		return *this;
	}

	int64_t CTicker::getIntervalTime() const
	{
		return this->m_nIntervalTime;
	}

	bool CTicker::isRegister() const
	{
		return this->m_pTickerNode != nullptr;
	}

	void CTicker::setCallback(const std::function<void(uint64_t)>& callback)
	{
		this->m_callback = callback;
	}

	uint64_t CTicker::getContext() const
	{
		return this->m_nContext;
	}

}