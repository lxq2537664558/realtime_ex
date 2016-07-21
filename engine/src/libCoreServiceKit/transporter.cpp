#include "stdafx.h"
#include "transporter.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_kit_define.h"
#include "core_service_kit_impl.h"

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

	bool CTransporter::call(const std::string& szServiceName, const SRequestMessageInfo& sRequestMessageInfo)
	{
		DebugAstEx(sRequestMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();

		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(szServiceName);
		if (nullptr == pCoreConnectionToService)
		{
			CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("nullptr == pCoreConnectionToService");
			return false;
		}

		uint64_t nSessionID = 0;
		SResponseWaitInfo* pResponseWaitInfo = nullptr;
		if (sRequestMessageInfo.callback != nullptr || sRequestMessageInfo.nCoroutineID != 0)
		{
			nSessionID = this->genSessionID();
			pResponseWaitInfo = new SResponseWaitInfo();
			pResponseWaitInfo->callback = sRequestMessageInfo.callback;
			pResponseWaitInfo->nSessionID = nSessionID;
			pResponseWaitInfo->nTraceID = nTraceID;
			pResponseWaitInfo->nCoroutineID = sRequestMessageInfo.nCoroutineID;
			pResponseWaitInfo->szServiceName = szServiceName;
			pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
			CBaseApp::Inst()->registerTicker(&pResponseWaitInfo->tickTimeout, CCoreServiceKitImpl::Inst()->getInvokeTimeout(), 0, nSessionID);

			this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;
		}

		// 野割cookice
		request_cookice cookice;
		cookice.nSessionID = nSessionID;
		cookice.nTraceID = nTraceID;

		pCoreConnectionToService->send(eMT_REQUEST, &cookice, sizeof(cookice), sRequestMessageInfo.pData, sRequestMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::response(const std::string& szServiceName, const SResponseMessageInfo& sResponseMessageInfo)
	{
		DebugAstEx(sResponseMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();
		
		CCoreConnectionFromService* pCoreConnectionFromService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getCoreConnectionFromService(szServiceName);
		if (pCoreConnectionFromService == nullptr)
		{
			CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("pCoreServiceConnection == nullptr by response");
			return false;
		}

		response_cookice cookice;
		cookice.nTraceID = nTraceID;
		cookice.nSessionID = sResponseMessageInfo.nSessionID;
		cookice.nResult = sResponseMessageInfo.nResult;
		
		pCoreConnectionFromService->send(eMT_RESPONSE, &cookice, sizeof(cookice), sResponseMessageInfo.pData, sResponseMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::forward(const std::string& szServiceName, const SGateForwardMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pData != nullptr, false);

		CCoreServiceKitImpl::Inst()->getInvokerTrace()->startNewTrace();
		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();

		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(szServiceName);
		if (nullptr == pCoreConnectionToService)
		{
			CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("nullptr == pCoreConnectionToService");
			return false;
		}

		
		// 野割cookice
		gate_cookice cookice;
		cookice.nSessionID = sGateMessageInfo.nSessionID;
		cookice.nTraceID = nTraceID;

		pCoreConnectionToService->send(eMT_GATE_FORWARD, &cookice, sizeof(cookice), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::send(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();
		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(szServiceName);
		if (nullptr == pCoreConnectionToService)
		{
			CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("nullptr == pCoreConnectionToService");
			return false;
		}

		
		// 野割cookice
		gate_cookice cookice;
		cookice.nSessionID = sGateMessageInfo.nSessionID;
		cookice.nTraceID = nTraceID;

		pCoreConnectionToService->send(eMT_TO_GATE, &cookice, sizeof(cookice), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::broadcast(const std::string& szServiceName, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo)
	{
		DebugAstEx(sGateBroadcastMessageInfo.pData != nullptr && !sGateBroadcastMessageInfo.vecSessionID.empty(), false);

		CCoreConnectionToService* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getCoreConnectionToService(szServiceName);
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

		CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("wait response time out session_id: "UINT64FMT" service_name: %s", pResponseWaitInfo->nSessionID, pResponseWaitInfo->szServiceName.c_str());

		if (pResponseWaitInfo->callback != nullptr)
		{
			uint64_t nCoroutineID = coroutine::start([&](uint64_t){ pResponseWaitInfo->callback(eMT_RESPONSE, nullptr, eRRT_TIME_OUT); });
			coroutine::resume(nCoroutineID, 0);
		}
		else
		{
			if (pResponseWaitInfo->nCoroutineID != 0)
			{
				coroutine::sendMessage(pResponseWaitInfo->nCoroutineID, reinterpret_cast<void*>(eRRT_TIME_OUT));
				coroutine::sendMessage(pResponseWaitInfo->nCoroutineID, nullptr);
				coroutine::sendMessage(pResponseWaitInfo->nCoroutineID, nullptr);
				coroutine::resume(pResponseWaitInfo->nCoroutineID, 0);
			}
		}

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
}