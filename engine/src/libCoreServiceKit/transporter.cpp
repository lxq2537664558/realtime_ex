#include "stdafx.h"
#include "transporter.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_kit_define.h"
#include "core_service_app_impl.h"

#include "libBaseCommon/defer.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/coroutine.h"

namespace core
{
	CTransporter::CTransporter()
		: m_nNextSessionID(0)
	{
	}

	CTransporter::~CTransporter()
	{

	}

	bool CTransporter::init()
	{
		return true;
	}

	uint64_t CTransporter::genSessionID()
	{
		++this->m_nNextSessionID;
		if (this->m_nNextSessionID == 0)
			this->m_nNextSessionID = 1;

		return this->m_nNextSessionID;
	}

	bool CTransporter::invoke(uint16_t nServiceID, const SRequestMessageInfo& sRequestMessageInfo)
	{
		DebugAstEx(sRequestMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceAppImpl::Inst()->getInvokerTrace()->getCurTraceID();

		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(nServiceID);
		if (nullptr == pCoreConnectionToService)
		{
			CCoreServiceAppImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("nullptr == pCoreConnectionToService");
			return false;
		}

		// 野割cookice
		request_cookice cookice;
		cookice.nSessionID = sRequestMessageInfo.nSessionID;
		cookice.nTraceID = nTraceID;
		cookice.nFromActorID = sRequestMessageInfo.nFromActorID;
		cookice.nToActorID = sRequestMessageInfo.nToActorID;

		pCoreConnectionToService->send(eMT_REQUEST, &cookice, sizeof(cookice), sRequestMessageInfo.pData, sRequestMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::response(uint16_t nServiceID, const SResponseMessageInfo& sResponseMessageInfo)
	{
		DebugAstEx(sResponseMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceAppImpl::Inst()->getInvokerTrace()->getCurTraceID();
		
		CCoreConnectionFromService* pCoreConnectionFromService = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getCoreConnectionFromService(nServiceID);
		if (pCoreConnectionFromService == nullptr)
		{
			CCoreServiceAppImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("pCoreServiceConnection == nullptr by response");
			return false;
		}

		response_cookice cookice;
		cookice.nTraceID = nTraceID;
		cookice.nSessionID = sResponseMessageInfo.nSessionID;
		cookice.nResult = sResponseMessageInfo.nResult;
		cookice.nActorID = sResponseMessageInfo.nToActorID;

		pCoreConnectionFromService->send(eMT_RESPONSE, &cookice, sizeof(cookice), sResponseMessageInfo.pData, sResponseMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::forward(uint16_t nServiceID, const SGateForwardMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pData != nullptr, false);

		CCoreServiceAppImpl::Inst()->getInvokerTrace()->startNewTrace();
		uint64_t nTraceID = CCoreServiceAppImpl::Inst()->getInvokerTrace()->getCurTraceID();

		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(nServiceID);
		if (nullptr == pCoreConnectionToService)
		{
			CCoreServiceAppImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("nullptr == pCoreConnectionToService");
			return false;
		}
		
		// 野割cookice
		gate_forward_cookice cookice;
		cookice.nTraceID = nTraceID;
		cookice.nSessionID = sGateMessageInfo.nSessionID;
		cookice.nActorID = sGateMessageInfo.nActorID;
		pCoreConnectionToService->send(eMT_GATE_FORWARD, &cookice, sizeof(cookice), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::send(uint16_t nServiceID, const SGateMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceAppImpl::Inst()->getInvokerTrace()->getCurTraceID();
		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(nServiceID);
		if (nullptr == pCoreConnectionToService)
		{
			CCoreServiceAppImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("nullptr == pCoreConnectionToService");
			return false;
		}
		
		// 野割cookice
		gate_send_cookice cookice;
		cookice.nSessionID = sGateMessageInfo.nSessionID;
		cookice.nTraceID = nTraceID;

		pCoreConnectionToService->send(eMT_TO_GATE, &cookice, sizeof(cookice), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::broadcast(uint16_t nServiceID, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo)
	{
		DebugAstEx(sGateBroadcastMessageInfo.pData != nullptr && !sGateBroadcastMessageInfo.vecSessionID.empty(), false);

		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(nServiceID);
		if (pCoreConnectionToService == nullptr)
			return false;
		
		static char szBuf[4096] = { 0 };
		// 野割cookice
		gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(szBuf);
		pCookice->nCount = (uint16_t)sGateBroadcastMessageInfo.vecSessionID.size();
		memcpy(pCookice + 1, &sGateBroadcastMessageInfo.vecSessionID[0], sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size());

		pCoreConnectionToService->send(eMT_TO_GATE, pCookice, (uint16_t)(sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size()), sGateBroadcastMessageInfo.pData, sGateBroadcastMessageInfo.pData->nMessageSize);

		return true;
	}

	void CTransporter::onRequestMessageTimeout(uint64_t nContext)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nContext);
		if (iter == this->m_mapResponseWaitInfo.end())
		{
			PrintWarning("iter == this->m_mapProtoBufResponseInfo.end() session_id: "UINT64FMT, nContext);
			return;
		}

		SResponseWaitInfo* pResponseWaitInfo = iter->second;
		if (nullptr == pResponseWaitInfo)
		{
			PrintWarning("nullptr == pResponseInfo session_id: "UINT64FMT, nContext);
			return;
		}

		CCoreServiceAppImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("wait response time out session_id: "UINT64FMT, pResponseWaitInfo->nSessionID);

		if (pResponseWaitInfo->err != nullptr)
			pResponseWaitInfo->err(eRRT_TIME_OUT);

		this->m_mapResponseWaitInfo.erase(iter);
		SAFE_DELETE(pResponseWaitInfo);
	}

	SServiceSessionInfo& CTransporter::getServiceSessionInfo()
	{
		return this->m_sServiceSessionInfo;
	}

	SResponseWaitInfo* CTransporter::getResponseWaitInfo(uint64_t nSessionID, bool bErase)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		if (iter == this->m_mapResponseWaitInfo.end())
			return nullptr;

		SResponseWaitInfo* pResponseWaitInfo = iter->second;
		if (bErase)
			this->m_mapResponseWaitInfo.erase(iter);

		return pResponseWaitInfo;
	}

	void CTransporter::addResponseWaitInfo(uint64_t nSessionID, uint64_t nTraceID)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		DebugAst(iter == this->m_mapResponseWaitInfo.end());

		SResponseWaitInfo* pResponseWaitInfo = new SResponseWaitInfo();
		pResponseWaitInfo->callback = nullptr;
		pResponseWaitInfo->nSessionID = nSessionID;
		pResponseWaitInfo->nTraceID = nTraceID;
		pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pResponseWaitInfo->tickTimeout, CCoreServiceAppImpl::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;
	}
}