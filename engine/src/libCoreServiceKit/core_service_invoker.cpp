#include "stdafx.h"
#include "core_service_invoker.h"
#include "proto_system.h"
#include "core_connection_to_master.h"
#include "core_service_kit_define.h"
#include "core_service_kit_impl.h"

#include "libCoreCommon\base_app.h"

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

		this->sendMessageInfo(szMessageName);
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
		
		this->sendMessageInfo(szMessageName);
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

		smt_register_service_message_info netMsg;
		for (auto iter = this->m_mapMessageName.begin(); iter != this->m_mapMessageName.end(); ++iter)
		{
			const std::string& szMessageName = iter->second;
			SMessageProxyInfo sMessageProxyInfo;
			sMessageProxyInfo.szMessageName = szMessageName;
			sMessageProxyInfo.szServiceName = CCoreServiceKitImpl::Inst()->getServiceBaseInfo().szName;
			sMessageProxyInfo.szServiceGroup = CCoreServiceKitImpl::Inst()->getServiceBaseInfo().szGroup;
			sMessageProxyInfo.nWeight = CCoreServiceKitImpl::Inst()->getServiceBaseInfo().nWeight;
			netMsg.vecMessageProxyInfo.push_back(sMessageProxyInfo);
		}

		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		pCoreConnectionToMaster->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	void CCoreServiceInvoker::sendMessageInfo(const std::string& szMessageName)
	{
		CCoreConnectionToMaster* pCoreConnectionToMaster = CCoreServiceKitImpl::Inst()->getConnectionToMaster();
		if (nullptr == pCoreConnectionToMaster)
			return;

		smt_register_service_message_info netMsg;
		SMessageProxyInfo sMessageProxyInfo;
		sMessageProxyInfo.szMessageName = szMessageName;
		sMessageProxyInfo.szServiceName = CCoreServiceKitImpl::Inst()->getServiceBaseInfo().szName;
		sMessageProxyInfo.szServiceGroup = CCoreServiceKitImpl::Inst()->getServiceBaseInfo().szGroup;
		sMessageProxyInfo.nWeight = CCoreServiceKitImpl::Inst()->getServiceBaseInfo().nWeight;
		netMsg.vecMessageProxyInfo.push_back(sMessageProxyInfo);
		
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		pCoreConnectionToMaster->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
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