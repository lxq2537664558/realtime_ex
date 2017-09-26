#include "stdafx.h"
#include "transporter.h"
#include "message_dispatcher.h"
#include "coroutine.h"
#include "base_connection_mgr.h"
#include "base_connection_other_node.h"
#include "logic_runnable.h"
#include "message_command.h"
#include "base_app.h"
#include "core_app.h"

#include "libBaseCommon/time_util.h"
#include "libBaseCommon/defer.h"

namespace core
{
	CTransporter::CTransporter()
	{
		this->m_szBuf.resize(UINT16_MAX);
	}

	CTransporter::~CTransporter()
	{

	}

	bool CTransporter::invoke(CCoreService* pCoreService, uint64_t nSessionID, uint64_t nFromActorID, uint32_t nToServiceID, uint64_t nToActorID, const void* pMessage)
	{
		DebugAstEx(pMessage != nullptr && pCoreService != nullptr, false);

		CServiceIDConverter* pServiceIDConverter = pCoreService->getServiceBase()->getServiceIDConverter();
		if (pServiceIDConverter != nullptr)
		{
			nToServiceID = pServiceIDConverter->convert(nToServiceID);
			DebugAstEx(nToServiceID != 0, false);
		}

		CMessageSerializer* pMessageSerializer = pCoreService->getServiceMessageSerializer(nToServiceID);
		DebugAstEx(pMessageSerializer != nullptr, false);

		char szMessageName[_MAX_MESSAGE_NAME_LEN] = { 0 };
		DebugAstEx(pMessageSerializer->getMessageName(pMessage, szMessageName, _countof(szMessageName)), false);

		uint16_t nMessageNameLen = (uint16_t)base::function_util::strnlen(szMessageName, _TRUNCATE);

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			// ���cookice
			request_cookice* pCookice = reinterpret_cast<request_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nFromServiceID = pCoreService->getServiceID();
			pCookice->nFromActorID = nFromActorID;
			pCookice->nToServiceID = nToServiceID;
			pCookice->nToActorID = nToActorID;
			pCookice->nMessageSerializerType = pMessageSerializer->getType();
			pCookice->nMessageNameLen = nMessageNameLen;
			base::function_util::strcpy(pCookice->szMessageName, nMessageNameLen + 1, szMessageName);

			int32_t nCookiceLen = (int32_t)(sizeof(request_cookice) + nMessageNameLen);
			DebugAstEx(nCookiceLen < (int32_t)this->m_szBuf.size(), false);

			int32_t nDataSize = pMessageSerializer->serializeMessageToBuf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_REQUEST, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			int32_t nDataSize = pMessageSerializer->serializeMessageToBuf(pMessage, &this->m_szBuf[0], (uint32_t)(this->m_szBuf.size()));
			if (nDataSize < 0)
				return false;

			char* szBuf = new char[sizeof(SMCT_REQUEST) + nMessageNameLen + nDataSize];
			SMCT_REQUEST* pContext = reinterpret_cast<SMCT_REQUEST*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nFromServiceID = pCoreService->getServiceID();
			pContext->nFromActorID = nFromActorID;
			pContext->nToServiceID = nToServiceID;
			pContext->nToActorID = nToActorID;
			pContext->nMessageSerializerType = pMessageSerializer->getType();
			pContext->nMessageDataLen = (uint16_t)nDataSize;
			pContext->nMessageNameLen = nMessageNameLen;
			base::function_util::strcpy(pContext->szMessageName, nMessageNameLen + 1, szMessageName);
			memcpy(szBuf + sizeof(SMCT_REQUEST) + nMessageNameLen, &this->m_szBuf[0], nDataSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_REQUEST;
			sMessagePacket.nDataSize = sizeof(SMCT_REQUEST);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::response(CCoreService* pCoreService, uint32_t nToServiceID, uint64_t nToActorID, uint64_t nSessionID, uint8_t nResult, const void* pMessage)
	{
		DebugAstEx(pCoreService != nullptr, false);

		CServiceIDConverter* pServiceIDConverter = pCoreService->getServiceBase()->getServiceIDConverter();
		if (pServiceIDConverter != nullptr)
		{
			nToServiceID = pServiceIDConverter->convert(nToServiceID);
			DebugAstEx(nToServiceID != 0, false);
		}

		CMessageSerializer* pMessageSerializer = pCoreService->getServiceMessageSerializer(nToServiceID);
		DebugAstEx(pMessageSerializer != nullptr, false);

		char szMessageName[_MAX_MESSAGE_NAME_LEN] = { 0 };
		if (pMessage != nullptr)
		{
			DebugAstEx(pMessageSerializer->getMessageName(pMessage, szMessageName, _countof(szMessageName)), false);
		}

		uint16_t nMessageNameLen = (uint16_t)base::function_util::strnlen(szMessageName, _TRUNCATE);

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			response_cookice* pCookice = reinterpret_cast<response_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nFromServiceID = pCoreService->getServiceID();
			pCookice->nToServiceID = nToServiceID;
			pCookice->nToActorID = nToActorID;
			pCookice->nMessageSerializerType = pMessageSerializer->getType();
			pCookice->nResult = nResult;
			pCookice->nMessageNameLen = nMessageNameLen;
			base::function_util::strcpy(pCookice->szMessageName, nMessageNameLen + 1, szMessageName);

			int32_t nCookiceLen = (int32_t)(sizeof(response_cookice) + nMessageNameLen);
			DebugAstEx(nCookiceLen < (int32_t)this->m_szBuf.size(), false);

			int32_t nDataSize = 0;
			if (pMessage != nullptr)
			{
				nDataSize = pMessageSerializer->serializeMessageToBuf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
				if (nDataSize < 0)
					return false;
			}
			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_RESPONSE, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			int32_t nDataSize = 0;
			if (pMessage != nullptr)
			{
				nDataSize = pMessageSerializer->serializeMessageToBuf(pMessage, &this->m_szBuf[0], (uint32_t)(this->m_szBuf.size()));
				if (nDataSize < 0)
					return false;
			}

			char* szBuf = new char[sizeof(SMCT_RESPONSE) + nMessageNameLen + nDataSize];
			SMCT_RESPONSE* pContext = reinterpret_cast<SMCT_RESPONSE*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nFromServiceID = pCoreService->getServiceID();
			pContext->nToServiceID = nToServiceID;
			pContext->nToActorID = nToActorID;
			pContext->nMessageSerializerType = pMessageSerializer->getType();
			pContext->nResult = nResult;
			pContext->nMessageDataLen = (uint16_t)nDataSize;
			pContext->nMessageNameLen = nMessageNameLen;
			base::function_util::strcpy(pContext->szMessageName, nMessageNameLen + 1, szMessageName);
			memcpy(szBuf + sizeof(SMCT_RESPONSE) + nMessageNameLen, &this->m_szBuf[0], nDataSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_RESPONSE;
			sMessagePacket.nDataSize = sizeof(SMCT_RESPONSE);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::gate_forward(uint64_t nSessionID, uint32_t nFromServiceID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnection* pBaseConnection = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnection)
			{
				PrintWarning("CTransporter::gate_forward error service_id: {} actor_id: {}", nToServiceID, nToActorID);
				return false;
			}

			gate_forward_cookice cookice;
			cookice.nSessionID = nSessionID;
			cookice.nFromServiceID = nFromServiceID;
			cookice.nToActorID = nToActorID;
			cookice.nToServiceID = nToServiceID;

			pBaseConnection->send(eMT_GATE_FORWARD, &cookice, (uint16_t)sizeof(cookice), pData, pData->nMessageSize);
		}
		else
		{
			char* szBuf = new char[sizeof(SMCT_GATE_FORWARD) + pData->nMessageSize];
			SMCT_GATE_FORWARD* pContext = reinterpret_cast<SMCT_GATE_FORWARD*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nFromServiceID = nFromServiceID;
			pContext->nToServiceID = nToServiceID;
			pContext->nToActorID = nToActorID;
			pContext->nMessageDataLen = pData->nMessageSize;
			memcpy(szBuf + sizeof(SMCT_GATE_FORWARD), pData, pData->nMessageSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_GATE_FORWARD;
			sMessagePacket.nDataSize = sizeof(SMCT_GATE_FORWARD);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::send(CCoreService* pCoreService, uint64_t nSessionID, uint32_t nToServiceID, const void* pMessage)
	{
		DebugAstEx(pMessage != nullptr && pCoreService != nullptr, false);

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CMessageSerializer* pProtobufFactory = pCoreService->getForwardMessageSerializer();
			DebugAstEx(pProtobufFactory != nullptr, false);

			char szMessageName[_MAX_MESSAGE_NAME_LEN] = { 0 };
			DebugAstEx(pProtobufFactory->getMessageName(pMessage, szMessageName, _countof(szMessageName)), false);

			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			gate_send_cookice* pCookice = reinterpret_cast<gate_send_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nToServiceID = nToServiceID;

			int32_t nCookiceLen = (int32_t)sizeof(gate_send_cookice);

			message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0] + nCookiceLen);

			nCookiceLen += (int32_t)sizeof(message_header);

			int32_t nDataSize = pProtobufFactory->serializeMessageToBuf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID_EX(szMessageName);

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_TO_GATE, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			CCoreService* pToCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService(nToServiceID);
			DebugAstEx(pToCoreService != nullptr, false);

			CMessageSerializer* pMessageSerializer = pToCoreService->getForwardMessageSerializer();
			DebugAstEx(pMessageSerializer != nullptr, false);

			char szMessageName[_MAX_MESSAGE_NAME_LEN] = { 0 };
			DebugAstEx(pMessageSerializer->getMessageName(pMessage, szMessageName, _countof(szMessageName)), false);

			message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

			int32_t nDataSize = pMessageSerializer->serializeMessageToBuf(pMessage, &this->m_szBuf[0] + sizeof(message_header), (uint32_t)(this->m_szBuf.size() - sizeof(message_header)));
			if (nDataSize < 0)
				return false;

			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID_EX(szMessageName);

			char* szBuf = new char[sizeof(SMCT_TO_GATE) + pHeader->nMessageSize];
			SMCT_TO_GATE* pContext = reinterpret_cast<SMCT_TO_GATE*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nToServiceID = nToServiceID;
			pContext->nDataSize = pHeader->nMessageSize;
			pContext->pData = szBuf + sizeof(SMCT_TO_GATE);
			memcpy(pContext->pData, &this->m_szBuf[0], pContext->nDataSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_TO_GATE;
			sMessagePacket.nDataSize = sizeof(SMCT_TO_GATE);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::broadcast(CCoreService* pCoreService, const std::vector<uint64_t>& vecSessionID, uint32_t nToServiceID, const void* pMessage)
	{
		if (vecSessionID.empty())
			return true;

		DebugAstEx(pMessage != nullptr && pCoreService != nullptr, false);

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			CMessageSerializer* pMessageSerializer = pCoreService->getForwardMessageSerializer();
			DebugAstEx(pMessageSerializer != nullptr, false);

			char szMessageName[_MAX_MESSAGE_NAME_LEN] = { 0 };
			DebugAstEx(pMessageSerializer->getMessageName(pMessage, szMessageName, _countof(szMessageName)), false);

			gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(&this->m_szBuf[0]);
			pCookice->nToServiceID = nToServiceID;
			pCookice->nSessionCount = (uint16_t)vecSessionID.size();

			int32_t nCookiceLen = (int32_t)sizeof(gate_broadcast_cookice);
			memcpy(&this->m_szBuf[0] + nCookiceLen, &vecSessionID[0], sizeof(uint64_t) * vecSessionID.size());
			nCookiceLen += (int32_t)(sizeof(uint64_t) *  vecSessionID.size());

			message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0] + nCookiceLen);

			nCookiceLen += (int32_t)sizeof(message_header);

			DebugAstEx(nCookiceLen < (int32_t)this->m_szBuf.size(), false);

			int32_t nDataSize = pMessageSerializer->serializeMessageToBuf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID_EX(szMessageName);

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_TO_GATE_BROADCAST, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			CCoreService* pToCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService(nToServiceID);
			DebugAstEx(pToCoreService != nullptr, false);

			CMessageSerializer* pMessageSerializer = pToCoreService->getForwardMessageSerializer();
			DebugAstEx(pMessageSerializer != nullptr, false);

			char szMessageName[_MAX_MESSAGE_NAME_LEN] = { 0 };
			DebugAstEx(pMessageSerializer->getMessageName(pMessage, szMessageName, _countof(szMessageName)), false);

			int32_t nSessionLen = (int32_t)(sizeof(uint64_t) * vecSessionID.size());

			int32_t nDataSize = pMessageSerializer->serializeMessageToBuf(pMessage, &this->m_szBuf[0], (uint32_t)(this->m_szBuf.size()));
			if (nDataSize < 0)
				return false;

			char* szBuf = new char[sizeof(SMCT_TO_GATE_BROADCAST) + nSessionLen + sizeof(message_header) + nDataSize];
			SMCT_TO_GATE_BROADCAST* pContext = reinterpret_cast<SMCT_TO_GATE_BROADCAST*>(szBuf);
			pContext->nSessionCount = (uint16_t)vecSessionID.size();
			pContext->nToServiceID = nToServiceID;
			pContext->nDataSize = (uint16_t)(sizeof(message_header) + nDataSize + nSessionLen);
			pContext->pData = szBuf + sizeof(SMCT_TO_GATE_BROADCAST);
			memcpy(pContext->pData, &vecSessionID[0], nSessionLen);
			message_header* pHeader = reinterpret_cast<message_header*>(pContext->pData + nSessionLen);
			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID_EX(szMessageName);
			memcpy(pContext->pData + nSessionLen + sizeof(message_header), &this->m_szBuf[0], nDataSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_TO_GATE_BROADCAST;
			sMessagePacket.nDataSize = sizeof(SMCT_TO_GATE);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}
}