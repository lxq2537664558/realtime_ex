#include "stdafx.h"
#include "service_mgr.h"
#include "connection_to_service.h"
#include "connection_from_service.h"

#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\core_app.h"

#include <functional>

#define _MAX_CACHE_REQUEST_MESSAGE_SIZE 1024 * 1024 * 100

#pragma pack(push,1)
struct message_request
{
	uint32_t	nSessionID;
	uint16_t	nMessageNameLen;
};
struct message_response
{
	uint32_t	nSessionID;
	uint8_t		nResult;
	uint16_t	nMessageNameLen;
};
#pragma pack(pop)

static google::protobuf::Message* createMessage(const std::string& szMessageName)
{
	const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
	if (pDescriptor == nullptr)
		return nullptr;

	const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
	if (pProtoType == nullptr)
		return nullptr;

	return pProtoType->New();
}

int32_t serializeRequestToBuf(const core::SRequestMessageInfo& sRequestMessageInfo, char* szBuf, uint32_t nBufSize)
{
	DebugAstEx(szBuf != nullptr, -1);
	DebugAstEx(nBufSize > sRequestMessageInfo.szMessageName.size() + sRequestMessageInfo.szMessageData.size() + sizeof(message_request) + 1, -1);

	message_request* pMessageRequest = reinterpret_cast<message_request*>(szBuf);
	pMessageRequest->nSessionID = sRequestMessageInfo.nSessionID;
	pMessageRequest->nMessageNameLen = (uint16_t)(sRequestMessageInfo.szMessageName.size() + 1);
	base::crt::strncpy(reinterpret_cast<char*>(pMessageRequest + 1), _countof(szBuf) - sizeof(message_request), sRequestMessageInfo.szMessageName.c_str(), sRequestMessageInfo.szMessageName.size());
	void* pData = szBuf + sizeof(message_request) + pMessageRequest->nMessageNameLen;
	memcpy(pData, sRequestMessageInfo.szMessageData.c_str(), sRequestMessageInfo.szMessageData.size());

	return (int32_t)(sizeof(message_request) + sRequestMessageInfo.szMessageName.size() + sRequestMessageInfo.szMessageData.size() + 1);
}

int32_t serializeResponseToBuf(const core::SResponseMessageInfo& sResponseMessageInfo, char* szBuf, uint32_t nBufSize)
{
	DebugAstEx(szBuf != nullptr, -1);
	DebugAstEx(nBufSize > sResponseMessageInfo.szMessageName.size() + sResponseMessageInfo.szMessageData.size() + sizeof(message_response) + 1, -1);

	message_response* pMessageResponse = reinterpret_cast<message_response*>(szBuf);
	pMessageResponse->nSessionID = sResponseMessageInfo.nSessionID;
	pMessageResponse->nResult = sResponseMessageInfo.nResult;
	pMessageResponse->nMessageNameLen = (uint16_t)(sResponseMessageInfo.szMessageName.size() + 1);
	base::crt::strncpy(reinterpret_cast<char*>(pMessageResponse + 1), _countof(szBuf) - sizeof(message_request), sResponseMessageInfo.szMessageName.c_str(), sResponseMessageInfo.szMessageName.size());
	void* pData = szBuf + sizeof(message_response) + pMessageResponse->nMessageNameLen;
	memcpy(pData, sResponseMessageInfo.szMessageData.c_str(), sResponseMessageInfo.szMessageData.size());

	return (int32_t)(sizeof(message_response) + sResponseMessageInfo.szMessageName.size() + sResponseMessageInfo.szMessageData.size() + 1);
}

core::CServiceMgr* g_pServiceMgr = nullptr;

namespace core
{
	CServiceMgr::CServiceMgr()
		: m_nNextSessionID(1)
	{
		this->m_tickCheckConnect.setCallback(&CServiceMgr::onCheckConnect, this);
	}

	CServiceMgr::~CServiceMgr()
	{
		
	}

