#include "stdafx.h"
#include "service_invoke_holder.h"
#include "core_app.h"

namespace core
{
	CServiceInvokeHolder::CServiceInvokeHolder(CServiceBase* pServiceBase)
		: m_pServiceBase(pServiceBase)
	{
		static uint64_t s_nNextHolderID = 1;

		this->m_nID = s_nNextHolderID++;
	}

	CServiceInvokeHolder::~CServiceInvokeHolder()
	{
		CCoreApp::Inst()->getLogicRunnable()->getTransporter()->delPendingResponseInfo(this->m_nID);
	}

	uint64_t CServiceInvokeHolder::getHolderID() const
	{
		return this->m_nID;
	}

	CServiceBase* CServiceInvokeHolder::getServiceBase() const
	{
		return this->m_pServiceBase;
	}
}