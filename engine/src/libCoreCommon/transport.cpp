#include "stdafx.h"
#include "transport.h"
#include "core_app.h"
#include "message_dispatcher.h"
#include "connection_to_service.h"
#include "connection_from_service.h"

#include "libBaseCommon/defer.h"

#define _MAX_CACHE_MESSAGE_SIZE			1024 * 1024 * 100
#define _MAX_REQUEST_MESSAGE_TIMEOUT	10000

static int32_t serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, core::message_header* pHeader, uint32_t nSize)
{
	DebugAstEx(pMessage != nullptr, false);

	std::string szMessageData;
	if (!pMessage->SerializeToString(&szMessageData))
		return -1;

	if (szMessageData.size() > nSize)
		return -1;

	pHeader->nMessageID = base::hash(pMessage->GetTypeName().c_str());
	pHeader->nMessageSize = (uint16_t)(sizeof(core::message_header) + szMessageData.size());

	memcpy(pHeader + 1, szMessageData.c_str(), szMessageData.size());

	return (int32_t)(szMessageData.size() + sizeof(core::message_header));
}

namespace core
{
	CTransport::CTransport()
		: m_nNextSessionID(0)
	{
		this->m_tickCheckConnect.setCallback(std::bind(&CTransport::onCheckConnect, this, std::placeholders::_1));
	}

	CTransport::~CTransport()
	{

	}

	bool CTransport::init()
	{
		CCoreApp::Inst()->registerTicker(&this->m_tickCheckConnect, 5 * 1000, 5 * 1000, 0);

		this->m_vecBuf.resize(UINT16_MAX);

		return true;
	}

	uint64_t CTransport::genSessionID()
	{
		++this->m_nNextSessionID;
		if (this->m_nNextSessionID == 0)
			this->m_nNextSessionID = 1;

		return this->m_nNextSessionID;
	}

	void CTransport::onCheckConnect(uint64_t nContext)
	{
		for (auto iter : this->m_mapMessageCacheInfo)
		{
			SMessageCacheInfo& sRequestMessageGroupInfo = iter.second;
			if (sRequestMessageGroupInfo.bRefuse)
			{
				const SServiceBaseInfo* pServiceBaseInfo = CCoreApp::Inst()->getServiceMgr()->getServiceBaseInfo(iter.first);
				IF_NOT(pServiceBaseInfo != nullptr)
					continue;

				CCoreApp::Inst()->getCoreConnectionMgr()->connect(pServiceBaseInfo->szHost, pServiceBaseInfo->nPort, pServiceBaseInfo->szName, _GET_CLASS_ID(CConnectionToService), pServiceBaseInfo->nSendBufSize, pServiceBaseInfo->nRecvBufSize, nullptr);
				sRequestMessageGroupInfo.bRefuse = false;
			}
		}
	}

	void CTransport::onConnectRefuse(const std::string& szContext)
	{
		auto iter = this->m_mapMessageCacheInfo.find(szContext);
		if (iter == this->m_mapMessageCacheInfo.end())
			return;

		iter->second.bRefuse = true;
	}

	SMessageCacheInfo* CTransport::getMessageCacheInfo(const std::string& szServiceName)
	{
		const SServiceBaseInfo* pServiceBaseInfo = CCoreApp::Inst()->getServiceMgr()->getServiceBaseInfo(szServiceName);
		if (pServiceBaseInfo == nullptr)
		{
			PrintWarning("invalid service name service_name: %s", szServiceName.c_str());
			return nullptr;
		}

		if (this->m_mapMessageCacheInfo.find(szServiceName) == this->m_mapMessageCacheInfo.end())
		{
			// 填充cache信息，发起连接靠定时连接重试机制
			SMessageCacheInfo sProtoBufRequestMessageGroupInfo;
			sProtoBufRequestMessageGroupInfo.nTotalSize = 0;
			sProtoBufRequestMessageGroupInfo.bRefuse = true;
			this->m_mapMessageCacheInfo[szServiceName] = sProtoBufRequestMessageGroupInfo;
		}

		SMessageCacheInfo& sMessageCacheInfo = this->m_mapMessageCacheInfo[szServiceName];
		if (sMessageCacheInfo.nTotalSize >= _MAX_CACHE_MESSAGE_SIZE)
		{
			sMessageCacheInfo.vecRequestMessageCacheInfo.clear();
			PrintWarning("cache message too big drop all cache message service_name: %s", szServiceName.c_str());
		}

		return &sMessageCacheInfo;
	}