	CServiceMgr* CServiceMgr::Inst()
	{
		if (g_pServiceMgr == nullptr)
			g_pServiceMgr = new CServiceMgr();

		return g_pServiceMgr;
	}

	uint32_t CServiceMgr::getSessionID() const
	{
		return this->m_nNextSessionID;
	}

	void CServiceMgr::incSessionID()
	{
		++this->m_nNextSessionID;
		if (this->m_nNextSessionID == 0)
			this->m_nNextSessionID = 1;
	}

	bool CServiceMgr::init()
	{
		CCoreApp::Inst()->getBaseConnectionMgr()->setConnectRefuseCallback(std::bind(&CServiceMgr::onConnectRefuse, this, std::placeholders::_1));
		CCoreApp::Inst()->registTicker(&this->m_tickCheckConnect, 5 * 1000, 5 * 1000, 0);

		return true;
	}

	void CServiceMgr::onCheckConnect(uint64_t nContext)
	{
		for (auto iter = this->m_mapCacheMessageGroupInfo.begin(); iter != this->m_mapCacheMessageGroupInfo.end(); ++iter)
		{
			SRequestMessageGroupInfo& sRequestMessageGroupInfo = iter->second;
			if (sRequestMessageGroupInfo.bRefuse)
			{
				const SServiceBaseInfo* pServiceBaseInfo = this->getServiceBaseInfo(iter->first);
				IF_NOT(pServiceBaseInfo != nullptr)
					continue;

				CCoreApp::Inst()->getBaseConnectionMgr()->connect(pServiceBaseInfo->szHost, pServiceBaseInfo->nPort, pServiceBaseInfo->szName, _GET_CLASS_NAME(CConnectionToService), pServiceBaseInfo->nSendBufSize, pServiceBaseInfo->nRecvBufSize, nullptr);
				sRequestMessageGroupInfo.bRefuse = false;
			}
		}
	}

	void CServiceMgr::onConnectRefuse(const std::string& szContext)
	{
		auto iter = this->m_mapCacheMessageGroupInfo.find(szContext);
		if (iter == this->m_mapCacheMessageGroupInfo.end())
			return;

		iter->second.bRefuse = true;
	}

	void CServiceMgr::onDispatch(uint16_t nType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		if (nType == eMT_REQUEST)
		{
			const message_request* pMessageRequest = reinterpret_cast<const message_request*>(pData);
			DebugAst(pMessageRequest->nMessageNameLen < nSize && pMessageRequest->nMessageNameLen >= 1);
			DebugAst(nSize > sizeof(message_request) + pMessageRequest->nMessageNameLen);

			const char* szMessageName = reinterpret_cast<const char*>(pMessageRequest + 1);
			DebugAst(szMessageName[pMessageRequest->nMessageNameLen - 1] == 0);

			auto iter = this->m_mapMessageHandler.find(szMessageName);
			if (iter == this->m_mapMessageHandler.end())
				return;

			google::protobuf::Message* pMessage = createMessage(szMessageName);
			if (nullptr == pMessage)
				return;

			const void* pMessageData = reinterpret_cast<const char*>(pData) + sizeof(message_request) + pMessageRequest->nMessageNameLen;
			if (!pMessage->ParseFromArray(pMessageData, nSize - sizeof(message_request) + pMessageRequest->nMessageNameLen))
				return;

			this->m_nWorkSessionID = pMessageRequest->nSessionID;
			this->m_szWorkServiceName = szMessageName;
			auto& callback = iter->second;
			if (callback != nullptr)
				callback(pMessage);
			this->m_nWorkSessionID = 0;
			this->m_szWorkServiceName.clear();
		}
		else if (nType == eMT_RESPONSE)
		{
			const message_response* pMessageResponse = reinterpret_cast<const message_response*>(pData);
			DebugAst(pMessageResponse->nMessageNameLen < nSize && pMessageResponse->nMessageNameLen >= 1);
			DebugAst(nSize > sizeof(message_request) + pMessageResponse->nMessageNameLen);

			const char* szMessageName = reinterpret_cast<const char*>(pMessageResponse + 1);
			DebugAst(szMessageName[pMessageResponse->nMessageNameLen - 1] == 0);

			auto iter = this->m_mapResponseInfo.find(pMessageResponse->nSessionID);
			if (iter == this->m_mapResponseInfo.end())
				return;

			google::protobuf::Message* pMessage = createMessage(szMessageName);
			if (nullptr == pMessage)
				return;

			const void* pMessageData = reinterpret_cast<const char*>(pData) + sizeof(message_response) + pMessageResponse->nMessageNameLen;
			if (!pMessage->ParseFromArray(pMessageData, nSize - sizeof(message_response) + pMessageResponse->nMessageNameLen))
				return;

			SResponseInfo* pResponseInfo = iter->second;
			DebugAst(pResponseInfo != nullptr);

			if (pResponseInfo->callback)
				pResponseInfo->callback(pMessage, (EResponseResultType)pMessageResponse->nResult);

			this->m_mapResponseInfo.erase(iter);
			SAFE_DELETE(pResponseInfo);
		}
	}

