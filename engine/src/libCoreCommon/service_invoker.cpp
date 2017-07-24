#include "stdafx.h"
#include "service_invoker.h"
#include "core_common_define.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

namespace core
{
	CServiceInvoker::CServiceInvoker(CServiceBaseImpl* pServiceBaseImpl)
		: m_pServiceBaseImpl(pServiceBaseImpl)
	{

	}

	CServiceInvoker::~CServiceInvoker()
	{

	}

	bool CServiceInvoker::init()
	{
		return true;
	}

	bool CServiceInvoker::send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBaseImpl, 0, eMTT_Service, this->m_pServiceBaseImpl->getServiceID(), eType, nID, pMessage);
	}

	bool CServiceInvoker::broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		const std::vector<uint32_t> vecServiceID = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceIDByTypeName(szServiceType);
		for (size_t i = 0; i < vecServiceID.size(); ++i)
		{
			this->send(eMTT_Service, vecServiceID[i], pMessage);
		}

		return true;
	}

	void CServiceInvoker::response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		bool bRet = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->response(this->m_pServiceBaseImpl, sSessionInfo.nFromServiceID, sSessionInfo.nFromActorID, sSessionInfo.nSessionID, eRRT_OK, pMessage);
		DebugAst(bRet);
	}

	bool CServiceInvoker::send(const SClientSessionInfo& sClientSessionInfo, const void* pData, uint16_t nDataSize)
	{
		DebugAstEx(pData != nullptr, false);

		return CCoreApp::Inst()->getLogicRunnable()->getTransporter()->send(sClientSessionInfo.nSessionID, sClientSessionInfo.nGateServiceID, pData, nDataSize);
	}

	bool CServiceInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const void* pData, uint16_t nDataSize)
	{
		DebugAstEx(pData != nullptr, false);

		std::map<uint32_t, std::vector<uint64_t>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].nGateServiceID].push_back(vecClientSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->broadcast(iter->second, iter->first, pData, nDataSize))
				bRet = false;
		}

		return bRet;
	}

	bool CServiceInvoker::invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback)
	{
		uint64_t nSessionID = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->genSessionID();
		if (!CCoreApp::Inst()->getLogicRunnable()->getTransporter()->invoke(this->m_pServiceBaseImpl, nSessionID, eMTT_Service, this->m_pServiceBaseImpl->getServiceID(), eType, nID, pMessage))
			return false;

		SPendingResponseInfo* pPendingResponseInfo = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->addPendingResponseInfo(nSessionID, nID, pMessage->GetTypeName(), callback);
		DebugAstEx(pPendingResponseInfo != nullptr, false);
		
		return true;
	}
}