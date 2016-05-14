#pragma once

#include "libCoreCommon/core_common.h"
#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "core_service_kit_define.h"
#include "core_connection_from_service.h"
#include "core_connection_to_service.h"

#include <map>
#include <string>

namespace core
{
	// 其他服务在本地服务的代理
	class CCoreServiceProxy :
		public base::noncopyable
	{
	public:
		CCoreServiceProxy();
		~CCoreServiceProxy();

		bool							init(tinyxml2::XMLElement* pRootXML);
		void							addMessageProxyInfo(const SMessageProxyInfo& sMessageProxyInfo);
		const SMessageProxyGroupInfo*	getMessageProxyGroupInfo(const std::string& szMessageName) const;
		const SMetaMessageProxyInfo*	getMetaMessageProxyInfo(const std::string& szMessageName) const;
		const SMetaMessageProxyInfo*	getMetaMessageProxyInfo(uint32_t nMessageID) const;
		uint32_t						getProxyInvokTimeout(uint32_t nMessageID) const;

		void							addService(const SServiceBaseInfo& sServiceBaseInfo);
		void							delService(const std::string& szServiceName);

		const SServiceBaseInfo*			getServiceBaseInfo(const std::string& szServiceName) const;
		CCoreConnectionToService*		getConnectionToService(const std::string& szName) const;
		bool							addConnectionToService(CCoreConnectionToService* pCoreConnectionToService);
		void							delConnectionToService(const std::string& szName);
		CCoreConnectionFromService*		getConnectionFromService(const std::string& szName) const;
		bool							addConnectionFromService(CCoreConnectionFromService* pCoreConnectionFromService);
		void							delConnectionFromService(const std::string& szName);

	private:
		std::map<std::string, SServiceBaseInfo>				m_mapServiceBaseInfo;
		std::map<std::string, CCoreConnectionToService*>	m_mapCoreConnectionToService;
		std::map<std::string, CCoreConnectionFromService*>	m_mapCoreConnectionFromService;
		std::map<uint32_t, SMetaMessageProxyInfo>			m_mapMetaMessageProxyInfo;
		std::map<std::string, SMessageProxyGroupInfo>		m_mapMessageProxyGroupInfo;
		uint32_t											m_nProxyInvokTimeout;
		std::string											m_szProxyLoadBalance;
		std::string											m_szProxyFail;
		uint32_t											m_nProxyRetries;

	};
}