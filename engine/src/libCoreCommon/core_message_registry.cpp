#include "stdafx.h"
#include "core_message_registry.h"
#include "proto_system.h"
#include "base_connection_to_master.h"
#include "core_app.h"

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

	void CCoreMessageRegistry::registerCallback(uint16_t nServiceID, const std::string& szMessageName, const std::function<void(SServiceSessionInfo, google::protobuf::Message*)>& callback)
	{
		SServiceRegistryInfo& sServiceRegistryInfo = this->m_mapServiceRegistryInfo[nServiceID];

		auto iter = sServiceRegistryInfo.mapServiceCallback.find(szMessageName);
		if (iter != sServiceRegistryInfo.mapServiceCallback.end())
		{
			PrintWarning("dup message callback service_id: %d message_name: %s", nServiceID, szMessageName.c_str());
			return;
		}
		sServiceRegistryInfo.mapServiceCallback[szMessageName] = callback;
	}

	void CCoreMessageRegistry::registerGateForwardCallback(uint16_t nServiceID, const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback)
	{
		SServiceRegistryInfo& sServiceRegistryInfo = this->m_mapServiceRegistryInfo[nServiceID];

		auto iter = sServiceRegistryInfo.mapGateForwardCallback.find(szMessageName);
		if (iter != sServiceRegistryInfo.mapGateForwardCallback.end())
		{
			PrintWarning("dup message gate forward service_id: %d message_name: %s", nServiceID, szMessageName.c_str());
			return;
		}
		sServiceRegistryInfo.mapGateForwardCallback[szMessageName] = callback;
	}

	std::function<void(SServiceSessionInfo, google::protobuf::Message*)>& CCoreMessageRegistry::getCallback(uint16_t nServiceID, const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceRegistryInfo.find(nServiceID);
		if (iter == this->m_mapServiceRegistryInfo.end())
		{
			static std::function<void(SServiceSessionInfo, google::protobuf::Message*)> callback;
			return callback;
		}

		auto iterCallback =  iter->second.mapServiceCallback.find(szMessageName);
		if (iterCallback == iter->second.mapServiceCallback.end())
		{
			static std::function<void(SServiceSessionInfo, google::protobuf::Message*)> callback;
			return callback;
		}

		return iterCallback->second;
	}

	std::function<void(SClientSessionInfo, google::protobuf::Message*)>& CCoreMessageRegistry::getGateForwardCallback(uint16_t nServiceID, const std::string& szMessageName)
	{
		auto iter = this->m_mapServiceRegistryInfo.find(nServiceID);
		if (iter == this->m_mapServiceRegistryInfo.end())
		{
			static std::function<void(SClientSessionInfo, google::protobuf::Message*)> callback;
			return callback;
		}

		auto iterCallback = iter->second.mapGateForwardCallback.find(szMessageName);
		if (iterCallback == iter->second.mapGateForwardCallback.end())
		{
			static std::function<void(SClientSessionInfo, google::protobuf::Message*)> callback;
			return callback;
		}

		return iterCallback->second;
	}

	void CCoreMessageRegistry::onConnectToMaster()
	{
		CBaseConnectionToMaster* pCoreConnectionToMaster = CCoreApp::Inst()->getConnectionToMaster();
		DebugAst(nullptr != pCoreConnectionToMaster);
	}
}