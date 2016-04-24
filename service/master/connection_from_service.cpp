#include "stdafx.h"
#include "connection_from_service.h"
#include "master_app.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/proto_system.h"

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
	CMasterApp::Inst()->getServiceMgr()->unregisterService(this->m_szServiceName);
}

void CConnectionFromService::onDispatch(uint32_t nMsgType, const void* pData, uint16_t nSize)
{
	DebugAst(nMsgType == eMT_SYSTEM);

	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	DebugAst(nSize > sizeof(core::message_header));

	if (pHeader->nMessageID == eSMT_register_service_base_info)
	{
		smt_register_service_base_info netMsg;
		netMsg.unpack(pData, nSize);

		this->m_szServiceName = netMsg.sServiceBaseInfo.szName;
		
		if (!CMasterApp::Inst()->getServiceMgr()->registerService(this, netMsg.sServiceBaseInfo))
		{
			PrintWarning("dup service service_name: %s", this->m_szServiceName.c_str());
			this->shutdown(true, "dup service connection");
			return;
		}
	}
	else if (pHeader->nMessageID == eSMT_unregister_service_base_info)
	{
		smt_unregister_service_base_info netMsg;
		netMsg.unpack(pData, nSize);

		CMasterApp::Inst()->getServiceMgr()->unregisterService(netMsg.szName);
	}
	else if (pHeader->nMessageID == eSMT_register_service_message_info)
	{
		smt_register_service_message_info netMsg;
		netMsg.unpack(pData, nSize);

		CMasterApp::Inst()->getServiceMgr()->registerMessageInfo(this->m_szServiceName, netMsg.vecMessageSyncInfo);
	}
}