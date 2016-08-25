#include "stdafx.h"
#include "core_message_registry.h"
#include "proto_system.h"
#include "core_connection_to_master.h"
#include "core_service_kit_define.h"
#include "core_service_app_impl.h"

#include "libCoreCommon/base_app.h"

namespace core
{
	CCoreMessageRegistry::CCoreMessageRegistry()
	{

	}

	CCoreMessageRegistry::~CCoreMessageRegistry()
	{

	}

	bool CCoreMessageRegistry::init()
	{
		return true;
	}

	void CCoreMessageRegistry::registerCallback(uint16_t nMessageID, const std::function<bool(uint16_t, CMessage)>& callback)
	{
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			PrintWarning("dup message by register node callback message_id: %d", nMessageID);
			return;
		}
		this->m_mapNodeCallback[nMessageID] = callback;
	}

	void CCoreMessageRegistry::registerGateForwardCallback(uint16_t nMessageID, const std::function<bool(SClientSessionInfo, CMessage)>& callback)
	{
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message by register node gate forward callback message_id: %d", nMessageID);
			return;
		}
		this->m_mapGateForwardCallback[nMessageID] = callback;
	}

	std::function<bool(uint16_t, CMessage)>& CCoreMessageRegistry::getCallback(uint32_t nMessageID)
	{
		auto iter = this->m_mapNodeCallback.find(nMessageID);
		if (iter == this->m_mapNodeCallback.end())
		{
			static std::function<bool(uint16_t, CMessage)> callback;
			return callback;
		}

		return iter->second;
	}

	std::function<bool(SClientSessionInfo, CMessage)>& CCoreMessageRegistry::getGateForwardCallback(uint32_t nMessageID)
	{
		auto iter = this->m_mapGateForwardCallback.find(nMessageID);
		if (iter == this->m_mapGateForwardCallback.end())
		{
			static std::function<bool(SClientSessionInfo, CMessage)> callback;
			return callback;
		}

		return iter->second;
	}

	void CCoreMessageRegistry::onConnectToMaster()
	{
		CCoreConnectionToMaster* pCoreConnectionToMaster = CCoreServiceAppImpl::Inst()->getConnectionToMaster();
		DebugAst(nullptr != pCoreConnectionToMaster);
	}
}