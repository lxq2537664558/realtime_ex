#pragma once

#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/ticker.h"

#include "tinyxml2/tinyxml2.h"

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
	class CLocalServiceRegistryProxy;

	class CGlobalServiceRegistryProxy :
		public base::noncopyable
	{
	public:
		CGlobalServiceRegistryProxy();
		~CGlobalServiceRegistryProxy();

		bool			init(tinyxml2::XMLElement* pXMLElement);

		void			addNodeProxyInfo(const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo, bool bExcludeConnect);
		void			setNodeProxyInfo(const std::map<uint32_t, SNodeInfo>& mapNodeInfo, const std::set<uint32_t>& setExcludeConnectNodeID);
		void			delNodeProxyInfo(uint32_t nNodeID);
		
		bool			getServiceBaseInfoByNodeID(uint32_t nNodeID, std::vector<SServiceBaseInfo>& vecServiceBaseInfo);
		
		bool			setOtherNodeSocketIDByNodeID(uint32_t nNodeID, uint64_t nSocketID);

		bool			addBaseConnectionToMaster(CBaseConnectionToMaster* pBaseConnectionToMaster);
		void			delBaseConnectionToMaster(uint32_t nMasterID);

		void			onNodeConnect(uint32_t nNodeID);
		void			onNodeDisconnect(uint32_t nNodeID);

		const std::set<std::string>&
						getConnectServiceName() const;
		const std::set<std::string>&
						getConnectServiceType() const;

		void			fillLocalServiceRegistryProxy(CLocalServiceRegistryProxy* pLocalServiceRegistryProxy);

	private:
		void			onCheckConnectMaster(uint64_t nContext);
		void			onConnectRefuse(const std::string& szContext);
		void			updateActiveServiceID(const std::set<std::string>& setType);
		void			updateLocalServiceRegistryProxy();

	public:
		struct SNodeProxyInfo
		{
			SNodeBaseInfo					sNodeBaseInfo;
			std::vector<SServiceBaseInfo>	vecServiceBaseInfo;
			uint64_t						nSocketID;
			std::unique_ptr<base::CTicker>	pTicker;
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
		};

	private:
		std::map<uint32_t, SNodeProxyInfo>		m_mapNodeProxyInfo;
		std::map<uint32_t, SServiceProxyInfo>	m_mapServiceProxyInfo;
		std::map<std::string, uint32_t>			m_mapServiceNameByID;
		std::map<uint32_t, std::string>			m_mapServiceIDByName;
		std::map<std::string, SServiceIDInfo>	m_mapServiceIDInfoByType;

		std::map<uint32_t, SMasterInfo>			m_mapMasterInfo;
		base::CTicker							m_tickCheckConnectMaster;

		std::set<std::string>					m_setConnectServiceName;	// 需要被连接的服务名字
		std::set<std::string>					m_setConnectServiceType;	// 需要被连接的服务类型
	};
}