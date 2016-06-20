#include "stdafx.h"
#include "core_service_invoker.h"
#include "proto_system.h"
#include "core_connection_to_master.h"
#include "core_service_kit_define.h"
#include "core_service_kit_impl.h"

#include "libCoreCommon/base_app.h"

namespace core
{
	CCoreServiceInvoker::CCoreServiceInvoker()
	{

	}

	CCoreServiceInvoker::~CCoreServiceInvoker()
	{

	}

	bool CCoreServiceInvoker::init()
	{
		return true;
	}

	void CCoreServiceInvoker::registerCallback(uint16_t nMessageID, const ServiceCallback& callback)
	{
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message by register service callback message_id: %d", nMessageID);
			return;
		}
		this->m_mapServiceCallback[nMessageID] = callback;
	}

	void CCoreServiceInvoker::registerCallback(uint16_t nMessageID, const GateForwardCallback& callback)
	{
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message by register gate client callback message_id: %d", nMessageID);
			return;
		}
		this->m_mapGateClientCallback[nMessageID] = callback;
	}

	ServiceCallback& CCoreServiceInvoker::getCallback(uint32_t nMessageID)
	{
		auto iter = this->m_mapServiceCallback.find(nMessageID);
		if (iter == this->m_mapServiceCallback.end())
		{
			static ServiceCallback callback;
			return callback;
		}

		return iter->second;
	}

	GateForwardCallback& CCoreServiceInvoker::getGateClientCallback(uint32_t nMessageID)
	{
		auto iter = this->m_mapGateClientCallback.find(nMessageID);
		if (iter == this->m_mapGateClientCallback.end())
		{
			static GateForwardCallback callback;
			return callback;
		}

		return iter->second;
	}

	void CCoreServiceInvoker::onConnectToMaster()
	{
		CCoreConnectionToMaster* pCoreConnectionToMaster = CCoreServiceKitImpl::Inst()->getConnectionToMaster();
		DebugAst(nullptr != pCoreConnectionToMaster);
	}
}