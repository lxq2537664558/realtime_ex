#pragma once

#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "core_common.h"
#include "base_connection_other_node.h"
#include "base_connection_to_master.h"

#include <map>
#include <vector>
#include <string>
#include <memory>

namespace core
{
	// 其他节点在本地节点的代理
	class CCoreOtherNodeProxy :
		public base::noncopyable
	{
	public:
		CCoreOtherNodeProxy();
		~CCoreOtherNodeProxy();

		bool							init(tinyxml2::XMLElement* pXMLElement);
		
		void							addNodeProxyInfo(const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo, bool bMaster);
		void							delNodeProxyInfo(uint16_t nID);

		uint16_t						getServiceID(const std::string& szName) const;
		std::string						getServiceType(uint16_t nServiceID) const;
		std::string						getServiceName(uint16_t nServiceID) const;
		const std::vector<uint16_t>&	getServiceIDByTypeName(const std::string& szName) const;
		
		const SServiceBaseInfo*			getServiceBaseInfoByServiceID(uint16_t nServiceID) const;
		bool							getServiceBaseInfoByNodeID(uint16_t nNodeID, std::vector<SServiceBaseInfo>& vecServiceBaseInfo) const;
		
		CBaseConnectionOtherNode*		getBaseConnectionOtherNodeByServiceID(uint16_t nServiceID) const;
		CBaseConnectionOtherNode*		getBaseConnectionOtherNodeByNodeID(uint16_t nNodeID) const;
		bool							addBaseConnectionOtherNodeByNodeID(uint16_t nNodeID, CBaseConnectionOtherNode* pBaseConnectionOtherNode);
		void							delBaseConnectionOtherNodeByNodeID(uint16_t nNodeID);
		
		CBaseConnectionToMaster*		getBaseConnectionToMaster() const;
		bool							addBaseConnectionToMaster(CBaseConnectionToMaster* pBaseConnectionToMaster);
		void							delBaseConnectionToMaster(uint16_t nMasterID);

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
			uint16_t	nID;
			std::string	szHost;
			uint16_t	nPort;
			bool		bActive;
			CBaseConnectionToMaster*
						pBaseConnectionToMaster;
		};

		std::map<uint16_t, SNodeProxyInfo>		m_mapNodeProxyInfo;
		std::map<uint16_t, SServiceProxyInfo>	m_mapServiceProxyInfo;
		std::map<std::string, uint16_t>			m_mapServiceName;
		std::map<std::string, std::vector<uint16_t>>
												m_mapServiceIDByServiceType;

		std::map<uint16_t, SMasterInfo>			m_mapMasterInfo;
		std::vector<CBaseConnectionToMaster*>	m_vecBaseConnectionToMaster;
		CTicker									m_tickCheckConnectMaster;

		std::map<std::string, uint32_t>			m_mapConnectServiceName;	// 需要被连接的服务名字
		std::map<std::string, uint32_t>			m_mapConnectServiceType;	// 需要被连接的服务类型
	};
}