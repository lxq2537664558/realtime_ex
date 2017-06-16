#include "stdafx.h"
#include "transporter.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_kit_common.h"
#include "core_service_app_impl.h"
#include "coroutine.h"

#include "libBaseCommon/defer.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"


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
		this->m_szBuf.resize(UINT16_MAX);

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

		CCoreConnectionOtherNode* pCoreConnectionOtherService = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNode(nServiceID);
		if (nullptr == pCoreConnectionOtherService)
			return false;
		
		// 野割cookice
		request_cookice cookice;
		cookice.nSessionID = sRequestMessageInfo.nSessionID;
		cookice.nFromID = sRequestMessageInfo.nFromActorID;
		cookice.nToID = sRequestMessageInfo.nToActorID;

		CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nServiceID);
		DebugAstEx(pSerializeAdapter != nullptr, false);
		
		message_header* pData = pSerializeAdapter->serialize(sRequestMessageInfo.pData, &this->m_szBuf[0], (uint16_t)this->m_szBuf.size());
		if (pData == nullptr)
			return false;
		
		pCoreConnectionOtherService->send(eMT_REQUEST, &cookice, sizeof(cookice), pData, pData->nMessageSize);
		
		return true;
	}

	bool CTransporter::response(uint16_t nNodeID, const SResponseMessageInfo& sResponseMessageInfo)
	{
		DebugAstEx(sResponseMessageInfo.pData != nullptr, false);
		
		CCoreConnectionOtherNode* pCoreConnectionFromOtherNode = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionFromOtherNode(nNodeID);
		if (pCoreConnectionFromOtherNode == nullptr)
			return false;
		
		response_cookice cookice;
		cookice.nSessionID = sResponseMessageInfo.nSessionID;
		cookice.nResult = sResponseMessageInfo.nResult;
		cookice.nActorID = sResponseMessageInfo.nToActorID;

		CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nNodeID);
		DebugAstEx(pSerializeAdapter != nullptr, false);

		message_header* pData = pSerializeAdapter->serialize(sResponseMessageInfo.pData, &this->m_szBuf[0], (uint16_t)this->m_szBuf.size());
		if (pData == nullptr)
			return false;
		
		pCoreConnectionFromOtherNode->send(eMT_RESPONSE, &cookice, sizeof(cookice), pData, pData->nMessageSize);

		return true;
	}

	bool CTransporter::forward(uint16_t nServiceID, const SGateForwardMessageInfo& sGateForwardMessageInfo)
	{
		DebugAstEx(sGateForwardMessageInfo.pData != nullptr, false);

		CCoreConnectionOtherNode* pCoreConnectionOtherService = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNode(nServiceID);
		if (nullptr == pCoreConnectionOtherService)
			return false;
		
		CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nServiceID);
		DebugAstEx(pSerializeAdapter != nullptr, false);

		message_header* pData = pSerializeAdapter->serialize(sGateForwardMessageInfo.pData, &this->m_szBuf[0], (uint16_t)this->m_szBuf.size());
		if (pData == nullptr)
			return false;
		
		// 野割cookice
		gate_forward_cookice cookice;
		cookice.nSessionID = sGateForwardMessageInfo.nSessionID;
		cookice.nActorID = sGateForwardMessageInfo.nActorID;
		pCoreConnectionOtherService->send(eMT_GATE_FORWARD, &cookice, sizeof(cookice), pData, pData->nMessageSize);

		return true;
	}

	bool CTransporter::send(uint16_t nServiceID, const SGateMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pData != nullptr, false);

		CCoreConnectionOtherNode* pCoreConnectionOtherService = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNode(nServiceID);
		if (nullptr == pCoreConnectionOtherService)
			return false;
		
		CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nServiceID);
		DebugAstEx(pSerializeAdapter != nullptr, false);

		message_header* pData = pSerializeAdapter->serialize(sGateMessageInfo.pData, &this->m_szBuf[0], (uint16_t)this->m_szBuf.size());
		if (pData == nullptr)
			return false;
		
		// 野割cookice
		gate_send_cookice cookice;
		cookice.nSessionID = sGateMessageInfo.nSessionID;

		pCoreConnectionOtherService->send(eMT_TO_GATE, &cookice, sizeof(cookice), pData, pData->nMessageSize);

		return true;
	}

	bool CTransporter::broadcast(uint16_t nServiceID, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo)
	{
		DebugAstEx(sGateBroadcastMessageInfo.pData != nullptr && !sGateBroadcastMessageInfo.vecSessionID.empty(), false);

		CCoreConnectionOtherNode* pCoreConnectionOtherService = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNode(nServiceID);
		if (pCoreConnectionOtherService == nullptr)
			return false;
		
		static char szBuf[4096] = { 0 };
		// 野割cookice
		gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(szBuf);
		pCookice->nCount = (uint16_t)sGateBroadcastMessageInfo.vecSessionID.size();
		memcpy(pCookice + 1, &sGateBroadcastMessageInfo.vecSessionID[0], sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size());

		CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nServiceID);
		DebugAstEx(pSerializeAdapter != nullptr, false);

		message_header* pData = pSerializeAdapter->serialize(sGateBroadcastMessageInfo.pData, &this->m_szBuf[0], (uint16_t)this->m_szBuf.size());
		if (pData == nullptr)
			return false;
		
		pCoreConnectionOtherService->send(eMT_TO_GATE, pCookice, (uint16_t)(sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size()), pData, pData->nMessageSize);

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

		pResponseWaitInfo->callback(nullptr, eRRT_TIME_OUT);
		
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

	SResponseWaitInfo* CTransporter::addResponseWaitInfo(uint64_t nSessionID)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		DebugAstEx(iter == this->m_mapResponseWaitInfo.end(), nullptr);

		SResponseWaitInfo* pResponseWaitInfo = new SResponseWaitInfo();
		pResponseWaitInfo->callback = nullptr;
		pResponseWaitInfo->nSessionID = nSessionID;
		pResponseWaitInfo->nToID = 0;
		pResponseWaitInfo->nMessageID = 0;
		pResponseWaitInfo->nBeginTime = 0;
		pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pResponseWaitInfo->tickTimeout, CCoreServiceAppImpl::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;

		return pResponseWaitInfo;
	}
}