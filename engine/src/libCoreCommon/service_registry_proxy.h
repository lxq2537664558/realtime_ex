#pragma once

#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "ticker.h"
#include "core_common.h"
#include "logic_service_lock.h"
#include "base_connection_other_node.h"
#include "base_connection_to_master.h"

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <set>

namespace core
{
	class CServiceRegistryProxy :
		public base::noncopyable
	{
	public:
		CServiceRegistryProxy();
		~CServiceRegistryProxy();

		bool							init(tinyxml2::XMLElement* pXMLElement);

		void							addNodeProxyInfo(const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo, bool bMaster);
		void							delNodeProxyInfo(uint32_t nID);

		uint32_t						getServiceID(const std::string& szName) const;
		std::string						getServiceType(uint32_t nServiceID) const;
		std::string						getServiceName(uint32_t nServiceID) const;
		const std::set<uint32_t>&		getServiceIDByType(const std::string& szName) const;
		const std::vector<uint32_t>&	getActiveServiceIDByType(const std::string& szName) const;

		bool							isValidService(uint32_t nServiceID) const;

		uint32_t						getServiceInvokeTimeout(uint32_t nServiceID) const;
		
		const SServiceBaseInfo*			getServiceBaseInfoByServiceID(uint32_t nServiceID) const;
		bool							getServiceBaseInfoByNodeID(uint32_t nNodeID, std::vector<SServiceBaseInfo>& vecServiceBaseInfo) const;
		
		uint64_t						getOtherNodeSocketIDByServiceID(uint32_t nServiceID) const;
		uint64_t						getOtherNodeSocketIDByNodeID(uint32_t nNodeID) const;
		bool							setOtherNodeSocketIDByNodeID(uint32_t nNodeID, uint64_t nSocketID);
		
		bool							addBaseConnectionToMaster(CBaseConnectionToMaster* pBaseConnectionToMaster);
		void							delBaseConnectionToMaster(uint32_t nMasterID);

	private:
		void							onCheckConnectMaster(uint64_t nContext);
		void							onConnectRefuse(const std::string& szContext);
		void							updateActiveServiceID(const std::set<std::string>& setType);

	private:
		struct SNodeProxyInfo
		{
			SNodeBaseInfo					sNodeBaseInfo;
			std::vector<SServiceBaseInfo>	vecServiceBaseInfo;
			uint64_t						nSocketID;
			std::unique_ptr<CTicker>		pTicker;
		};

		struct SServiceProxyInfo
		{
			SServiceBaseInfo	sServiceBaseInfo;
			uint64_t			nSocketID;
		};

		struct SMasterInfo
		{
			uint32_t	nID;
			std::string	szHost;
			uint16_t	nPort;
			bool		bActive;
			CBaseConnectionToMaster*
						pBaseConnectionToMaster;
		};

		struct SServiceIDInfo
		{
			std::set<uint32_t>		setServiceID;
			std::set<uint32_t>		setActiveServiceID;	// 没有剔除掉因为过载保护而踢除的服务
			std::vector<uint32_t>	vecActiveServiceID;
			CLogicServiceLock		lock;
		};

		CLogicServiceLock						m_sLock;
		std::map<uint32_t, SNodeProxyInfo>		m_mapNodeProxyInfo;
		std::map<uint32_t, SServiceProxyInfo>	m_mapServiceProxyInfo;
		std::map<std::string, uint32_t>			m_mapServiceNameByID;
		std::map<uint32_t, std::string>			m_mapServiceIDByName;
		std::map<std::string, SServiceIDInfo>	m_mapServiceIDInfoByType;

		std::map<uint32_t, SMasterInfo>			m_mapMasterInfo;
		CTicker									m_tickCheckConnectMaster;

		std::map<std::string, uint32_t>			m_mapConnectServiceName;	// 需要被连接的服务名字
		std::map<std::string, uint32_t>			m_mapConnectServiceType;	// 需要被连接的服务类型
	};
}