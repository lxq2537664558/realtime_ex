#include "stdafx.h"
#include "transporter.h"
#include "message_dispatcher.h"
#include "core_service_connection.h"
#include "protobuf_helper.h"
#include "core_service_kit_define.h"
#include "core_service_kit_impl.h"

#include "libBaseCommon/defer.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#define _MAX_CACHE_MESSAGE_SIZE			1024 * 1024 * 100

namespace core
{
	CTransporter::CTransporter()
		: m_nNextSessionID(0)
		, m_nCacheID(0)
	{
		this->m_tickCheckConnect.setCallback(std::bind(&CTransporter::onCheckConnect, this, std::placeholders::_1));
	}

	CTransporter::~CTransporter()
	{

	}

	bool CTransporter::init()
	{
		CBaseApp::Inst()->registerTicker(&this->m_tickCheckConnect, 5 * 1000, 5 * 1000, 0);

		return true;
	}

	uint64_t CTransporter::genSessionID()
	{
		++this->m_nNextSessionID;
		if (this->m_nNextSessionID == 0)
			this->m_nNextSessionID = 1;

		return this->m_nNextSessionID;
	}

	uint64_t CTransporter::genCacheID()
	{
		++this->m_nCacheID;
		if (this->m_nCacheID == 0)
			this->m_nCacheID = 1;

		return this->m_nCacheID;
	}

	void CTransporter::onCheckConnect(uint64_t nContext)
	{
		for (auto iter = this->m_mapMessageCacheInfo.begin(); iter != this->m_mapMessageCacheInfo.end(); ++iter)
		{
			SMessageCacheInfo& sMessageCacheInfo = iter->second;
			
			const SServiceBaseInfo* pServiceBaseInfo = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceBaseInfo(iter->first);
			IF_NOT(pServiceBaseInfo != nullptr)
				continue;

			if (sMessageCacheInfo.bRefuse)
			{
				CBaseApp::Inst()->getBaseConnectionMgr()->connect(pServiceBaseInfo->szHost, pServiceBaseInfo->nPort, eBCT_ConnectionService, pServiceBaseInfo->szName, pServiceBaseInfo->nSendBufSize, pServiceBaseInfo->nRecvBufSize);
				sMessageCacheInfo.bRefuse = false;
			}
			
			CCoreServiceConnection* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceConnection(pServiceBaseInfo->szName);
			if (pCoreConnectionToService != nullptr)
				this->sendCacheMessage(pServiceBaseInfo->szName);
		}
	}

	void CTransporter::onConnectRefuse(const std::string& szContext)
	{
		auto iter = this->m_mapMessageCacheInfo.find(szContext);
		if (iter == this->m_mapMessageCacheInfo.end())
			return;

		iter->second.bRefuse = true;
	}

	SMessageCacheInfo* CTransporter::getMessageCacheInfo(const std::string& szServiceName)
	{
		const SServiceBaseInfo* pServiceBaseInfo = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceBaseInfo(szServiceName);
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
			PrintWarning("cache message too big drop all cache message service_name: %s", szServiceName.c_str());

		return &sMessageCacheInfo;
	}