	bool CServiceMgr::call(const std::string& szServiceName, SRequestMessageInfo& sRequestMessageInfo)
	{
		if (sRequestMessageInfo.callback != nullptr)
			sRequestMessageInfo.nSessionID = this->getSessionID();
		else
			sRequestMessageInfo.nSessionID = 0;

		auto iter = this->m_mapConnectionToService.find(szServiceName);
		if (iter != this->m_mapConnectionToService.end())
		{
			CConnectionToService* pConnectionToService = iter->second;
			DebugAstEx(pConnectionToService != nullptr, false);

			static char szBuf[65535] = { 0 };
			int32_t nSize = serializeRequestToBuf(sRequestMessageInfo, szBuf, _countof(szBuf));
			DebugAstEx(nSize > 0, false);

			pConnectionToService->send(eMT_REQUEST, szBuf, (uint16_t)nSize);
			
			this->checkResponseTimeout(sRequestMessageInfo.nSessionID);
		}
		else
		{
			const SServiceBaseInfo* pServiceBaseInfo = this->getServiceBaseInfo(szServiceName);
			DebugAstEx(pServiceBaseInfo != nullptr, false);

			if (this->m_mapCacheMessageGroupInfo.find(szServiceName) == this->m_mapCacheMessageGroupInfo.end())
			{
				CCoreApp::Inst()->getBaseConnectionMgr()->connect(pServiceBaseInfo->szHost, pServiceBaseInfo->nPort, pServiceBaseInfo->szName, _GET_CLASS_NAME(CConnectionToService), pServiceBaseInfo->nSendBufSize, pServiceBaseInfo->nRecvBufSize, nullptr);
				SRequestMessageGroupInfo sRequestMessageGroupInfo;
				sRequestMessageGroupInfo.nTotalSize = 0;
				sRequestMessageGroupInfo.bRefuse = false;
				this->m_mapCacheMessageGroupInfo[szServiceName] = sRequestMessageGroupInfo;
			}

			SRequestMessageGroupInfo& sRequestMessageGroupInfo = this->m_mapCacheMessageGroupInfo[szServiceName];
			if (sRequestMessageGroupInfo.nTotalSize >= _MAX_CACHE_REQUEST_MESSAGE_SIZE)
			{
				sRequestMessageGroupInfo.vecRequestMessageInfo.clear();
				PrintWarning("cache request message too big clear it service_name: %s", szServiceName.c_str());
			}

			sRequestMessageGroupInfo.vecRequestMessageInfo.push_back(sRequestMessageInfo);
			sRequestMessageGroupInfo.nTotalSize += (uint32_t)(sizeof(message_request) + sRequestMessageInfo.szMessageName.size() + sRequestMessageInfo.szMessageData.size() + 1);
		}

		if (sRequestMessageInfo.callback != nullptr)
		{
			SResponseInfo* pResponseInfo = new SResponseInfo();
			pResponseInfo->callback = sRequestMessageInfo.callback;
			pResponseInfo->nSessionID = sRequestMessageInfo.nSessionID;
			pResponseInfo->szServiceName = szServiceName;
			pResponseInfo->szMessageName = sRequestMessageInfo.szMessageName;
			pResponseInfo->tickTimeout.setCallback(&CServiceMgr::onTimeout, this);

			this->m_mapResponseInfo[pResponseInfo->nSessionID] = pResponseInfo;
			this->incSessionID();
		}
		return true;
	}

