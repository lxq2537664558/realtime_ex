#include "stdafx.h"
#include "message_directory.h"
#include "proto_system.h"
#include "core_app.h"
#include "core_connection_to_master.h"

namespace core
{
	CMessageDirectory::CMessageDirectory()
	{

	}

	CMessageDirectory::~CMessageDirectory()
	{

	}

	bool CMessageDirectory::init()
	{
		return true;
	}

	void CMessageDirectory::registerCallback(const std::string& szMessageName, const ServiceCallback& callback)
	{
		uint32_t nMessageID = base::hash(szMessageName.c_str());
		auto iter = this->m_mapOwnerMessageName.find(nMessageID);
		if (iter != this->m_mapOwnerMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message name by register service callback message_id: %d exist_message_name: %s new_message_name: %s", nMessageID, iter->second.c_str(), szMessageName.c_str());
			return;
		}
		this->m_mapServiceCallback[nMessageID] = callback;
		this->m_mapOwnerMessageName[nMessageID] = szMessageName;

		this->sendMessageInfo(szMessageName);
	}

	void CMessageDirectory::registerCallback(const std::string& szMessageName, const GateClientCallback& callback)
	{
		uint32_t nMessageID = base::hash(szMessageName.c_str());
		auto iter = this->m_mapOwnerMessageName.find(nMessageID);
		if (iter != this->m_mapOwnerMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message name by register gate client callback message_id: %d exist_message_name: %s new_message_name: %s", nMessageID, iter->second.c_str(), szMessageName.c_str());
			return;
		}
		this->m_mapGateClientCallback[nMessageID] = callback;
		this->m_mapOwnerMessageName[nMessageID] = szMessageName;
		
		this->sendMessageInfo(szMessageName);
	}

	ServiceCallback& CMessageDirectory::getCallback(uint32_t nMessageID)
	{
		auto iter = this->m_mapServiceCallback.find(nMessageID);
		if (iter == this->m_mapServiceCallback.end())
		{
			static ServiceCallback callback;
			return callback;
		}

		return iter->second;
	}

	GateClientCallback& CMessageDirectory::getGateClientCallback(uint32_t nMessageID)
	{
		auto iter = this->m_mapGateClientCallback.find(nMessageID);
		if (iter == this->m_mapGateClientCallback.end())
		{
			static GateClientCallback callback;
			return callback;
		}

		return iter->second;
	}

	void CMessageDirectory::onConnectToMaster()
	{
		CCoreConnectionToMaster* pCoreConnectionToMaster = CCoreApp::Inst()->getServiceMgr()->getConnectionToMaster();
		DebugAst(nullptr != pCoreConnectionToMaster);

		smt_register_service_message_info netMsg;
		for (auto iter = this->m_mapOwnerMessageName.begin(); iter != this->m_mapOwnerMessageName.end(); ++iter)
		{
			const std::string& szMessageName = iter->second;
			SMessageSyncInfo sMessageSyncInfo;
			sMessageSyncInfo.szMessageName = szMessageName;
			netMsg.vecMessageSyncInfo.push_back(sMessageSyncInfo);
		}

		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		pCoreConnectionToMaster->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	void CMessageDirectory::sendMessageInfo(const std::string& szMessageName)
	{
		CCoreConnectionToMaster* pCoreConnectionToMaster = CCoreApp::Inst()->getServiceMgr()->getConnectionToMaster();
		if (nullptr == pCoreConnectionToMaster)
			return;

		smt_register_service_message_info netMsg;
		SMessageSyncInfo sMessageSyncInfo;
		sMessageSyncInfo.szMessageName = szMessageName;
		netMsg.vecMessageSyncInfo.push_back(sMessageSyncInfo);
		
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		pCoreConnectionToMaster->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	const std::set<std::string>& CMessageDirectory::getOtherServiceName(const std::string& szMessageName) const
	{
		auto iter = this->m_mapOtherMessageDirectoryByMessageName.find(szMessageName);
		if (iter == this->m_mapOtherMessageDirectoryByMessageName.end())
		{
			static std::set<std::string> s_setMessageName;
			return s_setMessageName;
		}

		return iter->second;
	}

	void CMessageDirectory::addGlobalBeforeFilter(const ServiceGlobalFilter& callback)
	{
		this->m_vecServiceGlobalBeforeFilter.push_back(callback);
	}

	void CMessageDirectory::addGlobalAfterFilter(const ServiceGlobalFilter& callback)
	{
		this->m_vecServiceGlobalAfterFilter.push_back(callback);
	}

	const std::vector<ServiceGlobalFilter>& CMessageDirectory::getGlobalBeforeFilter()
	{
		return this->m_vecServiceGlobalBeforeFilter;
	}

	const std::vector<ServiceGlobalFilter>& CMessageDirectory::getGlobalAfterFilter()
	{
		return this->m_vecServiceGlobalAfterFilter;
	}

	const std::string& CMessageDirectory::getMessageName(uint32_t nMessageID) const
	{
		auto iter = this->m_mapOwnerMessageName.find(nMessageID);
		if (iter == this->m_mapOwnerMessageName.end())
		{
			static std::string szMessageName;
			return szMessageName;
		}

		return iter->second;
	}

	void CMessageDirectory::delOtherServiceMessageInfo(const std::string& szServiceName)
	{
		auto& setMessageDirectory = this->m_mapOtherMessageDirectoryByServiceName[szServiceName];

		for (auto iter = setMessageDirectory.begin(); iter != setMessageDirectory.end(); ++iter)
		{
			const std::string& szMessageName = *iter;

			this->m_mapOtherMessageDirectoryByMessageName[szMessageName].erase(szServiceName);
		}

		this->m_mapOtherMessageDirectoryByServiceName.erase(szServiceName);
	}

	void CMessageDirectory::addOtherServiceMessageInfo(const std::string& szServiceName, const std::vector<SMessageSyncInfo>& vecMessageSyncInfo)
	{
		if (CCoreApp::Inst()->getServiceMgr()->getServiceBaseInfo(szServiceName) == nullptr)
		{
			PrintWarning("unknown service name server_name: %s", szServiceName.c_str());
			return;
		}

		auto& setMessageDirectory = this->m_mapOtherMessageDirectoryByServiceName[szServiceName];

		for (size_t i = 0; i < vecMessageSyncInfo.size(); ++i)
		{
			const SMessageSyncInfo& sMessageSyncInfo = vecMessageSyncInfo[i];
			auto iter = setMessageDirectory.find(sMessageSyncInfo.szMessageName);
			if (iter == setMessageDirectory.end())
			{
				setMessageDirectory.insert(sMessageSyncInfo.szMessageName);
			}
		}

		for (size_t i = 0; i < vecMessageSyncInfo.size(); ++i)
		{
			const std::string& szMessageName = vecMessageSyncInfo[i].szMessageName;

			this->m_mapOtherMessageDirectoryByMessageName[szMessageName].insert(szServiceName);
		}
	}
}