	bool CTransport::call(const std::string& szServiceName, const SRequestMessageInfo& sRequestMessageInfo)
	{
		DebugAstEx(sRequestMessageInfo.pMessage != nullptr, false);

		int32_t nBufSize = serialize_protobuf_message_to_buf(sRequestMessageInfo.pMessage, reinterpret_cast<message_header*>(&this->m_vecBuf[0]), (uint32_t)this->m_vecBuf.size());
		if (nBufSize < 0)
			return false;

		CConnectionToService* pConnectionToService = CCoreApp::Inst()->getServiceMgr()->getConnectionToService(szServiceName);
		if (pConnectionToService != nullptr)
		{
			uint64_t nSessionID = 0;
			if (sRequestMessageInfo.callback != nullptr)
				nSessionID = this->genSessionID();

			// 填充cookice
			request_cookice cookice;
			cookice.nSessionID = nSessionID;

			pConnectionToService->send(eMT_REQUEST, &cookice, sizeof(cookice), &this->m_vecBuf[0], (uint16_t)nBufSize);
			
			if (nSessionID != 0)
			{
				SResponseWaitInfo* pResponseInfo = new SResponseWaitInfo();
				pResponseInfo->callback = sRequestMessageInfo.callback;
				pResponseInfo->nSessionID = nSessionID;
				pResponseInfo->szServiceName = szServiceName;
				pResponseInfo->tickTimeout.setCallback(std::bind(&CTransport::onRequestMessageTimeout, this, std::placeholders::_1));
				CCoreApp::Inst()->registerTicker(&pResponseInfo->tickTimeout, _MAX_REQUEST_MESSAGE_TIMEOUT, 0, nSessionID);

				this->m_mapResponseWaitInfo[pResponseInfo->nSessionID] = pResponseInfo;
			}
			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		SRequestMessageCacheInfo* pRequestMessageCacheInfo = new SRequestMessageCacheInfo();
		pRequestMessageCacheInfo->vecBuf.resize(nBufSize);
		memcpy(&pRequestMessageCacheInfo->vecBuf[0], &this->m_vecBuf[0], nBufSize);
		pRequestMessageCacheInfo->callback = sRequestMessageInfo.callback;
		pMessageCacheInfo->vecRequestMessageCacheInfo.push_back(pRequestMessageCacheInfo);
		pMessageCacheInfo->nTotalSize += nBufSize;

		return true;
	}

	bool CTransport::response(const std::string& szServiceName, const SResponseMessageInfo& sResponseMessageInfo)
	{
		DebugAstEx(sResponseMessageInfo.pMessage != nullptr, false);

		CConnectionFromService* pConnectionFromService = CCoreApp::Inst()->getServiceMgr()->getConnectionFromService(szServiceName);
		DebugAstEx(pConnectionFromService != nullptr, false);

		response_cookice cookice;
		cookice.nSessionID = sResponseMessageInfo.nSessionID;
		cookice.nResult = sResponseMessageInfo.nResult;

		int32_t nBufSize = serialize_protobuf_message_to_buf(sResponseMessageInfo.pMessage, reinterpret_cast<message_header*>(&this->m_vecBuf[0]), (uint32_t)this->m_vecBuf.size());
		if (nBufSize < 0)
			return false;

		pConnectionFromService->send(eMT_RESPONSE, &cookice, sizeof(cookice), &this->m_vecBuf[0], (uint16_t)nBufSize);

		return true;
	}

	bool CTransport::send(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pMessage != nullptr, false);

		int32_t nBufSize = serialize_protobuf_message_to_buf(sGateMessageInfo.pMessage, reinterpret_cast<message_header*>(&this->m_vecBuf[0]), (uint32_t)this->m_vecBuf.size());
		if (nBufSize < 0)
			return false;

		CConnectionToService* pConnectionToService = CCoreApp::Inst()->getServiceMgr()->getConnectionToService(szServiceName);
		if (pConnectionToService != nullptr)
		{
			// 填充cookice
			gate_cookice cookice;
			cookice.nSessionID = sGateMessageInfo.nSessionID;

			pConnectionToService->send(eMT_TO_GATE, &cookice, sizeof(cookice), &this->m_vecBuf[0], (uint16_t)nBufSize);

			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		SGateMessageCacheInfo* pGateMessageCacheInfo = new SGateMessageCacheInfo();
		pGateMessageCacheInfo->vecBuf.resize(nBufSize);
		memcpy(&pGateMessageCacheInfo->vecBuf[0], &this->m_vecBuf[0], nBufSize);
		pGateMessageCacheInfo->nSessionID = sGateMessageInfo.nSessionID;
		pMessageCacheInfo->vecGateMessageCacheInfo.push_back(pGateMessageCacheInfo);
		pMessageCacheInfo->nTotalSize += nBufSize;

		return true;
	}

	bool CTransport::broadcast(const std::string& szServiceName, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo)
	{
		DebugAstEx(sGateBroadcastMessageInfo.pMessage != nullptr && !sGateBroadcastMessageInfo.vecSessionID.empty(), false);

		int32_t nBufSize = serialize_protobuf_message_to_buf(sGateBroadcastMessageInfo.pMessage, reinterpret_cast<message_header*>(&this->m_vecBuf[0]), (uint32_t)this->m_vecBuf.size());
		if (nBufSize < 0)
			return false;

		CConnectionToService* pConnectionToService = CCoreApp::Inst()->getServiceMgr()->getConnectionToService(szServiceName);
		if (pConnectionToService != nullptr)
		{
			// 填充cookice
			gate_cookice_broadcast* pCookice = reinterpret_cast<gate_cookice_broadcast*>(new char[sizeof(gate_cookice_broadcast) + sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size()]);
			pCookice->nCount = (uint16_t)sGateBroadcastMessageInfo.vecSessionID.size();
			memcpy(pCookice + 1, &sGateBroadcastMessageInfo.vecSessionID[0], sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size());

			pConnectionToService->send(eMT_TO_GATE | eMT_BROADCAST, pCookice, (uint16_t)(sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size()), &this->m_vecBuf[0], (uint16_t)nBufSize);

			return true;
		}
		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		SGateBroadcastMessageCacheInfo* sGateBroadcastMessageCacheInfo = new SGateBroadcastMessageCacheInfo();
		sGateBroadcastMessageCacheInfo->vecBuf.resize(nBufSize);
		memcpy(&sGateBroadcastMessageCacheInfo->vecBuf[0], &this->m_vecBuf[0], nBufSize);
		sGateBroadcastMessageCacheInfo->vecSessionID = sGateBroadcastMessageInfo.vecSessionID;
		pMessageCacheInfo->vecGateBroadcastMessageCacheInfo.push_back(sGateBroadcastMessageCacheInfo);
		pMessageCacheInfo->nTotalSize += nBufSize;

		return true;
	}

	void CTransport::onRequestMessageTimeout(uint64_t nContext)
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

		PrintWarning("wait response time out session_id: "UINT64FMT" service_name: %s", pResponseWaitInfo->nSessionID, pResponseWaitInfo->szServiceName.c_str());

		if (pResponseWaitInfo->callback)
			pResponseWaitInfo->callback(0, nullptr, eRRT_TIME_OUT);

		this->m_mapResponseWaitInfo.erase(iter);
		SAFE_DELETE(pResponseWaitInfo);
	}