	bool CServiceMgr::response(SResponseMessageInfo& sResponseMessageInfo)
	{
		DebugAstEx(this->m_nWorkSessionID != 0, false);

		sResponseMessageInfo.nSessionID = this->m_nWorkSessionID;
		auto iter = this->m_mapConnectionFromService.find(this->m_szWorkServiceName);
		if (iter == this->m_mapConnectionFromService.end())
			return false;

		CConnectionFromService* pConnectionFromService = iter->second;
		DebugAstEx(pConnectionFromService != nullptr, false);

		static char szBuf[65535] = { 0 };
		int32_t nSize = serializeResponseToBuf(sResponseMessageInfo, szBuf, _countof(szBuf));
		DebugAstEx(nSize > 0, false);

		pConnectionFromService->send(eMT_RESPONSE, szBuf, (uint16_t)nSize);

		return true;
	}

	void CServiceMgr::checkResponseTimeout(uint32_t nSessionID)
	{
		if (0 == nSessionID)
			return;

		auto iter = this->m_mapResponseInfo.find(nSessionID);
		DebugAst(iter != this->m_mapResponseInfo.end());

		SResponseInfo* pResponseInfo = iter->second;
		DebugAst(nullptr != pResponseInfo);
		DebugAst(!pResponseInfo->tickTimeout.isRegist());

		CCoreApp::Inst()->registTicker(&pResponseInfo->tickTimeout, 10000, 0, nSessionID);
	}

	void CServiceMgr::onTimeout(uint64_t nContext)
	{
		uint32_t nSessionID = (uint32_t)nContext;
		auto iter = this->m_mapResponseInfo.find(nSessionID);
		if (iter == this->m_mapResponseInfo.end())
		{
			PrintWarning("iter == this->m_mapResponseInfo.end() session_id: %d", nSessionID);
			return;
		}

		SResponseInfo* pResponseInfo = iter->second;
		if (nullptr == pResponseInfo)
		{
			PrintWarning("nullptr == pResponseInfo session_id: %d", nSessionID);
			return;
		}

		PrintWarning("wait response time out session_id: %u server_name: %s message_name: %s", pResponseInfo->nSessionID, pResponseInfo->szServiceName.c_str(), pResponseInfo->szMessageName.c_str());

		if (pResponseInfo->callback)
			pResponseInfo->callback(nullptr, eRRT_TIME_OUT);

		this->m_mapResponseInfo.erase(iter);
		SAFE_DELETE(pResponseInfo);
	}

	CConnectionToService* CServiceMgr::getConnectionToService(const std::string& szName) const
	{
		auto iter = this->m_mapConnectionToService.find(szName);
		if (iter == this->m_mapConnectionToService.end())
			return nullptr;

		return iter->second;
	}

