#include "stdafx.h"
#include "message_directory.h"
#include "proto_system.h"
#include "core_app.h"
#include "connection_to_master.h"

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
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter != this->m_mapMessageName.end())
		{
			// Ãû×Ö¹þÏ£³åÍ»ÁË
			PrintWarning("dup message name by register service callback message_id: %d exist_message_name: %s new_message_name: %s", nMessageID, iter->second.c_str(), szMessageName.c_str());
			return;
		}
		this->m_mapServiceCallback[nMessageID] = callback;
		this->m_mapMessageName[nMessageID] = szMessageName;

		this->sendMessageInfo(szMessageName, false);
	}

	void CMessageDirectory::registerCallback(const std::string& szMessageName, const GateClientCallback& callback)
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
		
		this->sendMessageInfo(szMessageName, true);
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
		CConnectionToMaster* pConnectionToMaster = CCoreApp::Inst()->getServiceMgr()->getConnectionToMaster();
		DebugAst(nullptr != pConnectionToMaster);

		smt_sync_service_message_info netMsg;
		netMsg.nAdd = 0;
		for (auto iter = this->m_mapServiceCallback.begin(); iter != this->m_mapServiceCallback.end(); ++iter)
		{
			const std::string& szMessageName = this->getMessageName(iter->first);
			if (szMessageName.empty())
				continue;

			SMessageSyncInfo sMessageSyncInfo;
			sMessageSyncInfo.nGate = 0;
			sMessageSyncInfo.szMessageName = szMessageName;
			netMsg.vecMessageSyncInfo.push_back(sMessageSyncInfo);
		}

		for (auto iter = this->m_mapGateClientCallback.begin(); iter != this->m_mapGateClientCallback.end(); ++iter)
		{
			const std::string& szMessageName = this->getMessageName(iter->first);
			if (szMessageName.empty())
				continue;

			SMessageSyncInfo sMessageSyncInfo;
			sMessageSyncInfo.nGate = 1;
			sMessageSyncInfo.szMessageName = szMessageName;
			netMsg.vecMessageSyncInfo.push_back(sMessageSyncInfo);
		}

		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		pConnectionToMaster->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	void CMessageDirectory::sendMessageInfo(const std::string& szMessageName, bool bGate)
	{
		CConnectionToMaster* pConnectionToMaster = CCoreApp::Inst()->getServiceMgr()->getConnectionToMaster();
		if (nullptr == pConnectionToMaster)
			return;

		smt_sync_service_message_info netMsg;
		netMsg.nAdd = 1;
		SMessageSyncInfo sMessageSyncInfo;
		sMessageSyncInfo.nGate = bGate;
		sMessageSyncInfo.szMessageName = szMessageName;
		netMsg.vecMessageSyncInfo.push_back(sMessageSyncInfo);
		
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		pConnectionToMaster->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	void CMessageDirectory::addMessage(const std::string& szServiceName, uint32_t nMessageID, bool bGate)
	{
		std::vector<std::string>* pMessageDirectory = nullptr;
		if (bGate)
			pMessageDirectory = &this->m_mapGateClientMessageDirectory[nMessageID];
		else
			pMessageDirectory = &this->m_mapServiceMessageDirectory[nMessageID];

		std::vector<std::string>& vecMessageDirectory = *pMessageDirectory;

		for (size_t i = 0; i < vecMessageDirectory.size(); ++i)
		{
			if (vecMessageDirectory[i] == szServiceName)
			{
				PrintWarning("dup message service_name: %s message_id: %d", szServiceName.c_str(), nMessageID);
				return;
			}
		}

		vecMessageDirectory.push_back(szServiceName);
	}

	void CMessageDirectory::delMessage(const std::string& szServiceName, uint32_t nMessageID, bool bGate)
	{
		std::vector<std::string>* pMessageDirectory = nullptr;
		if (bGate)
			pMessageDirectory = &this->m_mapGateClientMessageDirectory[nMessageID];
		else
			pMessageDirectory = &this->m_mapServiceMessageDirectory[nMessageID];

		std::vector<std::string>& vecMessageDirectory = *pMessageDirectory;

		for (size_t i = 0; i < vecMessageDirectory.size(); ++i)
		{
			if (vecMessageDirectory[i] == szServiceName)
			{
				vecMessageDirectory.erase(vecMessageDirectory.begin() + i);
				return;
			}
		}
	}

	const std::vector<std::string>& CMessageDirectory::getServiceName(uint32_t nMessageID, bool bGate) const
	{
		static std::vector<std::string> s_vecMessageServiceName;
		if (bGate)
		{
			auto iter = this->m_mapGateClientMessageDirectory.find(nMessageID);
			if (iter == this->m_mapGateClientMessageDirectory.end())
				return s_vecMessageServiceName;

			return iter->second;
		}
		else
		{
			auto iter = this->m_mapServiceMessageDirectory.find(nMessageID);
			if (iter == this->m_mapServiceMessageDirectory.end())
				return s_vecMessageServiceName;

			return iter->second;
		}
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
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter == this->m_mapMessageName.end())
		{
			std::string szMessageName;
			return szMessageName;
		}

		return iter->second;
	}

}