#include "stdafx.h"
#include "service_invoker.h"
#include "core_service_app.h"
#include "core_service_app_impl.h"
#include "core_service_kit_common.h"
#include "core_service_define.h"
#include "actor_base.h"
#include "coroutine.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

namespace core
{
	CServiceInvoker::CServiceInvoker()
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

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pMessage = pMessage;
		sRequestMessageInfo.nSessionID = 0;
		sRequestMessageInfo.nFromActorID = 0;
		sRequestMessageInfo.nToActorID = 0;

		return CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo);
	}

	void CServiceInvoker::response(const void* pData)
	{
		DebugAst(pData != nullptr);

		this->response(CCoreServiceAppImpl::Inst()->getTransporter()->getServiceSessionInfo(), pData);
	}

	void CServiceInvoker::response(const SServiceSessionInfo& sServiceSessionInfo, const void* pData)
	{
		DebugAst(pData != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sServiceSessionInfo.nSessionID;
		sResponseMessageInfo.pData = pData;
		sResponseMessageInfo.nResult = eRRT_OK;
		sResponseMessageInfo.nFromActorID = 0;
		sResponseMessageInfo.nToActorID = 0;

		bool bRet = CCoreServiceAppImpl::Inst()->getTransporter()->response(sServiceSessionInfo.nServiceID, sResponseMessageInfo);
		DebugAst(bRet);
	}

	bool CServiceInvoker::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = sClientSessionInfo.nSessionID;
		sGateMessageInfo.pMessage = pMessage;

		return CCoreServiceAppImpl::Inst()->getTransporter()->send(sClientSessionInfo.nGateNodeID, sGateMessageInfo);
	}

	bool CServiceInvoker::forward(uint16_t nServiceID, uint64_t nSessionID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SGateForwardMessageInfo sGateMessageInfo;
		sGateMessageInfo.nActorID = 0;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pMessage = pMessage;

		return CCoreServiceAppImpl::Inst()->getTransporter()->forward(nServiceID, sGateMessageInfo);
	}

	bool CServiceInvoker::forward_a(uint64_t nActorID, uint64_t nSessionID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SGateForwardMessageInfo sGateMessageInfo;
		sGateMessageInfo.nActorID = nActorID;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pMessage = pMessage;

		CActorIDConverter* pActorIDConverter = CCoreServiceAppImpl::Inst()->getActorIDConverter();
		DebugAstEx(pActorIDConverter != nullptr, false);

		uint16_t nServiceID = pActorIDConverter->convertToServiceID(nActorID);
		DebugAstEx(nServiceID != 0, false);

		return CCoreServiceAppImpl::Inst()->getTransporter()->forward(nServiceID, sGateMessageInfo);
	}

	bool CServiceInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::map<uint16_t, std::vector<uint64_t>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].nGateNodeID].push_back(vecClientSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			SGateBroadcastMessageInfo sGateBroadcastMessageInfo;
			sGateBroadcastMessageInfo.vecSessionID = iter->second;
			sGateBroadcastMessageInfo.pData = pData;
			if (!CCoreServiceAppImpl::Inst()->getTransporter()->broadcast(iter->first, sGateBroadcastMessageInfo))
				bRet = false;
		}

		return bRet;
	}

	bool CServiceInvoker::invokeImpl(uint16_t nServiceID, const void* pData, const std::function<void(CMessagePtr<char>, uint32_t)>& callback)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = pData;
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nFromActorID = 0;
		sRequestMessageInfo.nToActorID = 0;

		if (!CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo))
			return false;

		SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->addResponseWaitInfo(sRequestMessageInfo.nSessionID);
		DebugAstEx(pResponseWaitInfo != nullptr, false);
		pResponseWaitInfo->nToID = nServiceID;
		//pResponseWaitInfo->nMessageID = pData->nMessageID;
		pResponseWaitInfo->nBeginTime = base::getGmtTime();

		pResponseWaitInfo->callback = callback;
		return true;
	}
}