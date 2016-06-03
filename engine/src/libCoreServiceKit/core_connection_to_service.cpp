#include "stdafx.h"
#include "core_connection_to_service.h"
#include "proto_system.h"
#include "message_dispatcher.h"
#include "core_service_kit_impl.h"
#include "core_service_proxy.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"

namespace core
{
	CCoreConnectionToService::CCoreConnectionToService()
	{

	}

	CCoreConnectionToService::~CCoreConnectionToService()
	{

	}

	bool CCoreConnectionToService::init(const std::string& szContext)
	{
		this->m_szServiceName = szContext;

		return true;
	}

	uint32_t CCoreConnectionToService::getType() const
	{
		return eBCT_ConnectionToService;
	}

	void CCoreConnectionToService::release()
	{
		delete this;
	}

	void CCoreConnectionToService::onConnect()
	{
		// szContext中存的是服务名字
		if (CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getConnectionToService(this->getServiceName()) != nullptr)
		{
			PrintWarning("dup service service_name: %s", this->getServiceName().c_str());
			this->shutdown(true, "dup service connection");
			return;
		}

		// 同步服务名字
		smt_notify_service_base_info netMsg;
		netMsg.szKey = this->m_szServiceName;
		netMsg.szServiceName = CCoreServiceKitImpl::Inst()->getServiceBaseInfo().szName;
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);

		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		if (!CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->addConnectionToService(this))
		{
			this->shutdown(true, "dup service connection");
			return;
		}
	}

	void CCoreConnectionToService::onDisconnect()
	{
		if (!this->m_szServiceName.empty())
			CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->delConnectionToService(this->m_szServiceName);
	}

	void CCoreConnectionToService::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
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

	const std::string& CCoreConnectionToService::getServiceName() const
	{
		return this->m_szServiceName;
	}

}