	bool CTransporter::call(const std::string& szServiceName, const SRequestMessageInfo& sRequestMessageInfo)
	{
		DebugAstEx(sRequestMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();
		
		CCoreServiceConnection* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceConnection(szServiceName);
		if (pCoreConnectionToService != nullptr)
		{
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

			// 填充cookice
			request_cookice cookice;
			cookice.nSessionID = nSessionID;
			cookice.nTraceID = nTraceID;
			
			pCoreConnectionToService->send(eMT_REQUEST, &cookice, sizeof(cookice), sRequestMessageInfo.pData, sRequestMessageInfo.pData->nMessageSize);

			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		uint64_t nCacheID = this->genCacheID();
		SRequestMessageCacheInfo* pRequestMessageCacheInfo = new SRequestMessageCacheInfo();
		pRequestMessageCacheInfo->nTraceID = nTraceID;
		pRequestMessageCacheInfo->nType = eMCIT_Request;
		pRequestMessageCacheInfo->vecBuf.resize(sRequestMessageInfo.pData->nMessageSize);
		memcpy(&pRequestMessageCacheInfo->vecBuf[0], sRequestMessageInfo.pData, sRequestMessageInfo.pData->nMessageSize);
		pRequestMessageCacheInfo->callback = sRequestMessageInfo.callback;
		pRequestMessageCacheInfo->tickTimeout.setCallback(std::bind(&CTransporter::onCacheMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pRequestMessageCacheInfo->tickTimeout, CCoreServiceKitImpl::Inst()->getInvokeTimeout(), 0, nCacheID);

		pMessageCacheInfo->mapMessageCacheInfo[nCacheID] = pRequestMessageCacheInfo;
		pMessageCacheInfo->nTotalSize += sRequestMessageInfo.pData->nMessageSize;

		this->onCheckConnect(0);

		return true;
	}

	bool CTransporter::response(const std::string& szServiceName, const SResponseMessageInfo& sResponseMessageInfo)
	{
		DebugAstEx(sResponseMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();
		
		CCoreServiceConnection* pCoreServiceConnection = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceConnection(szServiceName);
		if (pCoreServiceConnection == nullptr)
		{
			CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("pCoreServiceConnection == nullptr by response");
			return false;
		}

		response_cookice cookice;
		cookice.nTraceID = nTraceID;
		cookice.nSessionID = sResponseMessageInfo.nSessionID;
		cookice.nResult = sResponseMessageInfo.nResult;
		
		pCoreServiceConnection->send(eMT_RESPONSE, &cookice, sizeof(cookice), sResponseMessageInfo.pData, sResponseMessageInfo.pData->nMessageSize);

		return true;
	}

	bool CTransporter::forward(const std::string& szServiceName, const SGateForwardMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pData != nullptr, false);

		CCoreServiceKitImpl::Inst()->getInvokerTrace()->startNewTrace();
		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();

		CCoreServiceConnection* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceConnection(szServiceName);
		if (pCoreConnectionToService != nullptr)
		{
			// 填充cookice
			gate_cookice cookice;
			cookice.nSessionID = sGateMessageInfo.nSessionID;
			cookice.nTraceID = nTraceID;

			pCoreConnectionToService->send(eMT_GATE_FORWARD, &cookice, sizeof(cookice), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		uint64_t nCacheID = this->genCacheID();
		SGateForwardMessageCacheInfo* pGateForwardMessageCacheInfo = new SGateForwardMessageCacheInfo();
		pGateForwardMessageCacheInfo->nType = eMCIT_GateForward;
		pGateForwardMessageCacheInfo->nTraceID = nTraceID;
		pGateForwardMessageCacheInfo->vecBuf.resize(sGateMessageInfo.pData->nMessageSize);
		memcpy(&pGateForwardMessageCacheInfo->vecBuf[0], sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);
		pGateForwardMessageCacheInfo->nSessionID = sGateMessageInfo.nSessionID;
		pGateForwardMessageCacheInfo->tickTimeout.setCallback(std::bind(&CTransporter::onCacheMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pGateForwardMessageCacheInfo->tickTimeout, CCoreServiceKitImpl::Inst()->getInvokeTimeout(), 0, nCacheID);

		pMessageCacheInfo->mapMessageCacheInfo[nCacheID] = pGateForwardMessageCacheInfo;
		pMessageCacheInfo->nTotalSize += sGateMessageInfo.pData->nMessageSize;

		this->onCheckConnect(0);

		return true;
	}

	bool CTransporter::send(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo)
	{
		DebugAstEx(sGateMessageInfo.pData != nullptr, false);

		uint64_t nTraceID = CCoreServiceKitImpl::Inst()->getInvokerTrace()->getCurTraceID();
		
		CCoreServiceConnection* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceConnection(szServiceName);
		if (pCoreConnectionToService != nullptr)
		{
			// 填充cookice
			gate_cookice cookice;
			cookice.nSessionID = sGateMessageInfo.nSessionID;
			cookice.nTraceID = nTraceID;

			pCoreConnectionToService->send(eMT_TO_GATE, &cookice, sizeof(cookice), sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);

			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		uint64_t nCacheID = this->genCacheID();
		SGateMessageCacheInfo* pGateMessageCacheInfo = new SGateMessageCacheInfo();
		pGateMessageCacheInfo->nType = eMCIT_Gate;
		pGateMessageCacheInfo->nTraceID = nTraceID;
		pGateMessageCacheInfo->nSessionID = sGateMessageInfo.nSessionID;
		pGateMessageCacheInfo->vecBuf.resize(sGateMessageInfo.pData->nMessageSize);
		memcpy(&pGateMessageCacheInfo->vecBuf[0], sGateMessageInfo.pData, sGateMessageInfo.pData->nMessageSize);
		pGateMessageCacheInfo->tickTimeout.setCallback(std::bind(&CTransporter::onCacheMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pGateMessageCacheInfo->tickTimeout, CCoreServiceKitImpl::Inst()->getInvokeTimeout(), 0, nCacheID);

		pMessageCacheInfo->mapMessageCacheInfo[nCacheID] = pGateMessageCacheInfo;
		pMessageCacheInfo->nTotalSize += sGateMessageInfo.pData->nMessageSize;

		this->onCheckConnect(0);

		return true;
	}

	bool CTransporter::broadcast(const std::string& szServiceName, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo)
	{
		DebugAstEx(sGateBroadcastMessageInfo.pData != nullptr && !sGateBroadcastMessageInfo.vecSessionID.empty(), false);

		CCoreServiceConnection* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceConnection(szServiceName);
		if (pCoreConnectionToService != nullptr)
		{
			static char szBuf[4096] = { 0 };
			// 填充cookice
			gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(szBuf);
			pCookice->nCount = (uint16_t)sGateBroadcastMessageInfo.vecSessionID.size();
			memcpy(pCookice + 1, &sGateBroadcastMessageInfo.vecSessionID[0], sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size());

			pCoreConnectionToService->send(eMT_TO_GATE, pCookice, (uint16_t)(sizeof(uint64_t) * sGateBroadcastMessageInfo.vecSessionID.size()), sGateBroadcastMessageInfo.pData, sGateBroadcastMessageInfo.pData->nMessageSize);

			return true;
		}

		SMessageCacheInfo* pMessageCacheInfo = this->getMessageCacheInfo(szServiceName);

		uint64_t nCacheID = this->genCacheID();
		SGateBroadcastMessageCacheInfo* pGateBroadcastMessageCacheInfo = new SGateBroadcastMessageCacheInfo();
		pGateBroadcastMessageCacheInfo->nType = eMCIT_GateBroadcast;
		pGateBroadcastMessageCacheInfo->vecBuf.resize(sGateBroadcastMessageInfo.pData->nMessageSize);
		memcpy(&pGateBroadcastMessageCacheInfo->vecBuf[0], sGateBroadcastMessageInfo.pData, sGateBroadcastMessageInfo.pData->nMessageSize);
		pGateBroadcastMessageCacheInfo->vecSessionID = sGateBroadcastMessageInfo.vecSessionID;
		pGateBroadcastMessageCacheInfo->tickTimeout.setCallback(std::bind(&CTransporter::onCacheMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pGateBroadcastMessageCacheInfo->tickTimeout, CCoreServiceKitImpl::Inst()->getInvokeTimeout(), 0, nCacheID);

		pMessageCacheInfo->mapMessageCacheInfo[nCacheID] = pGateBroadcastMessageCacheInfo;
		pMessageCacheInfo->nTotalSize += sGateBroadcastMessageInfo.pData->nMessageSize;

		this->onCheckConnect(0);

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

		if (pResponseWaitInfo->callback)
			pResponseWaitInfo->callback(0, nullptr, eRRT_TIME_OUT);

		this->m_mapResponseWaitInfo.erase(iter);
		SAFE_DELETE(pResponseWaitInfo);
	}

	void CTransporter::onCacheMessageTimeout(uint64_t nContext)
	{
		for (auto iter = this->m_mapMessageCacheInfo.begin(); iter != this->m_mapMessageCacheInfo.end(); ++iter)
		{
			SMessageCacheInfo& sMessageCacheInfo = iter->second;
			auto iterCache = sMessageCacheInfo.mapMessageCacheInfo.find(nContext);
			if (iterCache == sMessageCacheInfo.mapMessageCacheInfo.end())
				continue;

			SMessageCacheHead* pMessageCacheHead = iterCache->second;
			if (pMessageCacheHead == nullptr)
			{
				PrintWarning("pMessageCacheHead == nullptr context: "UINT64FMT, nContext);
				sMessageCacheInfo.mapMessageCacheInfo.erase(iterCache);
				return;
			}

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo(pMessageCacheHead->nTraceID, "cache message timeout");
			
			SAFE_DELETE(pMessageCacheHead);
			sMessageCacheInfo.mapMessageCacheInfo.erase(iterCache);
			return;
		}

		DebugAst(false);
	}

	void CTransporter::sendCacheMessage(const std::string& szServiceName)
	{
		CCoreServiceConnection* pCoreConnectionToService = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getServiceConnection(szServiceName);
		DebugAst(pCoreConnectionToService != nullptr);

		// 把之前cache的消息发送出去
		auto iterCache = this->m_mapMessageCacheInfo.find(szServiceName);
		if (iterCache != this->m_mapMessageCacheInfo.end())
		{
			const std::string& szServiceName = iterCache->first;
			SMessageCacheInfo& sMessageCacheInfo = iterCache->second;
			for (auto iter = sMessageCacheInfo.mapMessageCacheInfo.begin(); iter != sMessageCacheInfo.mapMessageCacheInfo.end(); ++iter)
			{
				SMessageCacheHead* pMessageCacheHead = iter->second;
				if (NULL == pMessageCacheHead)
					continue;

				switch(pMessageCacheHead->nType)
				{
				case eMCIT_Request:
					{
						SRequestMessageCacheInfo* pRequestMessageCacheInfo = dynamic_cast<SRequestMessageCacheInfo*>(pMessageCacheHead);
						if (nullptr == pRequestMessageCacheInfo)
						{
							PrintWarning("send cache message error cache_type: %d context: "UINT64FMT" service_name: %s", pMessageCacheHead->nType, iter->first, szServiceName.c_str());
							continue;
						}
						uint64_t nSessionID = 0;
						if (pRequestMessageCacheInfo->callback != nullptr)
						{
							nSessionID = this->genSessionID();
							SResponseWaitInfo* pResponseWaitInfo = new SResponseWaitInfo();
							pResponseWaitInfo->callback = pRequestMessageCacheInfo->callback;
							pResponseWaitInfo->nSessionID = nSessionID;
							pResponseWaitInfo->nTraceID = pRequestMessageCacheInfo->nTraceID;
							pResponseWaitInfo->szServiceName = szServiceName;
							pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CTransporter::onRequestMessageTimeout, this, std::placeholders::_1));
							CBaseApp::Inst()->registerTicker(&pResponseWaitInfo->tickTimeout, pMessageCacheHead->tickTimeout.getRemainTime(), 0, nSessionID);

							this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;
						}
						// 填充cookice
						request_cookice cookice;
						cookice.nSessionID = nSessionID;
						cookice.nTraceID = pRequestMessageCacheInfo->nTraceID;
						
						pCoreConnectionToService->send(eMT_REQUEST, &cookice, sizeof(cookice), &pRequestMessageCacheInfo->vecBuf[0], (uint16_t)pRequestMessageCacheInfo->vecBuf.size());
					}
					break;

					case eMCIT_Gate:
					{
						SGateMessageCacheInfo* pGateMessageCacheInfo = dynamic_cast<SGateMessageCacheInfo*>(pMessageCacheHead);
						if (NULL == pGateMessageCacheInfo)
						{
							PrintWarning("send cache message error cache_type: %d context: "UINT64FMT" service_name: %s", pMessageCacheHead->nType, iter->first, szServiceName.c_str());
							continue;
						}

						// 填充cookice
						gate_cookice cookice;
						cookice.nSessionID = pGateMessageCacheInfo->nSessionID;
						cookice.nTraceID = pGateMessageCacheInfo->nTraceID;
						
						pCoreConnectionToService->send(eMT_TO_GATE, &cookice, sizeof(cookice), &pGateMessageCacheInfo->vecBuf[0], (uint16_t)pGateMessageCacheInfo->vecBuf.size());
					}
					break;

					case eMCIT_GateBroadcast:
					{
						SGateBroadcastMessageCacheInfo* pGateBroadcastMessageCacheInfo = dynamic_cast<SGateBroadcastMessageCacheInfo*>(pMessageCacheHead);
						if (NULL == pGateBroadcastMessageCacheInfo)
						{
							PrintWarning("send cache message error cache_type: %d context: "UINT64FMT" service_name: %s", pMessageCacheHead->nType, iter->first, szServiceName.c_str());
							continue;
						}

						// 填充cookice
						gate_broadcast_cookice* pCookice = reinterpret_cast<gate_broadcast_cookice*>(new char[sizeof(gate_broadcast_cookice) + sizeof(uint64_t) * pGateBroadcastMessageCacheInfo->vecSessionID.size()]);
						pCookice->nCount = (uint16_t)pGateBroadcastMessageCacheInfo->vecSessionID.size();
						memcpy(pCookice + 1, &pGateBroadcastMessageCacheInfo->vecSessionID[0], sizeof(uint64_t) * pGateBroadcastMessageCacheInfo->vecSessionID.size());

						pCoreConnectionToService->send(eMT_TO_GATE, pCookice, (uint16_t)(sizeof(uint64_t) * pGateBroadcastMessageCacheInfo->vecSessionID.size()), &pGateBroadcastMessageCacheInfo->vecBuf[0], (uint16_t)pGateBroadcastMessageCacheInfo->vecBuf.size());
					}
					break;

					case eMCIT_GateForward:
					{
						SGateForwardMessageCacheInfo* pGateForwardMessageCacheInfo = dynamic_cast<SGateForwardMessageCacheInfo*>(pMessageCacheHead);
						if (NULL == pGateForwardMessageCacheInfo)
						{
							PrintWarning("send cache message error cache_type: %d context: "UINT64FMT" service_name: %s", pMessageCacheHead->nType, iter->first, szServiceName.c_str());
							continue;
						}

						// 填充cookice
						gate_cookice cookice;
						cookice.nSessionID = pGateForwardMessageCacheInfo->nSessionID;
						cookice.nTraceID = pGateForwardMessageCacheInfo->nTraceID;
						
						pCoreConnectionToService->send(eMT_GATE_FORWARD, &cookice, sizeof(cookice), &pGateForwardMessageCacheInfo->vecBuf[0], (uint16_t)pGateForwardMessageCacheInfo->vecBuf.size());
					}
					break;
				}

				SAFE_DELETE(pMessageCacheHead);
			}
			this->m_mapMessageCacheInfo.erase(iterCache);
		}
	}

	void CTransporter::delCacheMessage(const std::string& szServiceName)
	{
		this->m_mapMessageCacheInfo.erase(szServiceName);
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