	void CTransport::sendCacheMessage(const std::string& szServiceName)
	{
		CConnectionToService* pConnectionToService = CCoreApp::Inst()->getServiceMgr()->getConnectionToService(szServiceName);
		DebugAst(pConnectionToService != nullptr);

		// 把之前cache的消息发送出去
		auto iter = this->m_mapMessageCacheInfo.find(szServiceName);
		if (iter != this->m_mapMessageCacheInfo.end())
		{
			SMessageCacheInfo& sMessageCacheInfo = iter->second;
			for (size_t i = 0; i < sMessageCacheInfo.vecRequestMessageCacheInfo.size(); ++i)
			{
				SRequestMessageCacheInfo* pRequestMessageCacheInfo = sMessageCacheInfo.vecRequestMessageCacheInfo[i];
				if (NULL == pRequestMessageCacheInfo)
					continue;

				uint64_t nSessionID = 0;
				if (pRequestMessageCacheInfo->callback != nullptr)
					nSessionID = this->genSessionID();

				// 填充cookice
				request_cookice cookice;
				cookice.nSessionID = nSessionID;

				pConnectionToService->send(eMT_REQUEST, &cookice, sizeof(cookice), &pRequestMessageCacheInfo->vecBuf[0], (uint16_t)pRequestMessageCacheInfo->vecBuf.size());

				if (nSessionID != 0)
				{
					SResponseWaitInfo* pResponseInfo = new SResponseWaitInfo();
					pResponseInfo->callback = pRequestMessageCacheInfo->callback;
					pResponseInfo->nSessionID = nSessionID;
					pResponseInfo->szServiceName = szServiceName;
					pResponseInfo->tickTimeout.setCallback(std::bind(&CTransport::onRequestMessageTimeout, this, std::placeholders::_1));
					CCoreApp::Inst()->registerTicker(&pResponseInfo->tickTimeout, _MAX_REQUEST_MESSAGE_TIMEOUT, 0, nSessionID);

					this->m_mapResponseWaitInfo[pResponseInfo->nSessionID] = pResponseInfo;
				}

				SAFE_DELETE(pRequestMessageCacheInfo);
			}
			for (size_t i = 0; i < sMessageCacheInfo.vecGateMessageCacheInfo.size(); ++i)
			{
				SGateMessageCacheInfo* pGateMessageCacheInfo = sMessageCacheInfo.vecGateMessageCacheInfo[i];
				if (NULL == pGateMessageCacheInfo)
					continue;

				// 填充cookice
				gate_cookice cookice;
				cookice.nSessionID = pGateMessageCacheInfo->nSessionID;

				pConnectionToService->send(eMT_TO_GATE, &cookice, sizeof(cookice), &pGateMessageCacheInfo->vecBuf[0], (uint16_t)pGateMessageCacheInfo->vecBuf.size());

				SAFE_DELETE(pGateMessageCacheInfo);
			}
			for (size_t i = 0; i < sMessageCacheInfo.vecGateBroadcastMessageCacheInfo.size(); ++i)
			{
				SGateBroadcastMessageCacheInfo* pGateBroadcastMessageCacheInfo = sMessageCacheInfo.vecGateBroadcastMessageCacheInfo[i];
				if (NULL == pGateBroadcastMessageCacheInfo)
					continue;

				// 填充cookice
				gate_cookice_broadcast* pCookice = reinterpret_cast<gate_cookice_broadcast*>(new char[sizeof(gate_cookice_broadcast) + sizeof(uint64_t) * pGateBroadcastMessageCacheInfo->vecSessionID.size()]);
				pCookice->nCount = (uint16_t)pGateBroadcastMessageCacheInfo->vecSessionID.size();
				memcpy(pCookice + 1, &pGateBroadcastMessageCacheInfo->vecSessionID[0], sizeof(uint64_t) * pGateBroadcastMessageCacheInfo->vecSessionID.size());

				pConnectionToService->send(eMT_TO_GATE | eMT_BROADCAST, pCookice, (uint16_t)(sizeof(uint64_t) * pGateBroadcastMessageCacheInfo->vecSessionID.size()), &pGateBroadcastMessageCacheInfo->vecBuf[0], (uint16_t)pGateBroadcastMessageCacheInfo->vecBuf.size());
				
				SAFE_DELETE(pGateBroadcastMessageCacheInfo);
			}

			this->m_mapMessageCacheInfo.erase(iter);
		}
	}

	void CTransport::delCacheMessage(const std::string& szServiceName)
	{
		this->m_mapMessageCacheInfo.erase(szServiceName);
	}

	SServiceSessionInfo& CTransport::getServiceSessionInfo()
	{
		return this->m_sServiceSessionInfo;
	}

	SResponseWaitInfo* CTransport::getResponseWaitInfo(uint64_t nSessionID, bool bErase)
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