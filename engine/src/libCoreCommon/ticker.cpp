#include "stdafx.h"
#include "ticker.h"
#include "base_app.h"

#include "libBaseCommon/base_time.h"
#include "libBaseCommon/debug_helper.h"

namespace core
{

	CTicker::CTicker()
		: m_pTickerNode(nullptr)
	{
	}

	CTicker::~CTicker()
	{
		CBaseApp::Inst()->unregistTicker(this);
	}

	int64_t CTicker::getIntervalTime() const
	{
		return this->m_nIntervalTime;
	}

	int64_t CTicker::getNextTickTime() const
	{
		return this->m_nNextTickTime;
	}

	int64_t CTicker::getRemainTime() const
	{
		if (nullptr == this->m_pTickerNode)
			return 0;

		int64_t nRemainTime = this->m_nNextTickTime - base::getLocalTime();
		return nRemainTime < 0 ? 0 : nRemainTime;
	}

	bool CTicker::isRegist() const
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