#include "stdafx.h"
#include "service_invoker.h"
#include "core_common_define.h"
#include "service_invoke_holder.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

namespace core
{
	CServiceInvoker::CServiceInvoker(CServiceBase* pServiceBase)
		: m_pServiceBase(pServiceBase)
	{

	}

	CServiceInvoker::~CServiceInvoker()
	{

	}

	bool CServiceInvoker::send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, 0, eMTT_Service, this->m_pServiceBase->getServiceID(), eType, nID, pMessage);
	}

	void CServiceInvoker::broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		const std::vector<uint32_t>& vecServiceID = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceIDByTypeName(szServiceType);
		for (size_t i = 0; i < vecServiceID.size(); ++i)
		{
			this->send(eMTT_Service, vecServiceID[i], pMessage);
		}
	}

	void CServiceInvoker::response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		bool bRet = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->response(this->m_pServiceBase->m_pCoreService, sSessionInfo.nFromServiceID, sSessionInfo.nFromActorID, sSessionInfo.nSessionID, eRRT_OK, pMessage);
		DebugAst(bRet);
	}

	bool CServiceInvoker::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->send(this->m_pServiceBase->m_pCoreService, sClientSessionInfo.nSessionID, sClientSessionInfo.nSocketID, sClientSessionInfo.nGateServiceID, pMessage);
	}

	bool CServiceInvoker::send(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(szServiceSelectorType);
		DebugAstEx(pServiceSelector != nullptr, false);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, szServiceSelectorType, nServiceSelectorContext);
		return this->send(eMTT_Service, nServiceID, pMessage);
	}

	bool CServiceInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::map<uint32_t, std::vector<std::pair<uint64_t, uint64_t>>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].nGateServiceID].push_back(std::make_pair(vecClientSessionInfo[i].nSessionID, vecClientSessionInfo[i].nSocketID));
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->broadcast(this->m_pServiceBase->m_pCoreService, iter->second, iter->first, pMessage))
				bRet = false;
		}

		return bRet;
	}

	bool CServiceInvoker::invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder)
	{
		uint64_t nSessionID = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->genSessionID();
		if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBase->m_pCoreService, nSessionID, eMTT_Service, this->m_pServiceBase->getServiceID(), eType, nID, pMessage))
			return false;

		SPendingResponseInfo* pPendingResponseInfo = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->addPendingResponseInfo(nSessionID, nID, pMessage->GetTypeName(), callback, pServiceInvokeHolder != nullptr ? pServiceInvokeHolder->getHolderID() : 0);
		DebugAstEx(pPendingResponseInfo != nullptr, false);
		
		return true;
	}

	bool CServiceInvoker::forward(EMessageTargetType eType, uint64_t nID, const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->forward(this->m_pServiceBase->m_pCoreService, eType, nID, sClientSessionInfo, pMessage);
	}

	bool CServiceInvoker::gate_forward(uint64_t nSessionID, uint64_t nSocketID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData)
	{
		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->gate_forward(nSessionID, nSocketID, this->m_pServiceBase->getServiceID(), nToServiceID, nToActorID, pData);
	}
}