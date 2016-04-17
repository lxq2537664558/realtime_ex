#include "stdafx.h"
#include "connection_from_service.h"
#include "master_app.h"

#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\proto_system.h"

DEFINE_OBJECT(CConnectionFromService, 100)

CConnectionFromService::CConnectionFromService()
{
}

CConnectionFromService::~CConnectionFromService()
{

}

const std::string& CConnectionFromService::getServiceName() const
{
	return this->m_szServiceName;
}

void CConnectionFromService::onConnect(const std::string& szContext)
{
}

void CConnectionFromService::onDisconnect()
{
	CMasterApp::Inst()->getServiceMgr()->delService(this->m_szServiceName);
}

void CConnectionFromService::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
{
	DebugAst(nMsgType == eMT_SYSTEM);

	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	DebugAst(nSize > sizeof(core::message_header));

	if (pHeader->nMessageID == eSMT_sync_service_base_info)
	{
		smt_sync_service_base_info netMsg;
		netMsg.unpack(pData, nSize);

		this->m_szServiceName = netMsg.sServiceBaseInfo.szName;
		
		CMasterApp::Inst()->getServiceMgr()->addService(this, netMsg.sServiceBaseInfo);
	}
	else if (pHeader->nMessageID == eSMT_remove_service_base_info)
	{
		smt_remove_service_base_info netMsg;
		netMsg.unpack(pData, nSize);

		CMasterApp::Inst()->getServiceMgr()->delService(netMsg.szName);
	}
	else if (pHeader->nMessageID == eSMT_sync_service_message_info)
	{
		smt_sync_service_message_info netMsg;
		netMsg.unpack(pData, nSize);

		CMasterApp::Inst()->getServiceMgr()->addServiceMessageInfo(this->m_szServiceName, netMsg.vecMessageSyncInfo, !!netMsg.nAdd);
	}
}