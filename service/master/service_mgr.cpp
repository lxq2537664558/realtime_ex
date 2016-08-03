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


bool CServiceMgr::registerService(CConnectionFromService* pConnectionFromService, const core::SServiceBaseInfo& sServiceBaseInfo)
{
	auto iter = this->m_mapServiceInfo.find(sServiceBaseInfo.nID);
	if (iter != this->m_mapServiceInfo.end())
		return false;

	if (this->m_setServiceName.find(sServiceBaseInfo.szName) != this->m_setServiceName.end())
		return false;

	this->m_mapServiceInfo[sServiceBaseInfo.nID] = sServiceBaseInfo;
	this->m_setServiceName.insert(sServiceBaseInfo.szName);

	std::vector<uint64_t> vecSocketID;
	// 将其他服务的信息同步给新的服务
	std::vector<core::CBaseConnection*> vecBaseConnection = CMasterApp::Inst()->getBaseConnectionMgr()->getBaseConnection(eBCT_ConnectionFromService);
	for (size_t i = 0; i < vecBaseConnection.size(); ++i)
	{
		core::CBaseConnection* pBaseConnection = vecBaseConnection[i];
		if (nullptr == pBaseConnection)
			continue;

		CConnectionFromService* pOtherConnectionFromService = dynamic_cast<CConnectionFromService*>(pBaseConnection);
		if (nullptr == pOtherConnectionFromService || pOtherConnectionFromService == pConnectionFromService)
			continue;

		auto iter = this->m_mapServiceInfo.find(pOtherConnectionFromService->getServiceID());
		if (iter == this->m_mapServiceInfo.end())
			continue;

		core::SServiceBaseInfo& sOtherServiceBaseInfo = iter->second;
		base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

		// 同步基本服务信息
		core::smt_sync_service_base_info netMsg1;
		netMsg1.sServiceBaseInfo = sOtherServiceBaseInfo;

		netMsg1.pack(writeBuf);
		pConnectionFromService->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		vecSocketID.push_back(pOtherConnectionFromService->getID());
	}

	// 把这个新加入的服务广播给其他服务
	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();
	
	core::smt_sync_service_base_info netMsg;
	netMsg.sServiceBaseInfo = sServiceBaseInfo;
	// 监听方跟主动连接方的接收缓存，发送缓存大小对调
	std::swap(netMsg.sServiceBaseInfo.nRecvBufSize, netMsg.sServiceBaseInfo.nSendBufSize);

	netMsg.pack(writeBuf);

	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(vecSocketID, eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

	PrintInfo("register service service_id: %d service_name: %s local addr: %s %d remote addr: %s %d", sServiceBaseInfo.nID, sServiceBaseInfo.szName.c_str(), pConnectionFromService->getLocalAddr().szHost, pConnectionFromService->getLocalAddr().nPort, pConnectionFromService->getRemoteAddr().szHost, pConnectionFromService->getRemoteAddr().nPort);

	return true;
}

void CServiceMgr::unregisterService(uint16_t nServiceID)
{
	auto iter = this->m_mapServiceInfo.find(nServiceID);
	if (iter == this->m_mapServiceInfo.end())
		return;

	// 清理该服务的消息
	core::SServiceBaseInfo& sServiceBaseInfo = iter->second;

	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

	core::smt_remove_service_base_info netMsg;
	netMsg.nServiceID = nServiceID;

	netMsg.pack(writeBuf);

	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(eBCT_ConnectionFromService, eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), nullptr);

	this->m_setServiceName.erase(sServiceBaseInfo.szName);
	this->m_mapServiceInfo.erase(iter);

	PrintInfo("unregister service service_id: %d", nServiceID);
}