#include "stdafx.h"
#include "connection_from_service.h"
#include "connection_from_client.h"
#include "gate_app.h"

#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\proto_system.h"

DEFINE_OBJECT(CConnectionFromService, 100)

CConnectionFromService::CConnectionFromService()
{

}

CConnectionFromService::~CConnectionFromService()
{

}

void CConnectionFromService::onConnect(const std::string& szContext)
{
	// 这里不需要告诉其他服务，网关服务的信息，因为是其他主动连网关服务的，所以其他服务知道网关服务信息
}

void CConnectionFromService::onDisconnect()
{
	CGateApp::Inst()->getServiceMgr()->delServiceConnection(this->m_szServiceName);
}

void CConnectionFromService::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
{
	// 其他服务通过网关服务转发消息给客户端
	if (nMsgType == eMT_GATE)
	{
		DebugAst(nSize > sizeof(gate_header));
		const gate_header* pHeader = reinterpret_cast<const gate_header*>(pData);
		CBaseConnection*  pBaseConnection = CGateApp::Inst()->getBaseConnectionMgr()->getBaseConnection(pHeader->nID);
		if (nullptr == pBaseConnection)
			return;

		CConnectionFromClient* pConnectionFromClient = dynamic_cast<CConnectionFromClient*>(pBaseConnection);
		DebugAst(nullptr != pConnectionFromClient);
		
		pConnectionFromClient->send(eMT_CLIENT, pHeader + 1, nSize - sizeof(gate_header));
	}
	else if (nMsgType == eMT_SYSTEM)
	{
		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
		DebugAst(nSize > sizeof(core::message_header));

		if (pHeader->nMsgID == eSMT_SyncServiceInfo)
		{
			const CSMT_SyncServiceInfo* pInfo = reinterpret_cast<const CSMT_SyncServiceInfo*>(pData);
			DebugAst(pInfo->nType == CSMT_SyncServiceInfo::eSync);

			this->m_szServiceName = pInfo->szName;
			// 这里对其他服务的监听地址不感兴趣，因为网关服务不可能去主动连接其他服务

			CGateApp::Inst()->getServiceMgr()->addServiceConnection(this);
		}
	}
}

const std::string& CConnectionFromService::getServiceName() const
{
	return this->m_szServiceName;
}