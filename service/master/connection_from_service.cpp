#include "stdafx.h"
#include "connection_from_service.h"
#include "master_app.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreServiceKit/proto_system.h"


CConnectionFromService::CConnectionFromService()
	: m_nServiceID(0)
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

uint16_t CConnectionFromService::getServiceID() const
{
	return this->m_nServiceID;
}

void CConnectionFromService::onConnect()
{
}

void CConnectionFromService::onDisconnect()
{
	CMasterApp::Inst()->getServiceMgr()->unregisterService(this->m_nServiceID);
}

bool CConnectionFromService::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAstEx(nMessageType == eMT_SYSTEM, true);

	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	DebugAstEx(nSize > sizeof(core::message_header), true);

	if (pHeader->nMessageID == eSMT_register_service_base_info)
	{
		core::smt_register_service_base_info netMsg;
		netMsg.unpack(pData, nSize);
		
		this->m_nServiceID = netMsg.sServiceBaseInfo.nID;
		if (!CMasterApp::Inst()->getServiceMgr()->registerService(this, netMsg.sServiceBaseInfo))
		{
			PrintWarning("dup service service_id: %d", this->m_nServiceID);
			this->m_nServiceID = 0;
			this->shutdown(true, "dup service connection");
			return true;
		}
	}
	else if (pHeader->nMessageID == eSMT_unregister_service_base_info)
	{
		core::smt_unregister_service_base_info netMsg;
		netMsg.unpack(pData, nSize);

		CMasterApp::Inst()->getServiceMgr()->unregisterService(this->m_nServiceID);
	}

	return true;
}