#include "stdafx.h"
#include "service_invoker.h"
#include "core_common_define.h"
#include "service_invoke_holder.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"

namespace core
{
	CServiceInvoker::CServiceInvoker(CServiceBase* pServiceBase)
		: m_pServiceBase(pServiceBase)
	{

	}

	CServiceInvoker::~CServiceInvoker()
	{

	}

	bool CServiceInvoker::send(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, uint8_t nMessageSerializerType)
	{
		DebugAstEx(pMessage != nullptr, false);

		if (bCheckHealth && !this->m_pServiceBase->isServiceHealth(nServiceID))
			return false;

		return CCoreApp::Inst()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, nServiceID, 0, pMessage, nMessageSerializerType);
	}

	bool CServiceInvoker::send(uint32_t nServiceID, const void* pMessage, uint8_t nMessageSerializerType /* = 0 */)
	{
		return this->send(true, nServiceID, pMessage, nMessageSerializerType);
	}

	void CServiceInvoker::broadcast(const std::string& szServiceType, const void* pMessage, uint8_t nMessageSerializerType /* = 0 */)
	{
		DebugAst(pMessage != nullptr);

		const std::vector<uint32_t>& vecServiceID = core::CBaseApp::Inst()->getActiveServiceIDByType(szServiceType);
		for (size_t i = 0; i < vecServiceID.size(); ++i)
		{
			this->send(vecServiceID[i], pMessage, nMessageSerializerType);
		}
	}

	void CServiceInvoker::response(const SSessionInfo& sSessionInfo, const void* pMessage, uint32_t nErrorCode /* = eRRT_OK */, uint8_t nMessageSerializerType /* = 0 */)
	{
		DebugAst(sSessionInfo.nSessionID != 0);

		bool bRet = CCoreApp::Inst()->getTransporter()->response(this->m_pServiceBase->m_pCoreService, sSessionInfo.nFromServiceID, sSessionInfo.nSessionID, nErrorCode, pMessage, nMessageSerializerType);
		DebugAst(bRet);
	}

	bool CServiceInvoker::send(const SClientSessionInfo& sClientSessionInfo, const void* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->send(this->m_pServiceBase->m_pCoreService, sClientSessionInfo.nSessionID, sClientSessionInfo.nGateServiceID, pMessage);
	}

	bool CServiceInvoker::send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, uint8_t nMessageSerializerType /* = 0 */)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAstEx(pServiceSelector != nullptr, false);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);

		return this->send(pServiceSelector->isCheckHealth(), nServiceID, pMessage, nMessageSerializerType);
	}

	bool CServiceInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const void* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::map<uint32_t, std::vector<uint64_t>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].nGateServiceID].push_back(vecClientSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			if (!CCoreApp::Inst()->getTransporter()->broadcast(this->m_pServiceBase->m_pCoreService, iter->second, iter->first, pMessage))
				bRet = false;
		}

		return bRet;
	}

	bool CServiceInvoker::invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<void>, uint32_t)>& callback, uint8_t nMessageSerializerType, CServiceInvokeHolder* pServiceInvokeHolder)
	{
		if (bCheckHealth && !this->m_pServiceBase->m_pCoreService->isServiceHealth(nServiceID))
			return false;

		uint64_t nSessionID = this->m_pServiceBase->m_pCoreService->genSessionID();
		if (!CCoreApp::Inst()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, nServiceID, nSessionID, pMessage, nMessageSerializerType))
			return false;

		SPendingResponseInfo* pPendingResponseInfo = this->m_pServiceBase->m_pCoreService->addPendingResponseInfo(nServiceID, nSessionID, nCoroutineID, callback, pServiceInvokeHolder != nullptr ? pServiceInvokeHolder->getHolderID() : 0);
		DebugAstEx(pPendingResponseInfo != nullptr, false);

		return true;
	}

	bool CServiceInvoker::gate_forward(uint64_t nSessionID, uint32_t nToServiceID, const message_header* pData)
	{
		return CCoreApp::Inst()->getTransporter()->gate_forward(nSessionID, this->m_pServiceBase->getServiceID(), nToServiceID, pData);
	}
}