#include "stdafx.h"
#include "service_mgr.h"
#include "master_app.h"
#include "connection_from_service.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libBaseCommon/base_function.h"
#include "libCoreServiceKit/proto_system.h"
#include "libCoreServiceKit/core_service_kit_define.h"

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

bool CServiceMgr::registerService(CConnectionFromService* pConnectionFromService, const core::SServiceBaseInfo& sServiceBaseInfo)
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
		if (iter == this->m_mapServiceInfo.end())
			continue;

		SServiceInfo& sOtherServiceInfo = iter->second;
		base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

		// 同步基本服务信息
		core::smt_sync_service_base_info netMsg1;
		netMsg1.sServiceBaseInfo = sOtherServiceInfo.sServiceBaseInfo;

		netMsg1.pack(writeBuf);

		pConnectionFromService->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		// 同步消息基本信息
		writeBuf.clear();
		core::smt_sync_service_message_info netMsg2;
		netMsg2.szServiceName = sOtherServiceInfo.sServiceBaseInfo.szName;
		for (auto iter = sOtherServiceInfo.mapMessageProxyName.begin(); iter != sOtherServiceInfo.mapMessageProxyName.end(); ++iter)
		{
			netMsg2.vecMessageProxyInfo.push_back(iter->second);
		}

		netMsg2.pack(writeBuf);

		pConnectionFromService->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	// 把这个新加入的服务广播给其他服务
	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();
	
	core::smt_sync_service_base_info netMsg;
	netMsg.sServiceBaseInfo = sServiceBaseInfo;
	// 监听方跟主动连接方的接收缓存，发送缓存大小对调
	std::swap(netMsg.sServiceBaseInfo.nRecvBufSize, netMsg.sServiceBaseInfo.nSendBufSize);

	netMsg.pack(writeBuf);

	std::vector<uint64_t> vecExcludeID;
	vecExcludeID.push_back(pConnectionFromService->getID());
	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(_GET_CLASS_NAME(CConnectionFromService), eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), &vecExcludeID);

	PrintInfo("register service service_name: %s", sServiceBaseInfo.szName.c_str());

	return true;
}

void CServiceMgr::unregisterService(const std::string& szServiceName)
{
	auto iter = this->m_mapServiceInfo.find(szServiceName);
	if (iter == this->m_mapServiceInfo.end())
		return;

	// 清理该服务的消息
	SServiceInfo& sServiceInfo = iter->second;
	for (auto iter = sServiceInfo.mapMessageProxyName.begin(); iter != sServiceInfo.mapMessageProxyName.end(); ++iter)
	{
		uint32_t nMessageID = _GET_MESSAGE_ID(iter->second.szMessageName);
		this->m_mapMessageName.erase(nMessageID);
	}

	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

	core::smt_remove_service_base_info netMsg;
	netMsg.szName = szServiceName;

	netMsg.pack(writeBuf);

	std::vector<uint64_t> vecExcludeID;
	if (sServiceInfo.pConnectionFromService != nullptr)
		vecExcludeID.push_back(sServiceInfo.pConnectionFromService->getID());
	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(_GET_CLASS_NAME(CConnectionFromService), eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), &vecExcludeID);

	this->m_mapServiceInfo.erase(iter);

	PrintInfo("unregister service service_name: %s", szServiceName.c_str());
}

void CServiceMgr::registerMessageInfo(const std::string& szServiceName, const std::vector<core::SMessageProxyInfo>& vecMessageProxyInfo)
{
	auto iter = this->m_mapServiceInfo.find(szServiceName);
	if (iter == this->m_mapServiceInfo.end())
	{
		PrintWarning("unknown service name by register message info service_name: %s", szServiceName.c_str());
		return;
	}

	SServiceInfo& sServiceInfo = iter->second;
	
	std::vector<core::SMessageProxyInfo> vecDeltaMessageProxyInfo;
	for (size_t i = 0; i < vecMessageProxyInfo.size(); ++i)
	{
		const core::SMessageProxyInfo& sMessageProxyInfo = vecMessageProxyInfo[i];
		auto iter = sServiceInfo.mapMessageProxyName.find(sMessageProxyInfo.szMessageName);
		if (iter == sServiceInfo.mapMessageProxyName.end())
		{
			sServiceInfo.mapMessageProxyName[sMessageProxyInfo.szMessageName] = sMessageProxyInfo;
			vecDeltaMessageProxyInfo.push_back(sMessageProxyInfo);
		}
	}

	for (size_t i = 0; i < vecDeltaMessageProxyInfo.size(); ++i)
	{
		const core::SMessageProxyInfo& sMessageProxyInfo = vecDeltaMessageProxyInfo[i];
		// 检测hash冲突
		uint32_t nMessageID = _GET_MESSAGE_ID(sMessageProxyInfo.szMessageName);
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end() && iter->second != sMessageProxyInfo.szMessageName)
		{
			PrintWarning("dup message name exist_message_name :%s exist_message_id: %d new_message_name: %s new_message_id: %d", sMessageProxyInfo.szMessageName.c_str(), nMessageID, iter->second.c_str(), nMessageID);
		}
		else
		{
			this->m_mapMessageName[nMessageID] = sMessageProxyInfo.szMessageName;
		}
	}

	if (!vecDeltaMessageProxyInfo.empty())
	{
		// 把这个新消息信息广播给其他服务
		base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

		core::smt_sync_service_message_info netMsg;
		netMsg.szServiceName = szServiceName;
		netMsg.vecMessageProxyInfo = vecDeltaMessageProxyInfo;
		netMsg.pack(writeBuf);

		std::vector<uint64_t> vecExcludeID;
		if (sServiceInfo.pConnectionFromService != nullptr)
			vecExcludeID.push_back(sServiceInfo.pConnectionFromService->getID());
		CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(_GET_CLASS_NAME(CConnectionFromService), eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), &vecExcludeID);
	}

	for (size_t i = 0; i < vecDeltaMessageProxyInfo.size(); ++i)
	{
		PrintInfo("register service message info service_name: %s message_name: %s", szServiceName.c_str(), vecDeltaMessageProxyInfo[i].szMessageName.c_str());
	}
}