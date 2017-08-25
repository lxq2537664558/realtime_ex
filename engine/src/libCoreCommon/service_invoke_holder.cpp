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

	bool CServiceInvokeHolder::send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage)
	{
		return this->m_pServiceBase->getServiceInvoker()->send(eType, nID, pMessage);
	}

	bool CServiceInvokeHolder::send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage)
	{
		return this->m_pServiceBase->getServiceInvoker()->send(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage);
	}

	void CServiceInvokeHolder::broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage)
	{
		return this->m_pServiceBase->getServiceInvoker()->broadcast(szServiceType, pMessage);
	}

	void CServiceInvokeHolder::response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage, uint32_t nErrorCode /*= eRRT_OK*/)
	{
		return this->m_pServiceBase->getServiceInvoker()->response(sSessionInfo, pMessage, nErrorCode);
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