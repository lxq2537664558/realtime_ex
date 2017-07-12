#include "stdafx.h"
#include "transporter.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "coroutine.h"
#include "base_app.h"
#include "base_connection_mgr.h"
#include "base_connection_other_node.h"
#include "logic_runnable.h"
#include "core_app.h"
#include "message_command.h"

#include "libBaseCommon\base_time.h"

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

	bool CTransporter::invoke(EMessageTargetType eType, uint64_t nSessionID, uint64_t nFromID, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::string szMessageName = pMessage->GetTypeName();
		// Ìî³äcookice
		request_cookice* pCookice = reinterpret_cast<request_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nFromID = nFromID;
		pCookice->nToID = nToID;
		pCookice->nTargetType = (uint8_t)eType;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(request_cookice)+pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		uint16_t nToServiceID = 0;
		if (eType == eMTT_Actor)
		{
			CActorIDConverter* pActorIDConverter = CCoreApp::Inst()->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			nToServiceID = pActorIDConverter->convertToServiceID(nToID);
			DebugAstEx(nToServiceID != 0, false);
		}
		else
		{
			nToServiceID = (uint16_t)nToID;
		}

		if (!CCoreApp::Inst()->isOwnerService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			pBaseConnectionOtherNode->send(eMT_REQUEST, &this->m_szBuf[0], nDataSize);
		}
		else
		{
			char* pBuf = new char[sizeof(SMCT_INSIDE_MESSAGE) + nDataSize];
			SMCT_INSIDE_MESSAGE* pContext = reinterpret_cast<SMCT_INSIDE_MESSAGE*>(pBuf);
			pContext->nMessageType = eMT_REQUEST;
			pContext->nDataSize = nDataSize;
			pContext->pData = pBuf + sizeof(SMCT_INSIDE_MESSAGE);
			memcpy(pContext->pData, &this->m_szBuf[0], nDataSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_INSIDE_MESSAGE;
			sMessagePacket.nDataSize = sizeof(SMCT_INSIDE_MESSAGE);
			sMessagePacket.pData = pContext;

			CLogicRunnable::Inst()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::response(EMessageTargetType eType, uint64_t nSessionID, uint8_t nResult, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);
		
		std::string szMessageName = pMessage->GetTypeName();

		response_cookice* pCookice = reinterpret_cast<response_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToID = nToID;
		pCookice->nResult = nResult;
		pCookice->nTargetType = (uint8_t)eType;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(response_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = (uint16_t)(nCookiceLen + (uint16_t)pMessage->ByteSize());

		uint16_t nToServiceID = 0;
		if (eType == eMTT_Actor)
		{
			CActorIDConverter* pActorIDConverter = CCoreApp::Inst()->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			nToServiceID = pActorIDConverter->convertToServiceID(nToID);
			DebugAstEx(nToServiceID != 0, false);
		}
		else
		{
			nToServiceID = (uint16_t)nToID;
		}

		if (!CCoreApp::Inst()->isOwnerService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			pBaseConnectionOtherNode->send(eMT_RESPONSE, &this->m_szBuf[0], nDataSize);
		}
		else
		{
			char* pBuf = new char[sizeof(SMCT_INSIDE_MESSAGE)+nDataSize];
			SMCT_INSIDE_MESSAGE* pContext = reinterpret_cast<SMCT_INSIDE_MESSAGE*>(pBuf);
			pContext->nMessageType = eMT_RESPONSE;
			pContext->nDataSize = nDataSize;
			pContext->pData = pBuf + sizeof(SMCT_INSIDE_MESSAGE);
			memcpy(pContext->pData, &this->m_szBuf[0], nDataSize);
			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_INSIDE_MESSAGE;
			sMessagePacket.nDataSize = sizeof(SMCT_INSIDE_MESSAGE);
			sMessagePacket.pData = pContext;

			CLogicRunnable::Inst()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::invoke_a(EMessageTargetType eType, uint64_t nSessionID, uint64_t nFromActorID, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		if (eType == eMTT_Actor)
		{
			CActorBaseImpl* pToActorBaseImpl = CCoreApp::Inst()->getActorScheduler()->getActorBase(nToID);
			if (pToActorBaseImpl != nullptr)
			{
				google::protobuf::Message* pNewMessage = create_protobuf_message(pMessage->GetTypeName());
				if (nullptr == pMessage)
					return false;

				pNewMessage->CopyFrom(*pMessage);

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = nFromActorID;
				sActorMessagePacket.nSessionID = nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.pMessage = pNewMessage;

				pToActorBaseImpl->getChannel()->send(sActorMessagePacket);

				CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(pToActorBaseImpl);

				return true;
			}
			else
			{
				return this->invoke(eMTT_Actor, nSessionID, nFromActorID, nToID, pMessage);
			}
		}
		else
		{
			return this->invoke(eMTT_Service, nSessionID, nFromActorID, nToID, pMessage);
		}
	}

	bool CTransporter::response_a(EMessageTargetType eType, uint64_t nSessionID, uint8_t nResult, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		if (eType == eMTT_Actor)
		{
			CActorBaseImpl* pToActorBaseImpl = CCoreApp::Inst()->getActorScheduler()->getActorBase(nToID);
			if (pToActorBaseImpl != nullptr)
			{
				google::protobuf::Message* pNewMessage = create_protobuf_message(pMessage->GetTypeName());
				if (nullptr == pMessage)
					return false;

				pNewMessage->CopyFrom(*pMessage);

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = nResult;
				sActorMessagePacket.nSessionID = nSessionID;
				sActorMessagePacket.nType = eMT_RESPONSE;
				sActorMessagePacket.pMessage = pNewMessage;

				pToActorBaseImpl->getChannel()->send(sActorMessagePacket);

				CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(pToActorBaseImpl);

				return true;
			}
			else
			{
				return this->response(eMTT_Actor, nSessionID, nResult, nToID, pMessage);
			}
		}
		else
		{
			return this->response(eMTT_Service, nSessionID, nResult, nToID, pMessage);
		}
	}

	bool CTransporter::forward(EMessageTargetType eType, uint64_t nSessionID, uint64_t nFromID, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);
		
		std::string szMessageName = pMessage->GetTypeName();

		gate_forward_cookice* pCookice = reinterpret_cast<gate_forward_cookice*>(this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToID = nToID;
		pCookice->nFromID = nFromID;
		pCookice->nTargetType = (uint8_t)eType;
		pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
		base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

		uint32_t nCookiceLen = (uint16_t)(sizeof(gate_forward_cookice) + pCookice->nMessageNameLen);
		DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

		if (!pMessage->SerializeToArray(&this->m_szBuf[0] + nCookiceLen, (int32_t)(this->m_szBuf.size() - nCookiceLen)))
			return false;

		uint16_t nDataSize = uint16_t(nCookiceLen + (uint16_t)pMessage->ByteSize());

		uint16_t nToServiceID = 0;
		if (eType == eMTT_Actor)
		{
			CActorIDConverter* pActorIDConverter = CCoreApp::Inst()->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			nToServiceID = pActorIDConverter->convertToServiceID(nToID);
			DebugAstEx(nToServiceID != 0, false);
		}
		else
		{
			nToServiceID = (uint16_t)nToID;
		}

		if (!CCoreApp::Inst()->isOwnerService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			pBaseConnectionOtherNode->send(eMT_GATE_FORWARD, &this->m_szBuf[0], nDataSize);
		}
		else
		{
			char* pBuf = new char[sizeof(SMCT_INSIDE_MESSAGE)+nDataSize];
			SMCT_INSIDE_MESSAGE* pContext = reinterpret_cast<SMCT_INSIDE_MESSAGE*>(pBuf);
			pContext->nMessageType = eMT_GATE_FORWARD;
			pContext->nDataSize = nDataSize;
			pContext->pData = pBuf + sizeof(SMCT_INSIDE_MESSAGE);
			memcpy(pContext->pData, &this->m_szBuf[0], nDataSize);
			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_INSIDE_MESSAGE;
			sMessagePacket.nDataSize = sizeof(SMCT_INSIDE_MESSAGE);
			sMessagePacket.pData = pContext;

			CLogicRunnable::Inst()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::send(uint64_t nSessionID, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

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

		if (!CCoreApp::Inst()->isOwnerService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			pBaseConnectionOtherNode->send(eMT_TO_GATE, &this->m_szBuf[0], nDataSize);
		}
		else
		{
			char* pBuf = new char[sizeof(SMCT_INSIDE_MESSAGE)+nDataSize];
			SMCT_INSIDE_MESSAGE* pContext = reinterpret_cast<SMCT_INSIDE_MESSAGE*>(pBuf);
			pContext->nMessageType = eMT_TO_GATE;
			pContext->nDataSize = nDataSize;
			pContext->pData = pBuf + sizeof(SMCT_INSIDE_MESSAGE);
			memcpy(pContext->pData, &this->m_szBuf[0], nDataSize);
			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_INSIDE_MESSAGE;
			sMessagePacket.nDataSize = sizeof(SMCT_INSIDE_MESSAGE);
			sMessagePacket.pData = pContext;

			CLogicRunnable::Inst()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::broadcast(const std::vector<uint64_t>& vecSessionID, uint16_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		if (vecSessionID.empty())
			return true;

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

		if (!CCoreApp::Inst()->isOwnerService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getCoreOtherNodeProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			pBaseConnectionOtherNode->send(eMT_TO_GATE, &this->m_szBuf[0], nDataSize);
		}
		else
		{
			char* pBuf = new char[sizeof(SMCT_INSIDE_MESSAGE)+nDataSize];
			SMCT_INSIDE_MESSAGE* pContext = reinterpret_cast<SMCT_INSIDE_MESSAGE*>(pBuf);
			pContext->nMessageType = eMT_TO_GATE;
			pContext->nDataSize = nDataSize;
			pContext->pData = pBuf + sizeof(SMCT_INSIDE_MESSAGE);
			memcpy(pContext->pData, &this->m_szBuf[0], nDataSize);
			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_INSIDE_MESSAGE;
			sMessagePacket.nDataSize = sizeof(SMCT_INSIDE_MESSAGE);
			sMessagePacket.pData = pContext;

			CLogicRunnable::Inst()->sendInsideMessage(sMessagePacket);
		}

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

	SResponseWaitInfo* CTransporter::addResponseWaitInfo(uint64_t nSessionID, uint64_t nToID, const std::string& szMessageName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		DebugAstEx(iter == this->m_mapResponseWaitInfo.end(), nullptr);

		SResponseWaitInfo* pResponseWaitInfo = new SResponseWaitInfo();
		pResponseWaitInfo->callback = nullptr;
		pResponseWaitInfo->nSessionID = nSessionID;
		pResponseWaitInfo->nToID = nToID;
		pResponseWaitInfo->szMessageName = szMessageName;
		pResponseWaitInfo->nBeginTime = base::getGmtTime();
		pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(CTicker::eTT_Logic, 0, &pResponseWaitInfo->tickTimeout, CCoreApp::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;

		return pResponseWaitInfo;
	}
}