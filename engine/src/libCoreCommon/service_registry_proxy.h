#pragma once

#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "ticker.h"
#include "core_common.h"
#include "base_connection_other_node.h"
#include "base_connection_to_master.h"

#include <map>
#include <vector>
#include <string>
#include <memory>

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
		const std::vector<uint32_t>&	getServiceIDByTypeName(const std::string& szName) const;
		
		const SServiceBaseInfo*			getServiceBaseInfoByServiceID(uint32_t nServiceID) const;
		bool							getServiceBaseInfoByNodeID(uint32_t nNodeID, std::vector<SServiceBaseInfo>& vecServiceBaseInfo) const;
		
		CBaseConnectionOtherNode*		getBaseConnectionOtherNodeByServiceID(uint32_t nServiceID) const;
		CBaseConnectionOtherNode*		getBaseConnectionOtherNodeByNodeID(uint32_t nNodeID) const;
		bool							addBaseConnectionOtherNodeByNodeID(uint32_t nNodeID, CBaseConnectionOtherNode* pBaseConnectionOtherNode);
		void							delBaseConnectionOtherNodeByNodeID(uint32_t nNodeID);
		
		bool							addBaseConnectionToMaster(CBaseConnectionToMaster* pBaseConnectionToMaster);
		void							delBaseConnectionToMaster(uint32_t nMasterID);

	private:
		void							onCheckConnectMaster(uint64_t nContext);
		void							onConnectRefuse(const std::string& szContext);

	private:
		struct SNodeProxyInfo
		{
			SNodeBaseInfo					sNodeBaseInfo;
			std::vector<SServiceBaseInfo>	vecServiceBaseInfo;
			CBaseConnectionOtherNode*		pBaseConnectionOtherNode;
			std::unique_ptr<CTicker>		pTicker;
		};

		struct SServiceProxyInfo
		{
			SServiceBaseInfo				sServiceBaseInfo;
			CBaseConnectionOtherNode*		pBaseConnectionOtherNode;
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

		std::map<uint32_t, SNodeProxyInfo>		m_mapNodeProxyInfo;
		std::map<uint32_t, SServiceProxyInfo>	m_mapServiceProxyInfo;
		std::map<std::string, uint32_t>			m_mapServiceName;
		std::map<std::string, std::vector<uint32_t>>
												m_mapServiceIDByServiceType;

		std::map<uint32_t, SMasterInfo>			m_mapMasterInfo;
		CTicker									m_tickCheckConnectMaster;

		std::map<std::string, uint32_t>			m_mapConnectServiceName;	// 需要被连接的服务名字
		std::map<std::string, uint32_t>			m_mapConnectServiceType;	// 需要被连接的服务类型
	};
}