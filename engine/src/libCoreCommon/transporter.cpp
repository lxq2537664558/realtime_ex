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
		: m_nNextSessionID(0)
	{
		this->m_szBuf.resize(UINT16_MAX);
	}

	CTransporter::~CTransporter()
	{

	}

	uint64_t CTransporter::genSessionID()
	{
		++this->m_nNextSessionID;
		if (this->m_nNextSessionID == 0)
			this->m_nNextSessionID = 1;

		return this->m_nNextSessionID;
	}

	bool CTransporter::invoke(CCoreService* pCoreService, uint64_t nSessionID, EMessageTargetType eFromType, uint64_t nFromID, EMessageTargetType eToType, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr && pCoreService != nullptr, false);
		
		uint32_t nToServiceID = 0;
		if (eToType == eMTT_Actor)
		{
			CActorIDConverter* pActorIDConverter = pCoreService->getServiceBase()->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			nToServiceID = pActorIDConverter->convertToServiceID(nToID);
			DebugAstEx(nToServiceID != 0, false);
		}
		else
		{
			nToServiceID = (uint32_t)nToID;
			CServiceIDConverter* pServiceIDConverter = pCoreService->getServiceBase()->getServiceIDConverter();
			if (pServiceIDConverter != nullptr)
			{
				nToServiceID = pServiceIDConverter->convert(nToServiceID);
				DebugAstEx(nToServiceID != 0, false);
			}
		}

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getServiceProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			std::string szMessageName = pMessage->GetTypeName();
			// Ìî³äcookice
			request_cookice* pCookice = reinterpret_cast<request_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nFromServiceID = pCoreService->getServiceID();
			pCookice->nToServiceID = nToServiceID;
			pCookice->nFromActorID = 0;
			pCookice->nToActorID = 0;
			if (eFromType == eMTT_Actor)
				pCookice->nFromActorID = nFromID;
			if (eToType == eMTT_Actor)
				pCookice->nToActorID = nToID;
			pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
			base::function_util::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

			int32_t nCookiceLen = (int32_t)(sizeof(request_cookice)+pCookice->nMessageNameLen);
			DebugAstEx(nCookiceLen < (int32_t)this->m_szBuf.size(), false);

			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_REQUEST, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			CCoreService* pToCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(nToServiceID);
			DebugAstEx(pToCoreService != nullptr, false);

			CProtobufFactory* pProtobufFactory = pToCoreService->getServiceBase()->getServiceProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			google::protobuf::Message* pNewMessage = pProtobufFactory->clone_protobuf_message(pMessage);
			if (nullptr == pNewMessage)
				return false;

			char* szBuf = new char[sizeof(SMCT_REQUEST)];
			SMCT_REQUEST* pContext = reinterpret_cast<SMCT_REQUEST*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nFromServiceID = pCoreService->getServiceID();
			pContext->nToServiceID = nToServiceID;
			pContext->nFromActorID = 0;
			pContext->nToActorID = 0;
			if (eFromType == eMTT_Actor)
				pContext->nFromActorID = nFromID;
			if (eToType == eMTT_Actor)
				pContext->nToActorID = nToID;
			pContext->pMessage = pNewMessage;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_REQUEST;
			sMessagePacket.nDataSize = sizeof(SMCT_REQUEST);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::response(CCoreService* pCoreService, uint32_t nToServiceID, uint64_t nToActorID, uint64_t nSessionID, uint8_t nResult, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pCoreService != nullptr, false);

		CServiceIDConverter* pServiceIDConverter = pCoreService->getServiceBase()->getServiceIDConverter();
		if (pServiceIDConverter != nullptr)
		{
			nToServiceID = pServiceIDConverter->convert(nToServiceID);
			DebugAstEx(nToServiceID != 0, false);
		}

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getServiceProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			std::string szMessageName;
			if (pMessage != nullptr)
				szMessageName = pMessage->GetTypeName();

			response_cookice* pCookice = reinterpret_cast<response_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nToServiceID = nToServiceID;
			pCookice->nResult = nResult;
			pCookice->nToActorID = nToActorID;
			pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
			base::function_util::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

			int32_t nCookiceLen = (int32_t)(sizeof(response_cookice)+pCookice->nMessageNameLen);
			DebugAstEx(nCookiceLen < (int32_t)this->m_szBuf.size(), false);

			int32_t nDataSize = 0;
			if (pMessage != nullptr)
			{
				nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
				if (nDataSize < 0)
					return false;
			}
			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_RESPONSE, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			CCoreService* pToCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(nToServiceID);
			DebugAstEx(pToCoreService != nullptr, false);

			CProtobufFactory* pProtobufFactory = pToCoreService->getServiceBase()->getServiceProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			google::protobuf::Message* pNewMessage = nullptr;

			if (pMessage != nullptr)
			{
				pNewMessage = pProtobufFactory->clone_protobuf_message(pMessage);
				if (nullptr == pNewMessage)
					return false;
			}

			char* szBuf = new char[sizeof(SMCT_RESPONSE)];
			SMCT_RESPONSE* pContext = reinterpret_cast<SMCT_RESPONSE*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nToServiceID = nToServiceID;
			pContext->nToActorID = nToActorID;
			pContext->nResult = nResult;
			pContext->pMessage = pNewMessage;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_RESPONSE;
			sMessagePacket.nDataSize = sizeof(SMCT_RESPONSE);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::forward(CCoreService* pCoreService, EMessageTargetType eType, uint64_t nID, const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr && pCoreService != nullptr, false);

		uint32_t nToServiceID = 0;
		if (eType == eMTT_Actor)
		{
			CActorIDConverter* pActorIDConverter = pCoreService->getServiceBase()->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			nToServiceID = pActorIDConverter->convertToServiceID(nID);
			DebugAstEx(nToServiceID != 0, false);
		}
		else
		{
			nToServiceID = (uint32_t)nID;
			CServiceIDConverter* pServiceIDConverter = pCoreService->getServiceBase()->getServiceIDConverter();
			if (pServiceIDConverter != nullptr)
			{
				nToServiceID = pServiceIDConverter->convert(nToServiceID);
				DebugAstEx(nToServiceID != 0, false);
			}
		}

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getForwardProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			gate_forward_cookice* pCookice = reinterpret_cast<gate_forward_cookice*>(this->m_szBuf[0]);
			pCookice->nSessionID = sClientSessionInfo.nSessionID;
			pCookice->nFromServiceID = sClientSessionInfo.nGateServiceID;
			pCookice->nToServiceID = nToServiceID;
			pCookice->nToActorID = 0;
			if (eType == eMTT_Actor)
				pCookice->nToActorID = nID;

			int32_t nCookiceLen = (int32_t)sizeof(gate_forward_cookice);
			
			message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0] + nCookiceLen);

			nCookiceLen += (int32_t)sizeof(message_header);
			
			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID(pMessage->GetTypeName());

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_GATE_FORWARD, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			CCoreService* pToCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(nToServiceID);
			DebugAstEx(pToCoreService != nullptr, false);

			CProtobufFactory* pProtobufFactory = pToCoreService->getServiceBase()->getForwardProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			google::protobuf::Message* pNewMessage = pProtobufFactory->clone_protobuf_message(pMessage);
			if (nullptr == pNewMessage)
				return false;

			char* szBuf = new char[sizeof(SMCT_GATE_FORWARD)];
			SMCT_GATE_FORWARD* pContext = reinterpret_cast<SMCT_GATE_FORWARD*>(szBuf);
			pContext->nSessionID = sClientSessionInfo.nSessionID;
			pContext->nFromServiceID = sClientSessionInfo.nGateServiceID;
			pContext->nToServiceID = nToServiceID;
			pContext->nToActorID = 0;
			if (eType == eMTT_Actor)
				pContext->nToActorID = nID;
			pContext->pMessage = pNewMessage;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_GATE_FORWARD;
			sMessagePacket.nDataSize = sizeof(SMCT_GATE_FORWARD);
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
			CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(nToServiceID);
			DebugAstEx(pCoreService != nullptr, false);

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getForwardProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			const std::string& szMessageName = pCoreService->getForwardMessageName(pData->nMessageID);
			if (szMessageName.empty())
			{
				PrintWarning("CTransporter::gate_forward error szMessageName.empty() service_id: {} actor_id: {} message_id: {}", nToServiceID, nToActorID, pData->nMessageID);
				return false;
			}
			google::protobuf::Message* pMessage = pProtobufFactory->unserialize_protobuf_message_from_buf(szMessageName, pData + 1, pData->nMessageSize - sizeof(message_header));
			if (nullptr == pMessage)
			{
				PrintWarning("CTransporter::gate_forward error nullptr == pNewMessage service_id: {} actor_id: {} message_name: {}", nToServiceID, nToActorID, szMessageName);
				return false;
			}

			char* szBuf = new char[sizeof(SMCT_GATE_FORWARD)];
			SMCT_GATE_FORWARD* pContext = reinterpret_cast<SMCT_GATE_FORWARD*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nFromServiceID = nFromServiceID;
			pContext->nToServiceID = nToServiceID;
			pContext->nToActorID = nToActorID;
			pContext->pMessage = pMessage;

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_GATE_FORWARD;
			sMessagePacket.nDataSize = sizeof(SMCT_GATE_FORWARD);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	bool CTransporter::invoke_a(CCoreService* pCoreService, uint64_t nSessionID, uint64_t nFromActorID, EMessageTargetType eToType, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pCoreService != nullptr, false);

		if (eToType == eMTT_Actor)
		{
			CCoreActor* pToCoreActor = pCoreService->getActorScheduler()->getCoreActor(nToID);
			if (pToCoreActor != nullptr)
			{
				CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getServiceProtobufFactory();
				DebugAstEx(pProtobufFactory != nullptr, false);

				google::protobuf::Message* pNewMessage = pProtobufFactory->clone_protobuf_message(pMessage);
				if (nullptr == pNewMessage)
					return false;

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = nFromActorID;
				sActorMessagePacket.nFromServiceID = pCoreService->getServiceID();
				sActorMessagePacket.nSessionID = nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.pMessage = pNewMessage;

				pToCoreActor->getChannel()->send(sActorMessagePacket);

				pCoreService->getActorScheduler()->addWorkCoreActor(pToCoreActor);

				return true;
			}
			else
			{
				return this->invoke(pCoreService, nSessionID, eMTT_Actor, nFromActorID, eMTT_Actor, nToID, pMessage);
			}
		}
		else
		{
			return this->invoke(pCoreService, nSessionID, eMTT_Actor, nFromActorID, eMTT_Service, nToID, pMessage);
		}
	}

	bool CTransporter::send(CCoreService* pCoreService, uint64_t nSessionID, uint32_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr && pCoreService != nullptr, false);

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getForwardProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			gate_send_cookice* pCookice = reinterpret_cast<gate_send_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nToServiceID = nToServiceID;

			int32_t nCookiceLen = (int32_t)sizeof(gate_send_cookice);

			message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0] + nCookiceLen);

			nCookiceLen += (int32_t)sizeof(message_header);

			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID(pMessage->GetTypeName());

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_TO_GATE, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			CCoreService* pToCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(nToServiceID);
			DebugAstEx(pToCoreService != nullptr, false);

			CProtobufFactory* pProtobufFactory = pToCoreService->getServiceBase()->getForwardProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + sizeof(message_header), (uint32_t)(this->m_szBuf.size() - sizeof(message_header)));
			if (nDataSize < 0)
				return false;

			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID(pMessage->GetTypeName());

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

	bool CTransporter::broadcast(CCoreService* pCoreService, const std::vector<uint64_t>& vecSessionID, uint32_t nToServiceID, const google::protobuf::Message* pMessage)
	{
		if (vecSessionID.empty())
			return true;

		DebugAstEx(pMessage != nullptr && pCoreService != nullptr, false);

		if (!CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			CProtobufFactory* pProtobufFactory = pCoreService->getServiceBase()->getForwardProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(&this->m_szBuf[0]);
			pCookice->nToServiceID = nToServiceID;
			pCookice->nSessionCount = (uint16_t)vecSessionID.size();

			int32_t nCookiceLen = (int32_t)sizeof(gate_broadcast_cookice);
			memcpy(&this->m_szBuf[0] + nCookiceLen, &vecSessionID[0], sizeof(uint64_t) * vecSessionID.size());
			nCookiceLen += (int32_t)(sizeof(uint64_t) *  vecSessionID.size());

			message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0] + nCookiceLen);

			nCookiceLen += (int32_t)sizeof(message_header);

			DebugAstEx(nCookiceLen < (int32_t)this->m_szBuf.size(), false);

			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
			pHeader->nMessageID = _GET_MESSAGE_ID(pMessage->GetTypeName());

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_TO_GATE_BROADCAST, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			CCoreService* pToCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(nToServiceID);
			DebugAstEx(pToCoreService != nullptr, false);

			CProtobufFactory* pProtobufFactory = pToCoreService->getServiceBase()->getForwardProtobufFactory();
			DebugAstEx(pProtobufFactory != nullptr, false);

			int32_t nSessionLen = (int32_t)(sizeof(uint64_t) * vecSessionID.size());

			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0], (uint32_t)(this->m_szBuf.size()));
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
			pHeader->nMessageID = _GET_MESSAGE_ID(pMessage->GetTypeName());
			memcpy(pContext->pData + nSessionLen + sizeof(message_header), &this->m_szBuf[0], nDataSize);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_TO_GATE_BROADCAST;
			sMessagePacket.nDataSize = sizeof(SMCT_TO_GATE);
			sMessagePacket.pData = pContext;

			CCoreApp::Inst()->getLogicRunnable()->sendInsideMessage(sMessagePacket);
		}

		return true;
	}

	void CTransporter::onRequestMessageTimeout(uint64_t nContext)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nContext);
		if (iter == this->m_mapPendingResponseInfo.end())
		{
			PrintWarning("iter == this->m_mapProtoBufResponseInfo.end() session_id: {}", nContext);
			return;
		}

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		this->m_mapPendingResponseInfo.erase(iter);
		if (nullptr == pPendingResponseInfo)
		{
			PrintWarning("nullptr == pPendingResponseInfo session_id: {}", nContext);
			return;
		}

		defer([&]() 
		{
			SAFE_DELETE(pPendingResponseInfo);
		});
		
		if (pPendingResponseInfo->nCoroutineID != 0)
		{
			SSyncCallResultInfo* pSyncCallResultInfo = new SSyncCallResultInfo();
			pSyncCallResultInfo->nResult = eRRT_TIME_OUT;
			pSyncCallResultInfo->pMessage = nullptr;
			coroutine::setLocalData(pPendingResponseInfo->nCoroutineID, "response", reinterpret_cast<uint64_t>(pSyncCallResultInfo));

			coroutine::resume(pPendingResponseInfo->nCoroutineID, 0);
		}
		else
		{
			DebugAst(pPendingResponseInfo->callback != nullptr);

			pPendingResponseInfo->callback(nullptr, eRRT_TIME_OUT);
		}
	}

	SPendingResponseInfo* CTransporter::getPendingResponseInfo(uint64_t nSessionID)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		if (iter == this->m_mapPendingResponseInfo.end())
			return nullptr;

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		this->m_mapPendingResponseInfo.erase(iter);
		if (pPendingResponseInfo->nHolderID != 0)
		{
			auto iter = this->m_mapHolderSessionIDList.find(pPendingResponseInfo->nHolderID);
			if (iter != this->m_mapHolderSessionIDList.end())
			{
				std::list<uint64_t>& listSessionID = iter->second;
				listSessionID.erase(pPendingResponseInfo->iterHolder);
			}
		}
		return pPendingResponseInfo;
	}

	SPendingResponseInfo* CTransporter::addPendingResponseInfo(uint64_t nSessionID, uint64_t nCoroutineID, uint64_t nToID, const std::string& szMessageName, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, uint64_t nHolderID)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		DebugAstEx(iter == this->m_mapPendingResponseInfo.end(), nullptr);

		SPendingResponseInfo* pPendingResponseInfo = new SPendingResponseInfo();
		pPendingResponseInfo->callback = callback;
		pPendingResponseInfo->nSessionID = nSessionID;
		pPendingResponseInfo->nCoroutineID = nCoroutineID;
		pPendingResponseInfo->nHolderID = nHolderID;
		pPendingResponseInfo->nToID = nToID;
		pPendingResponseInfo->szMessageName = szMessageName;
		pPendingResponseInfo->nBeginTime = base::time_util::getGmtTime();
		pPendingResponseInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(CTicker::eTT_Service, 0, 0, &pPendingResponseInfo->tickTimeout, CCoreApp::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapPendingResponseInfo[pPendingResponseInfo->nSessionID] = pPendingResponseInfo;

		if (nHolderID != 0)
		{
			std::list<uint64_t>& listSessionID = this->m_mapHolderSessionIDList[nHolderID];
			listSessionID.push_back(nSessionID);
			pPendingResponseInfo->iterHolder = (--listSessionID.end());
			pPendingResponseInfo->nHolderID = nHolderID;
		}

		return pPendingResponseInfo;
	}

	void CTransporter::delPendingResponseInfo(uint64_t nHolderID)
	{
		auto iter = this->m_mapHolderSessionIDList.find(nHolderID);
		if (iter == this->m_mapHolderSessionIDList.end())
			return;

		std::list<uint64_t>& listSessionID = iter->second;
		for (auto iter = listSessionID.begin(); iter != listSessionID.end(); ++iter)
		{
			auto iterPendingResponseInfo = this->m_mapPendingResponseInfo.find(*iter);
			if (iterPendingResponseInfo == this->m_mapPendingResponseInfo.end())
				continue;

			SPendingResponseInfo* pPendingResponseInfo = iterPendingResponseInfo->second;
			this->m_mapPendingResponseInfo.erase(iterPendingResponseInfo);

			SAFE_DELETE(pPendingResponseInfo);
		}

		this->m_mapHolderSessionIDList.erase(iter);
	}
}