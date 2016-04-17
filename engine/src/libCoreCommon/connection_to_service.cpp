#include "stdafx.h"
#include "connection_to_service.h"
#include "core_app.h"
#include "base_connection_mgr.h"
#include "proto_system.h"
#include "base_app.h"
#include "message_dispatcher.h"

namespace core
{

	DEFINE_OBJECT(CConnectionToService, 100)

	CConnectionToService::CConnectionToService()
	{

	}

	CConnectionToService::~CConnectionToService()
	{

	}

	void CConnectionToService::onConnect(const std::string& szContext)
	{
		// szContext中存的是服务名字
		// 同步服务名字
		smt_notify_service_base_info netMsg;
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);

		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		this->m_szServiceName = szContext;

		CCoreApp::Inst()->getServiceMgr()->addConnectionToService(this);
	}

	void CConnectionToService::onDisconnect()
	{
		if (!this->m_szServiceName.empty())
			CCoreApp::Inst()->getServiceMgr()->delConnectionToService(this->m_szServiceName);
	}

	void CConnectionToService::onDispatch(uint16_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_SYSTEM)
		{
			// empty
		}
		else
		{
			CMessageDispatcher::Inst()->dispatch(this->getServiceName(), nMessageType, pData, nSize);
		}
	}

	const std::string& CConnectionToService::getServiceName() const
	{
		return this->m_szServiceName;
	}

}