	void CServiceMgr::addConnectionToService(CConnectionToService* pConnectionToService)
	{
		DebugAst(pConnectionToService != nullptr);

		DebugAst(this->m_mapConnectionToService.find(pConnectionToService->getServiceName()) == this->m_mapConnectionToService.end());

		this->m_mapConnectionToService[pConnectionToService->getServiceName()] = pConnectionToService;

		// 把之前cache的消息发送出去
		auto iter = this->m_mapCacheMessageGroupInfo.find(pConnectionToService->getServiceName());
		if (iter == this->m_mapCacheMessageGroupInfo.end())
			return;

		SRequestMessageGroupInfo& sRequestMessageGroupInfo = iter->second;
		for (size_t i = 0; i < sRequestMessageGroupInfo.vecRequestMessageInfo.size(); ++i)
		{
			static char szBuf[65535] = { 0 };
			int32_t nSize = serializeRequestToBuf(sRequestMessageGroupInfo.vecRequestMessageInfo[i], szBuf, _countof(szBuf));
			IF_NOT(nSize > 0)
				continue;

			pConnectionToService->send(eMT_REQUEST, szBuf, (uint16_t)nSize);

			this->checkResponseTimeout(sRequestMessageGroupInfo.vecRequestMessageInfo[i].nSessionID);
		}
		this->m_mapCacheMessageGroupInfo.erase(iter);
	}

	void CServiceMgr::delConnectionToService(const std::string& szName)
	{
		auto iter = this->m_mapConnectionToService.find(szName);
		if (iter == this->m_mapConnectionToService.end())
			return;

		this->m_mapConnectionToService.erase(iter);
	}

	CConnectionFromService* CServiceMgr::getConnectionFromService(const std::string& szName) const
	{
		auto iter = this->m_mapConnectionFromService.find(szName);
		if (iter == this->m_mapConnectionFromService.end())
			return nullptr;

		return iter->second;
	}

	void CServiceMgr::addConnectionFromService(CConnectionFromService* pConnectionFromService)
	{
		DebugAst(pConnectionFromService != nullptr);

		auto iter = this->m_mapConnectionFromService.find(pConnectionFromService->getServiceName());
		DebugAst(iter == this->m_mapConnectionFromService.end());

		this->m_mapConnectionFromService[pConnectionFromService->getServiceName()] = pConnectionFromService;
	}

	void CServiceMgr::delConnectionFromService(const std::string& szName)
	{
		auto iter = this->m_mapConnectionFromService.find(szName);
		if (iter == this->m_mapConnectionFromService.end())
			return;

		this->m_mapConnectionFromService.erase(iter);
	}

	const SServiceBaseInfo* CServiceMgr::getServiceBaseInfo(const std::string& szName) const
	{
		auto iter = this->m_mapServiceBaseInfo.find(szName);
		if (iter == this->m_mapServiceBaseInfo.end())
			return nullptr;

		return &iter->second;
	}

	void CServiceMgr::getServiceName(const std::string& szType, std::vector<std::string>& vecServiceName) const
	{
		for (auto iter = this->m_mapServiceBaseInfo.begin(); iter != this->m_mapServiceBaseInfo.end(); ++iter)
		{
			if (iter->second.szType == szType)
				vecServiceName.push_back(iter->second.szName);
		}
	}

	void CServiceMgr::addServiceBaseInfo(const SServiceBaseInfo& sServiceBaseInfo)
	{
		DebugAst(sServiceBaseInfo.szName[0] != 0);
		DebugAst(this->m_mapServiceBaseInfo.find(sServiceBaseInfo.szName) == this->m_mapServiceBaseInfo.end());

		PrintInfo("add service name: %s", sServiceBaseInfo.szName);
		this->m_mapServiceBaseInfo[sServiceBaseInfo.szName] = sServiceBaseInfo;
	}

	void CServiceMgr::delServiceBaseInfo(const std::string& szName)
	{
		this->m_mapServiceBaseInfo.erase(szName);
		PrintInfo("del service name: %s", szName.c_str());

		this->m_mapCacheMessageGroupInfo.erase(szName);
	}

	void CServiceMgr::registMessageHandler(const std::string& szMessageName, std::function<void(const google::protobuf::Message*)>& callback)
	{
		DebugAst(callback != nullptr);

		this->m_mapMessageHandler[szMessageName] = callback;
	}
}