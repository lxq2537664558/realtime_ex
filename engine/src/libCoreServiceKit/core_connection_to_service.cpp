#include "stdafx.h"
#include "core_connection_to_service.h"
#include "proto_system.h"
#include "message_dispatcher.h"
#include "core_service_app_impl.h"
#include "core_service_proxy.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"

namespace core
{
	CCoreConnectionToService::CCoreConnectionToService()
		: m_nServiceID(0)
	{

	}

	CCoreConnectionToService::~CCoreConnectionToService()
	{

	}

	bool CCoreConnectionToService::init(const std::string& szContext)
	{
		uint32_t nServiceID = 0;
		if (!base::crt::atoui(szContext.c_str(), nServiceID))
			return false;

		this->m_nServiceID = (uint16_t)nServiceID;
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
		if (CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getServiceBaseInfo(this->getServiceID()) == nullptr)
		{
			PrintWarning("unknown service service_id: %d", this->getServiceID());
			this->shutdown(true, "unknown service");
			return;
		}

		// 同步服务名字
		smt_notify_service_base_info netMsg;
		netMsg.nFromServiceID = CCoreServiceAppImpl::Inst()->getServiceBaseInfo().nID;
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);

		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		if (!CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->addCoreConnectionToService(this))
		{
			this->shutdown(true, "dup service connection");
			return;
		}

		auto& funConnect = CCoreServiceAppImpl::Inst()->getServiceConnectCallback();
		if (funConnect != nullptr)
			funConnect(this->getServiceID());
	}

	void CCoreConnectionToService::onDisconnect()
	{
		if (!this->getServiceID() != 0)
		{
			CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->delCoreConnectionToService(this->getServiceID());
			auto& funDisconnect = CCoreServiceAppImpl::Inst()->getServiceDisconnectCallback();
			if (funDisconnect != nullptr)
				funDisconnect(this->getServiceID());
		}
	}

	bool CCoreConnectionToService::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType != eMT_SYSTEM)
		{
			return CMessageDispatcher::Inst()->dispatch(this->getServiceID(), nMessageType, pData, nSize);
		}

		return true;
	}

	uint16_t CCoreConnectionToService::getServiceID() const
	{
		return this->m_nServiceID;
	}
}