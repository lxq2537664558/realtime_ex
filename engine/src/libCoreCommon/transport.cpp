#include "stdafx.h"
#include "transport.h"
#include "core_app.h"
#include "message_dispatcher.h"
#include "connection_to_service.h"
#include "connection_from_service.h"

#include "libBaseCommon/defer.h"

#define _MAX_CACHE_MESSAGE_SIZE			1024 * 1024 * 100
#define _MAX_REQUEST_MESSAGE_TIMEOUT	10000

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
		CCoreApp::Inst()->registTicker(&this->m_tickCheckConnect, 5 * 1000, 5 * 1000, 0);

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
			sMessageCacheInfo.vecRequestMessageInfo.clear();
			PrintWarning("cache message too big drop all cache message service_name: %s", szServiceName.c_str());
		}

		return &sMessageCacheInfo;
	}

	bool CTransport::call(const std::string& szServiceName, const SRequestMessageInfo& sRequestMessageInfo)
	{
		DebugAstEx(sRequestMessageInfo.pData != nullptr, false);

		CConnectionToService* pConnectionToService = CCoreApp::Inst()->getServiceMgr()->getConnectionToService(szServiceName);
		if (pConnectionToService != nullptr)
		{
			uint64_t nSessionID = 0;
			if (sRequestMessageInfo.callback != nullptr)
				nSessionID = this->genSessionID();

			// 填充cookice
			request_cookice cookice;
			cookice.nSessionID = nSessionID;

			pConnectionToService->send(eMT_REQUEST | sRequestMessageInfo.nMessageFormat, &cookice, sizeof(cookice), sRequestMessageInfo.pData, sRequestMessageInfo.pData->nMessageSize);
			
			if (nSessionID != 0)
			{
				SResponseWaitInfo* pResponseInfo = new SResponseWaitInfo();
				pResponseInfo->callback = sRequestMessageInfo.callback;
				pResponseInfo->nSessionID = nSessionID;
				pResponseInfo->szServiceName = szServiceName;
				pResponseInfo->tickTimeout.setCallback(std::bind(&CTransport::onRequestMessageTimeout, this, std::placeholders::_1));
				CCoreApp::Inst()->registTicker(&pResponseInfo->tickTimeout, _MAX_REQUEST_MESSAGE_TIMEOUT, 0, nSessionID);

				this->m_mapResponseWaitInfo[pResponseInfo->nSessionID] = pResponseInfo;
			}
			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		pMessageCacheInfo->vecRequestMessageInfo.push_back(sRequestMessageInfo);
		pMessageCacheInfo->nTotalSize += (uint32_t)sRequestMessageInfo.pData->nMessageSize;

		// 需要把数据拷贝出来，不然等函数返回后数据指针就成为无效的野指针了
		SRequestMessageInfo& sDstRequestMessageInfo = pMessageCacheInfo->vecRequestMessageInfo[pMessageCacheInfo->vecRequestMessageInfo.size() - 1];
		sDstRequestMessageInfo.pData = reinterpret_cast<message_header*>(new char[sRequestMessageInfo.pData->nMessageSize]);
		memcpy(const_cast<message_header*>(sDstRequestMessageInfo.pData), sRequestMessageInfo.pData, sRequestMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransport::response(const std::string& szServiceName, const SResponseMessageInfo& sResponseMessageInfo)
	{
		CConnectionFromService* pConnectionFromService = CCoreApp::Inst()->getServiceMgr()->getConnectionFromService(szServiceName);
		DebugAstEx(pConnectionFromService != nullptr, false);

		response_cookice cookice;
		cookice.nSessionID = sResponseMessageInfo.nSessionID;
		cookice.nResult = sResponseMessageInfo.nResult;

		pConnectionFromService->send(eMT_RESPONSE | sResponseMessageInfo.nMessageFormat, &cookice, sizeof(cookice), sResponseMessageInfo.pData, sResponseMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransport::send(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo)
	{
		CConnectionToService* pConnectionToService = CCoreApp::Inst()->getServiceMgr()->getConnectionToService(szServiceName);
		if (pConnectionToService != nullptr)
		{
			// 填充cookice
			gate_cookice cookice;
			cookice.nSessionID = sGateMessageInfo.nSessionID;

			pConnectionToService->send(sGateMessageInfo.nMessageFormat, &cookice, sizeof(cookice), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		pMessageCacheInfo->vecGateMessageInfo.push_back(sGateMessageInfo);
		pMessageCacheInfo->nTotalSize += (uint32_t)sGateMessageInfo.pData->nMessageSize;

		// 需要把数据拷贝出来，不然等函数返回后数据指针就成为无效的野指针了
		SGateMessageInfo& sDstGateMessageInfo = pMessageCacheInfo->vecGateMessageInfo[pMessageCacheInfo->vecGateMessageInfo.size() - 1];
		sDstGateMessageInfo.pData = reinterpret_cast<message_header*>(new char[sGateMessageInfo.pData->nMessageSize]);
		memcpy(const_cast<message_header*>(sDstGateMessageInfo.pData), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransport::broadcast(const std::string& szServiceName, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo)
	{
		DebugAstEx(!sGateBroadcastMessageInfo.vecSessionID.empty(), false);

		CConnectionToService* pConnectionToService = CCoreApp::Inst()->getServiceMgr()->getConnectionToService(szServiceName);
		if (pConnectionToService != nullptr)
		{
			// 填充cookice
			gate_cookice_broadcast* pCookice = reinterpret_cast<gate_cookice_broadcast*>(new char[sizeof(gate_cookice_broadcast) + sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size()]);
			pCookice->nCount = (uint16_t)sGateBroadcastMessageInfo.vecSessionID.size();
			memcpy(pCookice + 1, &sGateBroadcastMessageInfo.vecSessionID[0], sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size());

			pConnectionToService->send(eMT_TO_GATE | sGateBroadcastMessageInfo.nMessageFormat, pCookice, (uint16_t)(sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size()), sGateBroadcastMessageInfo.pData, sGateBroadcastMessageInfo.pData->nMessageSize);

			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		pMessageCacheInfo->vecGateBroadcastMessageInfo.push_back(sGateBroadcastMessageInfo);
		pMessageCacheInfo->nTotalSize += (uint32_t)sGateBroadcastMessageInfo.pData->nMessageSize;

		// 需要把数据拷贝出来，不然等函数返回后数据指针就成为无效的野指针了
		SGateBroadcastMessageInfo& sDstGateBroadcastMessageInfo = pMessageCacheInfo->vecGateBroadcastMessageInfo[pMessageCacheInfo->vecGateBroadcastMessageInfo.size() - 1];
		sDstGateBroadcastMessageInfo.pData = reinterpret_cast<message_header*>(new char[sGateBroadcastMessageInfo.pData->nMessageSize]);
		memcpy(const_cast<message_header*>(sDstGateBroadcastMessageInfo.pData), sGateBroadcastMessageInfo.pData, sGateBroadcastMessageInfo.pData->nMessageSize);

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
		// 把之前cache的消息发送出去
		auto iter = this->m_mapMessageCacheInfo.find(szServiceName);
		if (iter != this->m_mapMessageCacheInfo.end())
		{
			SMessageCacheInfo& sMessageCacheInfo = iter->second;
			for (size_t i = 0; i < sMessageCacheInfo.vecRequestMessageInfo.size(); ++i)
			{
				this->call(szServiceName, sMessageCacheInfo.vecRequestMessageInfo[i]);
				SAFE_DELETE_ARRAY(sMessageCacheInfo.vecRequestMessageInfo[i].pData);
			}
			for (size_t i = 0; i < sMessageCacheInfo.vecGateMessageInfo.size(); ++i)
			{
				this->send(szServiceName, sMessageCacheInfo.vecGateMessageInfo[i]);
				SAFE_DELETE_ARRAY(sMessageCacheInfo.vecGateMessageInfo[i].pData);
			}
			for (size_t i = 0; i < sMessageCacheInfo.vecGateBroadcastMessageInfo.size(); ++i)
			{
				this->broadcast(szServiceName, sMessageCacheInfo.vecGateBroadcastMessageInfo[i]);
				SAFE_DELETE_ARRAY(sMessageCacheInfo.vecGateBroadcastMessageInfo[i].pData);
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