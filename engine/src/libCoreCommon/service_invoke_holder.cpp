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

		if (this->m_pServiceBase != nullptr)
			this->m_pServiceBase->m_pCoreService->delPendingResponseInfo(this->m_nID);
		this->m_pServiceBase = rhs.m_pServiceBase;
		this->m_nID = genHolderID();

		return *this;
	}

	CServiceInvokeHolder::~CServiceInvokeHolder()
	{
		if (this->m_pServiceBase != nullptr)
			this->m_pServiceBase->m_pCoreService->delPendingResponseInfo(this->m_nID);
	}

	bool CServiceInvokeHolder::send(uint32_t nServiceID, const void* pMessage, uint8_t nMessageSerializerType /* = 0 */)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, false);

		return this->m_pServiceBase->getServiceInvoker()->send(nServiceID, pMessage, nMessageSerializerType);
	}

	bool CServiceInvokeHolder::send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, uint8_t nMessageSerializerType /* = 0 */)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, false);

		return this->m_pServiceBase->getServiceInvoker()->send(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage, nMessageSerializerType);
	}

	void CServiceInvokeHolder::broadcast(const std::string& szServiceType, const void* pMessage, uint8_t nMessageSerializerType /* = 0 */)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		return this->m_pServiceBase->getServiceInvoker()->broadcast(szServiceType, pMessage, nMessageSerializerType);
	}

	void CServiceInvokeHolder::response(const SSessionInfo& sSessionInfo, const void* pMessage, uint32_t nErrorCode /*= eRRT_OK*/, uint8_t nMessageSerializerType /* = 0 */)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		return this->m_pServiceBase->getServiceInvoker()->response(sSessionInfo, pMessage, nErrorCode, nMessageSerializerType);
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