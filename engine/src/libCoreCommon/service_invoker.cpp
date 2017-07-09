#include "stdafx.h"
#include "service_invoker.h"
#include "core_service_kit_common.h"
#include "core_service_define.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

namespace core
{
	CServiceInvoker::CServiceInvoker(uint16_t nServiceID)
		: m_nServiceID(nServiceID)
	{

	}

	CServiceInvoker::~CServiceInvoker()
	{

	}

	bool CServiceInvoker::init()
	{
		return true;
	}

	bool CServiceInvoker::send(uint16_t nServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->invoke(nServiceID, 0, nServiceID, pMessage);
	}

	void CServiceInvoker::response(const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		this->response(CCoreApp::Inst()->getTransporter()->getServiceSessionInfo(), pMessage);
	}

	void CServiceInvoker::response(const SServiceSessionInfo& sServiceSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		bool bRet = CCoreApp::Inst()->getTransporter()->response(sServiceSessionInfo.nSessionID, eRRT_OK, sServiceSessionInfo.nServiceID, pMessage);
		DebugAst(bRet);
	}

	bool CServiceInvoker::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->send(sClientSessionInfo.nSessionID, sClientSessionInfo.nGateServiceID, pMessage);
	}

	bool CServiceInvoker::forward(uint16_t nServiceID, uint64_t nSessionID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->forward(nSessionID, nServiceID, pMessage);
	}

	bool CServiceInvoker::forward_a(uint64_t nActorID, uint64_t nSessionID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->forward_a(nSessionID, nActorID, pMessage);
	}

	bool CServiceInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::map<uint16_t, std::vector<uint64_t>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].nGateServiceID].push_back(vecClientSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			if (!CCoreApp::Inst()->getTransporter()->broadcast(iter->second, iter->first, pMessage))
				bRet = false;
		}

		return bRet;
	}

	bool CServiceInvoker::invoke(uint16_t nServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		uint64_t nSessionID = CCoreApp::Inst()->getTransporter()->genSessionID();
		if (!CCoreApp::Inst()->getTransporter()->invoke(nServiceID, this->m_nServiceID, nServiceID, pMessage))
			return false;

		SResponseWaitInfo* pResponseWaitInfo = CCoreApp::Inst()->getTransporter()->addResponseWaitInfo(nSessionID);
		DebugAstEx(pResponseWaitInfo != nullptr, false);
		pResponseWaitInfo->nToID = nServiceID;
		pResponseWaitInfo->szMessageName = pMessage->GetTypeName();
		pResponseWaitInfo->nBeginTime = base::getGmtTime();

		pResponseWaitInfo->callback = callback;
		return true;
	}
}