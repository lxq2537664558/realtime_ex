#include "stdafx.h"
#include "service_invoke_holder.h"
#include "core_app.h"

namespace
{
	uint64_t genHolderID()
	{
		static uint64_t s_nNextHolderID = 1;

		return s_nNextHolderID++;
	}
}

namespace core
{
	CServiceInvokeHolder::CServiceInvokeHolder(CServiceBase* pServiceBase)
		: m_pServiceBase(pServiceBase)
	{
		this->m_nID = genHolderID();
	}

	CServiceInvokeHolder::CServiceInvokeHolder(CServiceInvokeHolder& rhs)
	{
		this->m_pServiceBase = rhs.m_pServiceBase;
		this->m_nID = genHolderID();
	}

	CServiceInvokeHolder& CServiceInvokeHolder::operator=(CServiceInvokeHolder& rhs)
	{
		if (this == &rhs)
			return rhs;

		CCoreApp::Inst()->getLogicRunnable()->getTransporter()->delPendingResponseInfo(this->m_nID);
		this->m_pServiceBase = rhs.m_pServiceBase;
		this->m_nID = genHolderID();

		return *this;
	}

	CServiceInvokeHolder::~CServiceInvokeHolder()
	{
		CCoreApp::Inst()->getLogicRunnable()->getTransporter()->delPendingResponseInfo(this->m_nID);
	}

	bool CServiceInvokeHolder::send(uint32_t nServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, false);

		return this->m_pServiceBase->getServiceInvoker()->send(nServiceID, pMessage);
	}

	bool CServiceInvokeHolder::send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, false);

		return this->m_pServiceBase->getServiceInvoker()->send(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage);
	}

	void CServiceInvokeHolder::broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		return this->m_pServiceBase->getServiceInvoker()->broadcast(szServiceType, pMessage);
	}

	bool CServiceInvokeHolder::send_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, false);

		return this->m_pServiceBase->getServiceInvoker()->send_a(nServiceID, nActorID, pMessage);
	}

	void CServiceInvokeHolder::response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage, uint32_t nErrorCode /*= eRRT_OK*/)
	{
		DebugAst(this->m_pServiceBase != nullptr);

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