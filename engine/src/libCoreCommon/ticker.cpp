#include "stdafx.h"
#include "ticker.h"
#include "ticker_runnable.h"
#include "base_app.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{
	CTicker::CTicker()
		: m_pCoreContext(nullptr)
		, m_nIntervalTime(0)
		, m_nFrom(0)
		, m_nType(eTT_None)
	{
	}

	CTicker::~CTicker()
	{
		CBaseApp::Inst()->unregisterTicker(this);
	}

	CTicker::CTicker(CTicker&& rhs)
	{
		this->m_nIntervalTime = rhs.m_nIntervalTime;
		this->m_nContext = rhs.m_nContext;
		this->m_callback = rhs.m_callback;
		this->m_pCoreContext = rhs.m_pCoreContext;
		if (this->m_pCoreContext != nullptr)
			reinterpret_cast<CCoreTickerNode*>(this->m_pCoreContext)->Value.m_pTicker = this;

		rhs.m_pCoreContext = nullptr;
	}

	CTicker& CTicker::operator = (CTicker&& rhs)
	{
		if (this == &rhs)
			return *this;

		this->m_nIntervalTime = rhs.m_nIntervalTime;
		this->m_nContext = rhs.m_nContext;
		this->m_callback = rhs.m_callback;
		this->m_pCoreContext = rhs.m_pCoreContext;
		if (this->m_pCoreContext != nullptr)
			reinterpret_cast<CCoreTickerNode*>(this->m_pCoreContext)->Value.m_pTicker = this;

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

	void CTicker::setCallback(const std::function<void(uint64_t)>& callback)
	{
		this->m_callback = callback;
	}

	uint64_t CTicker::getContext() const
	{
		return this->m_nContext;
	}

}