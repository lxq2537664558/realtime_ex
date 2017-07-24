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

	uint64_t CTransporter::genSessionID()
	{
		++this->m_nNextSessionID;
		if (this->m_nNextSessionID == 0)
			this->m_nNextSessionID = 1;

		return this->m_nNextSessionID;
	}

	bool CTransporter::invoke(CServiceBaseImpl* pServiceBaseImpl, uint64_t nSessionID, EMessageTargetType eFromType, uint64_t nFromID, EMessageTargetType eToType, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr && pServiceBaseImpl != nullptr, false);

		CProtobufFactory* pProtobufFactory = pServiceBaseImpl->getProtobufFactory();
		DebugAstEx(pProtobufFactory != nullptr, false);
		
		uint32_t nToServiceID = 0;
		if (eToType == eMTT_Actor)
		{
			CActorIDConverter* pActorIDConverter = pServiceBaseImpl->getActorIDConverter();
			DebugAstEx(pActorIDConverter != nullptr, false);

			nToServiceID = pActorIDConverter->convertToServiceID(nToID);
			DebugAstEx(nToServiceID != 0, false);
		}
		else
		{
			nToServiceID = (uint32_t)nToID;
			CServiceIDConverter* pServiceIDConverter = pServiceBaseImpl->getServiceIDConverter();
			if (pServiceIDConverter != nullptr)
			{
				std::string szServiceType = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceType(nToServiceID);
				nToServiceID = pServiceIDConverter->convert(szServiceType, nToServiceID);
				DebugAstEx(nToServiceID != 0, false);
			}
		}

		if (!CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->isOwnerService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			
			std::string szMessageName = pMessage->GetTypeName();
			// Ìî³äcookice
			request_cookice* pCookice = reinterpret_cast<request_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nFromServiceID = pServiceBaseImpl->getServiceID();
			pCookice->nToServiceID = nToServiceID;
			pCookice->nFromActorID = 0;
			pCookice->nToActorID = 0;
			if (eFromType == eMTT_Actor)
				pCookice->nFromActorID = nFromID;
			if (eToType == eMTT_Actor)
				pCookice->nToActorID = nToID;
			pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
			base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

			int32_t nCookiceLen = (int32_t)(sizeof(request_cookice)+pCookice->nMessageNameLen);
			DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_REQUEST, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			google::protobuf::Message* pNewMessage = pProtobufFactory->clone_protobuf_message(pMessage);
			if (nullptr == pNewMessage)
				return false;

			char* szBuf = new char[sizeof(SMCT_REQUEST)];
			SMCT_REQUEST* pContext = reinterpret_cast<SMCT_REQUEST*>(szBuf);
			pContext->nSessionID = nSessionID;
			pContext->nFromServiceID = pServiceBaseImpl->getServiceID();
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

	bool CTransporter::response(CServiceBaseImpl* pServiceBaseImpl, uint32_t nToServiceID, uint64_t nToActorID, uint64_t nSessionID, uint8_t nResult, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr && pServiceBaseImpl != nullptr, false);
		
		CProtobufFactory* pProtobufFactory = pServiceBaseImpl->getProtobufFactory();
		DebugAstEx(pProtobufFactory != nullptr, false);

		CServiceIDConverter* pServiceIDConverter = pServiceBaseImpl->getServiceIDConverter();
		if (pServiceIDConverter != nullptr)
		{
			std::string szServiceType = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceType(nToServiceID);
			nToServiceID = pServiceIDConverter->convert(szServiceType, nToServiceID);
			DebugAstEx(nToServiceID != 0, false);
		}

		if (!CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->isOwnerService(nToServiceID))
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
			if (nullptr == pBaseConnectionOtherNode)
				return false;

			std::string szMessageName = pMessage->GetTypeName();

			response_cookice* pCookice = reinterpret_cast<response_cookice*>(&this->m_szBuf[0]);
			pCookice->nSessionID = nSessionID;
			pCookice->nToServiceID = nToServiceID;
			pCookice->nResult = nResult;
			pCookice->nToActorID = nToActorID;
			pCookice->nMessageNameLen = (uint16_t)szMessageName.size();
			base::crt::strcpy(pCookice->szMessageName, szMessageName.size() + 1, szMessageName.c_str());

			int32_t nCookiceLen = (int32_t)(sizeof(response_cookice)+pCookice->nMessageNameLen);
			DebugAstEx(nCookiceLen < this->m_szBuf.size(), false);

			int32_t nDataSize = pProtobufFactory->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
			if (nDataSize < 0)
				return false;

			nDataSize += nCookiceLen;

			pBaseConnectionOtherNode->send(eMT_RESPONSE, &this->m_szBuf[0], (uint16_t)nDataSize);
		}
		else
		{
			google::protobuf::Message* pNewMessage = pProtobufFactory->clone_protobuf_message(pMessage);
			if (nullptr == pNewMessage)
				return false;

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

	bool CTransporter::invoke_a(CServiceBaseImpl* pServiceBaseImpl, uint64_t nSessionID, uint64_t nFromActorID, EMessageTargetType eToType, uint64_t nToID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pServiceBaseImpl != nullptr, false);

		CProtobufFactory* pProtobufFactory = pServiceBaseImpl->getProtobufFactory();
		DebugAstEx(pProtobufFactory != nullptr, false);

		if (eToType == eMTT_Actor)
		{
			CActorBaseImpl* pToActorBaseImpl = pServiceBaseImpl->getActorScheduler()->getActorBase(nToID);
			if (pToActorBaseImpl != nullptr)
			{
				google::protobuf::Message* pNewMessage = pProtobufFactory->clone_protobuf_message(pMessage);
				if (nullptr == pNewMessage)
					return false;

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = nFromActorID;
				sActorMessagePacket.nFromServiceID = pServiceBaseImpl->getServiceID();
				sActorMessagePacket.nSessionID = nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.pMessage = pNewMessage;

				pToActorBaseImpl->getChannel()->send(sActorMessagePacket);

				pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pToActorBaseImpl);

				return true;
			}
			else
			{
				return this->invoke(pServiceBaseImpl, nSessionID, eMTT_Actor, nFromActorID, eMTT_Actor, nToID, pMessage);
			}
		}
		else
		{
			return this->invoke(pServiceBaseImpl, nSessionID, eMTT_Actor, nFromActorID, eMTT_Service, nToID, pMessage);
		}
	}

	bool CTransporter::send(uint64_t nSessionID, uint32_t nToServiceID, const void* pData, uint16_t nDataSize)
	{
		DebugAstEx(pData != nullptr, false);
		DebugAstEx(nDataSize + sizeof(gate_send_cookice) < this->m_szBuf.size(), false);

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;

		gate_send_cookice* pCookice = reinterpret_cast<gate_send_cookice*>(&this->m_szBuf[0]);
		pCookice->nSessionID = nSessionID;
		pCookice->nToServiceID = nToServiceID;
		
		memcpy(&this->m_szBuf[0] + sizeof(gate_send_cookice), pData, nDataSize);

		nDataSize += (uint16_t)sizeof(gate_send_cookice);

		pBaseConnectionOtherNode->send(eMT_TO_GATE, &this->m_szBuf[0], nDataSize);

		return true;
	}

	bool CTransporter::broadcast(const std::vector<uint64_t>& vecSessionID, uint32_t nToServiceID, const void* pData, uint16_t nDataSize)
	{
		DebugAstEx(pData != nullptr, false);
		DebugAstEx(nDataSize + sizeof(gate_send_cookice) + sizeof(uint64_t)*vecSessionID.size() < this->m_szBuf.size(), false);

		if (vecSessionID.empty())
			return true;

		CBaseConnectionOtherNode* pBaseConnectionOtherNode = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getBaseConnectionOtherNodeByServiceID(nToServiceID);
		if (nullptr == pBaseConnectionOtherNode)
			return false;

		gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(&this->m_szBuf[0]);
		pCookice->nToServiceID = nToServiceID;
		pCookice->nSessionCount = (uint16_t)vecSessionID.size();
		
		int32_t nCookiceLen = (int32_t)sizeof(gate_broadcast_cookice);
		memcpy(&this->m_szBuf[0] + nCookiceLen, &vecSessionID[0], sizeof(uint64_t) * vecSessionID.size());
		nCookiceLen += (int32_t)(sizeof(uint64_t)* vecSessionID.size());
		memcpy(&this->m_szBuf[0] + nCookiceLen, pData, nDataSize);
		nDataSize += (uint16_t)nCookiceLen;

		pBaseConnectionOtherNode->send(eMT_TO_GATE, &this->m_szBuf[0], nDataSize);

		return true;
	}

	void CTransporter::onRequestMessageTimeout(uint64_t nContext)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nContext);
		if (iter == this->m_mapPendingResponseInfo.end())
		{
			PrintWarning("iter == this->m_mapProtoBufResponseInfo.end() session_id: "UINT64FMT, nContext);
			return;
		}

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		if (nullptr == pPendingResponseInfo)
		{
			PrintWarning("nullptr == pPendingResponseInfo session_id: "UINT64FMT, nContext);
			return;
		}

		pPendingResponseInfo->callback(nullptr, eRRT_TIME_OUT);
		
		this->m_mapPendingResponseInfo.erase(iter);
		SAFE_DELETE(pPendingResponseInfo);
	}

	SPendingResponseInfo* CTransporter::getPendingResponseInfo(uint64_t nSessionID, bool bErase)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		if (iter == this->m_mapPendingResponseInfo.end())
			return nullptr;

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		if (bErase)
			this->m_mapPendingResponseInfo.erase(iter);

		return pPendingResponseInfo;
	}

	SPendingResponseInfo* CTransporter::addPendingResponseInfo(uint64_t nSessionID, uint64_t nToID, const std::string& szMessageName, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		DebugAstEx(iter == this->m_mapPendingResponseInfo.end(), nullptr);

		SPendingResponseInfo* pPendingResponseInfo = new SPendingResponseInfo();
		pPendingResponseInfo->callback = callback;
		pPendingResponseInfo->nSessionID = nSessionID;
		pPendingResponseInfo->nToID = nToID;
		pPendingResponseInfo->nCoroutineID = 0;
		pPendingResponseInfo->szMessageName = szMessageName;
		pPendingResponseInfo->nBeginTime = base::getGmtTime();
		pPendingResponseInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(CTicker::eTT_Service, 0, 0, &pPendingResponseInfo->tickTimeout, CCoreApp::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapPendingResponseInfo[pPendingResponseInfo->nSessionID] = pPendingResponseInfo;

		return pPendingResponseInfo;
	}
}