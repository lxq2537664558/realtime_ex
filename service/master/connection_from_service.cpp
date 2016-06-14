#include "stdafx.h"
#include "connection_from_service.h"
#include "master_app.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreServiceKit/proto_system.h"


CConnectionFromService::CConnectionFromService()
{
}

CConnectionFromService::~CConnectionFromService()
{

}


bool CConnectionFromService::init(const std::string& szContext)
{
	return true;
}

uint32_t CConnectionFromService::getType() const
{
	return eBCT_ConnectionFromService;
}

void CConnectionFromService::release()
{
	delete this;
}

const std::string& CConnectionFromService::getServiceName() const
{
	return this->m_szServiceName;
}

void CConnectionFromService::onConnect()
{
	
}

void CConnectionFromService::onDisconnect()
{
	CMasterApp::Inst()->getServiceMgr()->unregisterService(this->m_szServiceName);
}

void CConnectionFromService::onDispatch(uint8_t nMsgType, const void* pData, uint16_t nSize)
{
	DebugAst(nMsgType == eMT_SYSTEM);

	const core::inside_message_cookice* pCookice = reinterpret_cast<const core::inside_message_cookice*>(pData);
	DebugAst(nSize > sizeof(core::inside_message_cookice));

	if (pCookice->nMessageID == eSMT_register_service_base_info)
	{
		core::smt_register_service_base_info netMsg;
		netMsg.unpack(pData, nSize);
		
		this->m_szServiceName = netMsg.sServiceBaseInfo.szName;
		if (!CMasterApp::Inst()->getServiceMgr()->registerService(this, netMsg.sServiceBaseInfo))
		{
			PrintWarning("dup service service_name: %s", this->m_szServiceName.c_str());
			this->m_szServiceName.clear();
			this->shutdown(true, "dup service connection");
			return;
		}
	}
	else if (pCookice->nMessageID == eSMT_unregister_service_base_info)
	{
		core::smt_unregister_service_base_info netMsg;
		netMsg.unpack(pData, nSize);

		CMasterApp::Inst()->getServiceMgr()->unregisterService(netMsg.szName);
	}
}