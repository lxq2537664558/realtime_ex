#include "stdafx.h"
#include "transporter.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_kit_common.h"
#include "coroutine.h"
#include "base_app.h"
#include "base_connection_mgr.h"
#include "base_connection_other_node.h"
#include "core_app.h"

#include "libBaseCommon/defer.h"

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

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;
		
		std::string szMessageName = pMessage->GetTypeName();
		// 野割cookice
		request_cookice* pCookice = reinterpret_cast<request_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nFromServiceID = nFromServiceID;
		pCookice->nToServiceID = nToServiceID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());
		
		uint32_t nCookiceLen = (uint16_t)(sizeof(request_cookice)+pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());
		
		pBaseConnectionOtherNode->send(eMT_REQUEST, &this->m_szBuf[0], nDataSize);
		
		return true;
	}

	bool CTransporter::invoke_a(uint64_t nSessionID, uint64_t nFromActorID, uint64_t nToActorID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CActorIDConverter* pActorIDConverter = CCoreApp::Inst()->getActorIDConverter();
		DebugAstEx(pActorIDConverter != nullptr, false);

		uint16_t nToServiceID = pActorIDConverter->convertToServiceID(nToActorID);
		DebugAstEx(nToServiceID != 0, false);

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;

		std::string szMessageName = pMessage->GetTypeName();
		// 野割cookice
		actor_request_cookice* pCookice = reinterpret_cast<actor_request_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nFromActorID = nFromActorID;
		pCookice->nToActorID = nToActorID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(actor_request_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		pBaseConnectionOtherNode->send(eMT_ACTOR_REQUEST, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::response(uint64_t nSessionID, uint8_t nResult, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);
		
		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (pBaseConnectionOtherNode == nullptr)
			return false;
		
		std::string szMessageName = pMessage->GetTypeName();

		response_cookice* pCookice = reinterpret_cast<response_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nResult = nResult;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(response_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		pBaseConnectionOtherNode->send(eMT_RESPONSE, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::response_a(uint64_t nSessionID, uint8_t nResult, uint64_t nToActorID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CActorIDConverter* pActorIDConverter = CCoreApp::Inst()->getActorIDConverter();
		DebugAstEx(pActorIDConverter != nullptr, false);

		uint16_t nToServiceID = pActorIDConverter->convertToServiceID(nToActorID);
		DebugAstEx(nToServiceID != 0, false);

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;

		std::string szMessageName = pMessage->GetTypeName();

		actor_response_cookice* pCookice = reinterpret_cast<actor_response_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToActorID = nToActorID;
		pCookice->nResult = nResult;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(actor_response_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		pBaseConnectionOtherNode->send(eMT_ACTOR_RESPONSE, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::forward(uint64_t nSessionID, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;
		
		std::string szMessageName = pMessage->GetTypeName();

		gate_forward_cookice* pCookice = reinterpret_cast<gate_forward_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToServiceID = nToServiceID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(gate_forward_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = uint16_t(nCookiceLen + (uint16_t)pMessage->ByteSize());

		pBaseConnectionOtherNode->send(eMT_GATE_FORWARD, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::forward_a(uint64_t nSessionID, uint64_t nToActorID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CActorIDConverter* pActorIDConverter = CCoreApp::Inst()->getActorIDConverter();
		DebugAstEx(pActorIDConverter != nullptr, false);

		uint16_t nToServiceID = pActorIDConverter->convertToServiceID(nToActorID);
		DebugAstEx(nToServiceID != 0, false);

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;

		std::string szMessageName = pMessage->GetTypeName();

		actor_gate_forward_cookice* pCookice = reinterpret_cast<actor_gate_forward_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToActorID = nToActorID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(actor_gate_forward_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		pBaseConnectionOtherNode->send(eMT_ACTOR_GATE_FORWARD, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::send(uint64_t nSessionID, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;

		std::string szMessageName = pMessage->GetTypeName();

		gate_send_cookice* pCookice = reinterpret_cast<gate_send_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToServiceID = nToServiceID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(gate_send_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		pBaseConnectionOtherNode->send(eMT_TO_GATE, &this->m_szBuf[0], nDataSize);
		
		return true;
	}

	bool CTransporter::broadcast(const std::vector<uint64_t>& vecSessionID, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		if (vecSessionID.empty())
			return true;

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;

		std::string szMessageName = pMessage->GetTypeName();

		gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(this->m_szBuf[0]);
		pCookice->nToServiceID = nToServiceID;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(gate_broadcast_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		pCookice->nSessionCount = (uint16_t)vecSessionID.size();
		memcpy(&this->m_szBuf[0] + nCookiceLen, &vecSessionID[0], sizeof(uint64_t) * vecSessionID.size());
		nCookiceLen += (uint32_t)(sizeof(uint64_t) * vecSessionID.size());
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		pBaseConnectionOtherNode->send((uint8_t)(eMT_TO_GATE | eMT_BROADCAST), &this->m_szBuf[0], nDataSize);

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
		pResponseWaitInfo->nBeginTime = 0;
		pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(CTicker::eTT_Logic, 0, &pResponseWaitInfo->tickTimeout, CCoreApp::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;

		return pResponseWaitInfo;
	}
}