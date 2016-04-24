#include "stdafx.h"
#include "service_mgr.h"
#include "master_app.h"
#include "connection_from_service.h"

#include "libCoreCommon/proto_system.h"
#include "libCoreCommon/base_connection_mgr.h"

#include "libBaseCommon/base_function.h"

CServiceMgr::CServiceMgr()
{

}

CServiceMgr::~CServiceMgr()
{

}

bool CServiceMgr::init()
{
	return true;
}

CConnectionFromService* CServiceMgr::getServiceConnection(const std::string& szName) const
{
	auto iter = this->m_mapServiceInfo.find(szName);
	if (iter == this->m_mapServiceInfo.end())
		return nullptr;

	return iter->second.pConnectionFromService;
}

bool CServiceMgr::registerService(CConnectionFromService* pConnectionFromService, const SServiceBaseInfo& sServiceBaseInfo)
{
	DebugAstEx(pConnectionFromService != nullptr, false);

	auto iter = this->m_mapServiceInfo.find(sServiceBaseInfo.szName);
	if (iter != this->m_mapServiceInfo.end())
		return false;

	// 初始化服务信息
	SServiceInfo sServiceInfo;
	sServiceInfo.pConnectionFromService = pConnectionFromService;
	sServiceInfo.sServiceBaseInfo = sServiceBaseInfo;

	this->m_mapServiceInfo[sServiceBaseInfo.szName] = sServiceInfo;

	// 将其他服务的信息同步给新的服务
	std::vector<core::CBaseConnection*> vecBaseConnection;
	CMasterApp::Inst()->getBaseConnectionMgr()->getBaseConnection(_GET_CLASS_NAME(CConnectionFromService), vecBaseConnection);
	for (size_t i = 0; i < vecBaseConnection.size(); ++i)
	{
		core::CBaseConnection* pBaseConnection = vecBaseConnection[i];
		if (nullptr == pBaseConnection)
			continue;

		CConnectionFromService* pOtherConnectionFromService = dynamic_cast<CConnectionFromService*>(pBaseConnection);
		if (nullptr == pOtherConnectionFromService || pOtherConnectionFromService == pConnectionFromService)
			continue;

		auto iter = this->m_mapServiceInfo.find(pOtherConnectionFromService->getServiceName());
		
		SServiceInfo& sOtherServiceInfo = iter->second;
		base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

		// 同步基本服务信息
		smt_sync_service_base_info netMsg1;
		netMsg1.sServiceBaseInfo = sOtherServiceInfo.sServiceBaseInfo;

		netMsg1.pack(writeBuf);

		pConnectionFromService->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		// 同步消息基本信息
		writeBuf.clear();
		smt_sync_service_message_info netMsg2;
		netMsg2.szServiceName = sOtherServiceInfo.sServiceBaseInfo.szName;
		for (auto iter = sOtherServiceInfo.setServiceMessageName.begin(); iter != sOtherServiceInfo.setServiceMessageName.end(); ++iter)
		{
			SMessageSyncInfo sMessageSyncInfo;
			sMessageSyncInfo.szMessageName = *iter;
			netMsg2.vecMessageSyncInfo.push_back(sMessageSyncInfo);
		}

		netMsg2.pack(writeBuf);

		pConnectionFromService->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	// 把这个新加入的服务广播给其他服务
	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();
	
	smt_sync_service_base_info netMsg;
	netMsg.sServiceBaseInfo = sServiceBaseInfo;
	// 监听方跟主动连接方的接收缓存，发送缓存大小对调
	std::swap(netMsg.sServiceBaseInfo.nRecvBufSize, netMsg.sServiceBaseInfo.nSendBufSize);

	netMsg.pack(writeBuf);

	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(_GET_CLASS_NAME(CConnectionFromService), eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

	return true;
}

void CServiceMgr::unregisterService(const std::string& szServiceName)
{
	auto iter = this->m_mapServiceInfo.find(szServiceName);
	if (iter == this->m_mapServiceInfo.end())
		return;

	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

	smt_remove_service_base_info netMsg;
	netMsg.szName = szServiceName;

	netMsg.pack(writeBuf);

	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(_GET_CLASS_NAME(CConnectionFromService), eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

	// 清理该服务的消息
	SServiceInfo& sServiceInfo = iter->second;
	for (auto iter = sServiceInfo.setServiceMessageName.begin(); iter != sServiceInfo.setServiceMessageName.end(); ++iter)
	{
		uint32_t nMessageID = base::hash(iter->c_str());
		this->m_mapMessageName.erase(nMessageID);
	}
	this->m_mapServiceInfo.erase(iter);
}

void CServiceMgr::registerMessageInfo(const std::string& szServiceName, const std::vector<SMessageSyncInfo>& vecMessageSyncInfo)
{
	auto iter = this->m_mapServiceInfo.find(szServiceName);
	if (iter == this->m_mapServiceInfo.end())
	{
		PrintWarning("unknown service name by register message info service_name: %s", szServiceName.c_str());
		return;
	}

	SServiceInfo& sServiceInfo = iter->second;
	
	std::vector<SMessageSyncInfo> vecDeltaMessageInfo;
	for (size_t i = 0; i < vecMessageSyncInfo.size(); ++i)
	{
		const SMessageSyncInfo& sMessageSyncInfo = vecMessageSyncInfo[i];
		auto iter = sServiceInfo.setServiceMessageName.find(sMessageSyncInfo.szMessageName);
		if (iter == sServiceInfo.setServiceMessageName.end())
		{
			sServiceInfo.setServiceMessageName.insert(sMessageSyncInfo.szMessageName);
			vecDeltaMessageInfo.push_back(sMessageSyncInfo);
		}
	}

	for (size_t i = 0; i < vecDeltaMessageInfo.size(); ++i)
	{
		const SMessageSyncInfo& sMessageSyncInfo = vecDeltaMessageInfo[i];
		// 检测hash冲突
		uint32_t nMessageID = base::hash(sMessageSyncInfo.szMessageName.c_str());
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			PrintWarning("dup message name exist_message_name :%s exist_message_id: %d new_message_name: %s new_message_id: %d", sMessageSyncInfo.szMessageName.c_str(), nMessageID, iter->second.c_str(), nMessageID);
		}
		else
		{
			this->m_mapMessageName[nMessageID] = sMessageSyncInfo.szMessageName;
		}
	}

	if (!vecDeltaMessageInfo.empty())
	{
		// 把这个新消息信息广播给其他服务
		base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

		smt_sync_service_message_info netMsg;
		netMsg.szServiceName = szServiceName;
		netMsg.vecMessageSyncInfo = vecDeltaMessageInfo;
		netMsg.pack(writeBuf);

		CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(_GET_CLASS_NAME(CConnectionFromService), eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}
}
