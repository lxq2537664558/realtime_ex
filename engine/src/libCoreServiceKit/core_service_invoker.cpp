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

	void CCoreServiceInvoker::registerCallback(const std::string& szMessageName, const ServiceCallback& callback)
	{
		uint32_t nMessageID = base::hash(szMessageName.c_str());
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message name by register service callback message_id: %d exist_message_name: %s new_message_name: %s", nMessageID, iter->second.c_str(), szMessageName.c_str());
			return;
		}
		this->m_mapServiceCallback[nMessageID] = callback;
		this->m_mapMessageName[nMessageID] = szMessageName;
	}

	void CCoreServiceInvoker::registerCallback(const std::string& szMessageName, const GateForwardCallback& callback)
	{
		uint32_t nMessageID = base::hash(szMessageName.c_str());
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message name by register gate client callback message_id: %d exist_message_name: %s new_message_name: %s", nMessageID, iter->second.c_str(), szMessageName.c_str());
			return;
		}
		this->m_mapGateClientCallback[nMessageID] = callback;
		this->m_mapMessageName[nMessageID] = szMessageName;
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

	const std::string& CCoreServiceInvoker::getMessageName(uint32_t nMessageID) const
	{
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter == this->m_mapMessageName.end())
		{
			static std::string szMessageName;
			return szMessageName;
		}

		return iter->second;
	}
}