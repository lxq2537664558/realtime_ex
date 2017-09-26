#include "stdafx.h"
#include "service_invoker.h"
#include "core_common_define.h"
#include "service_invoke_holder.h"
#include "actor_base.h"
#include "coroutine.h"
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

	bool CServiceInvoker::send(uint32_t nServiceID, const void* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		uint64_t nCurWorkActorID = this->m_pServiceBase->m_pCoreService->getActorScheduler()->getCurWorkActorID();
		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, 0, nCurWorkActorID, nServiceID, 0, pMessage);
	}

	bool CServiceInvoker::send_a(uint32_t nServiceID, uint64_t nActorID, const void* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		uint64_t nCurWorkActorID = this->m_pServiceBase->m_pCoreService->getActorScheduler()->getCurWorkActorID();
		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, 0, nCurWorkActorID, nServiceID, nActorID, pMessage);
	}

	void CServiceInvoker::broadcast(const std::string& szServiceType, const void* pMessage)
	{
		DebugAst(pMessage != nullptr);

		const std::vector<uint32_t>& vecServiceID = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceIDByTypeName(szServiceType);
		for (size_t i = 0; i < vecServiceID.size(); ++i)
		{
			this->send(vecServiceID[i], pMessage);
		}
	}

	void CServiceInvoker::response(const SSessionInfo& sSessionInfo, const void* pMessage, uint32_t nErrorCode /* = eRRT_OK */)
	{
		DebugAst(sSessionInfo.nSessionID != 0);

		bool bRet = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->response(this->m_pServiceBase->m_pCoreService, sSessionInfo.nFromServiceID, sSessionInfo.nFromActorID, sSessionInfo.nSessionID, (uint8_t)nErrorCode, pMessage);
		DebugAst(bRet);
	}

	bool CServiceInvoker::send(const SClientSessionInfo& sClientSessionInfo, const void* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->send(this->m_pServiceBase->m_pCoreService, sClientSessionInfo.nSessionID, sClientSessionInfo.nGateServiceID, pMessage);
	}

	bool CServiceInvoker::send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAstEx(pServiceSelector != nullptr, false);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);
		return this->send(nServiceID, pMessage);
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
			if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->broadcast(this->m_pServiceBase->m_pCoreService, iter->second, iter->first, pMessage))
				bRet = false;
		}

		return bRet;
	}

	bool CServiceInvoker::invoke(uint32_t nServiceID, uint64_t nActorID, const void* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<void>, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder)
	{
		if (!this->m_pServiceBase->m_pCoreService->isServiceHealth(nServiceID))
			return false;

		uint64_t nCurWorkActorID = this->m_pServiceBase->m_pCoreService->getActorScheduler()->getCurWorkActorID();
		if (nCurWorkActorID != 0)
		{
			CCoreActor* pCoreActor = this->m_pServiceBase->m_pCoreService->getActorScheduler()->getCoreActor(nCurWorkActorID);
			if (nullptr == pCoreActor)
				return false;

			uint64_t nSessionID = this->m_pServiceBase->m_pCoreService->genSessionID();

			if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, nSessionID, nCurWorkActorID, nServiceID, nActorID, pMessage))
				return false;

			SPendingResponseInfo* pPendingResponseInfo = pCoreActor->addPendingResponseInfo(nServiceID, nSessionID, nCoroutineID, callback, pServiceInvokeHolder != nullptr ? pServiceInvokeHolder->getHolderID() : 0);
			DebugAstEx(nullptr != pPendingResponseInfo, false);
		}
		else
		{
			uint64_t nSessionID = this->m_pServiceBase->m_pCoreService->genSessionID();
			if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, nSessionID, 0, nServiceID, nActorID, pMessage))
				return false;

			SPendingResponseInfo* pPendingResponseInfo = this->m_pServiceBase->m_pCoreService->addPendingResponseInfo(nServiceID, nSessionID, nCoroutineID, callback, pServiceInvokeHolder != nullptr ? pServiceInvokeHolder->getHolderID() : 0);
			DebugAstEx(pPendingResponseInfo != nullptr, false);
		}

		return true;
	}

	bool CServiceInvoker::gate_forward(uint64_t nSessionID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData)
	{
		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->gate_forward(nSessionID, this->m_pServiceBase->getServiceID(), nToServiceID, nToActorID, pData);
	}
}