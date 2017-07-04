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
		this->m_szBuf.resize(UINT16_MAX);
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

	bool CTransporter::invoke(uint64_t nSessionID, uint16_t nFromServiceID, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CCoreConnectionOtherNode* pCoreConnectionOtherNode = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pCoreConnectionOtherNode)
			return false;
		
		std::string szMessageName = pMessage->GetTypeName();
		// 野割cookice
		request_cookice* pCookice = reinterpret_cast<request_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nFromServiceID = nFromServiceID;
		pCookice->nToServiceID = nToServiceID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());
		
		uint16_t nCookiceLen = (uint16_t)(sizeof(request_cookice)+pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, this->m_szBuf.size() - nCookiceLen))
			return false;

		uint16_t nDataSize = nCookiceLen + (uint16_t)pMessage->ByteSize();
		
		pCoreConnectionOtherNode->send(eMT_REQUEST, &this->m_szBuf[0], nDataSize);
		
		return true;
	}

	bool CTransporter::invoke_a(uint64_t nSessionID, uint64_t nFromActorID, uint64_t nToActorID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CActorIDConverter* pActorIDConverter = CCoreServiceAppImpl::Inst()->getActorIDConverter();
		DebugAstEx(pActorIDConverter != nullptr, false);

		uint16_t nToServiceID = pActorIDConverter->convertToServiceID(nToActorID);
		DebugAstEx(nToServiceID != 0, false);

		CCoreConnectionOtherNode* pCoreConnectionOtherNode = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pCoreConnectionOtherNode)
			return false;

		std::string szMessageName = pMessage->GetTypeName();
		// 野割cookice
		actor_request_cookice* pCookice = reinterpret_cast<actor_request_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nFromActorID = nFromActorID;
		pCookice->nToActorID = nToActorID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint16_t nCookiceLen = (uint16_t)(sizeof(actor_request_cookice)+pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, this->m_szBuf.size() - nCookiceLen))
			return false;

		uint16_t nDataSize = nCookiceLen + (uint16_t)pMessage->ByteSize();

		pCoreConnectionOtherNode->send(eMT_ACTOR_REQUEST, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::response(uint64_t nSessionID, uint8_t nResult, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);
		
		CCoreConnectionOtherNode* pCoreConnectionOtherNode = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNodeByServiceID(nToServiceID);
		if (pCoreConnectionOtherNode == nullptr)
			return false;
		
		std::string szMessageName = pMessage->GetTypeName();

		response_cookice* pCookice = reinterpret_cast<response_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nResult = nResult;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint16_t nCookiceLen = (uint16_t)(sizeof(response_cookice)+pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, this->m_szBuf.size() - nCookiceLen))
			return false;

		uint16_t nDataSize = nCookiceLen + (uint16_t)pMessage->ByteSize();

		pCoreConnectionOtherNode->send(eMT_RESPONSE, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::response_a(uint64_t nSessionID, uint8_t nResult, uint64_t nToActorID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CActorIDConverter* pActorIDConverter = CCoreServiceAppImpl::Inst()->getActorIDConverter();
		DebugAstEx(pActorIDConverter != nullptr, false);

		uint16_t nToServiceID = pActorIDConverter->convertToServiceID(nToActorID);
		DebugAstEx(nToServiceID != 0, false);

		CCoreConnectionOtherNode* pCoreConnectionOtherNode = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getCoreConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pCoreConnectionOtherNode)
			return false;

		std::string szMessageName = pMessage->GetTypeName();

		actor_response_cookice* pCookice = reinterpret_cast<actor_response_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToActorID = nToActorID;
		pCookice->nResult = nResult;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint16_t nCookiceLen = (uint16_t)(sizeof(actor_response_cookice)+pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, this->m_szBuf.size() - nCookiceLen))
			return false;

		uint16_t nDataSize = nCookiceLen + (uint16_t)pMessage->ByteSize();

		pCoreConnectionOtherNode->send(eMT_ACTOR_RESPONSE, &this->m_szBuf[0], nDataSize);

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

		message_header* pData = pSerializeAdapter->serialize(sGateForwardMessageInfo.pData, &this->m_szHeadBuf[0], (uint16_t)this->m_szHeadBuf.size());
		if (pData == nullptr)
			return false;
		
		// 野割cookice
		gate_forward_cookice cookice;
		cookice.nSessionID = sGateForwardMessageInfo.nSessionID;
		cookice.nToServiceID = sGateForwardMessageInfo.nActorID;
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

		message_header* pData = pSerializeAdapter->serialize(sGateMessageInfo.pData, &this->m_szHeadBuf[0], (uint16_t)this->m_szHeadBuf.size());
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

		message_header* pData = pSerializeAdapter->serialize(sGateBroadcastMessageInfo.pData, &this->m_szHeadBuf[0], (uint16_t)this->m_szHeadBuf.size());
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