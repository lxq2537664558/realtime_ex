#pragma once

#include "libCoreCommon/core_common.h"
#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "core_service_kit_define.h"
#include "core_connection_other_node.h"
#include "serialize_adapter.h"

#include <map>
#include <string>

namespace core
{
	// 其他节点在本地节点的代理
	class CCoreOtherNodeProxy :
		public base::noncopyable
	{
	public:
		CCoreOtherNodeProxy();
		~CCoreOtherNodeProxy();

		bool							init();
		
		void							addNodeProxyInfo(const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo);
		void							delNodeProxyInfo(uint16_t nID);

		void							setSerializeAdapter(uint16_t nID, CSerializeAdapter* pSerializeAdapter);
		CSerializeAdapter*				getSerializeAdapter(uint16_t nID) const;

		uint16_t						getServiceID(const std::string& szName) const;
		
		const SServiceBaseInfo*			getServiceBaseInfo(uint16_t nID) const;
		
		CCoreConnectionOtherNode*		getCoreConnectionOtherNodeByServiceID(uint16_t nServiceID) const;
		CCoreConnectionOtherNode*		getCoreConnectionOtherNodeByNodeID(uint16_t nNodeID) const;
		bool							addCoreConnectionOtherNodeByNodeID(uint16_t nNodeID, CCoreConnectionOtherNode* pCoreConnectionOtherNode);
		void							delCoreConnectionOtherNodeByNodeID(uint16_t nNodeID);
		
	private:
		struct SNodeProxyInfo
		{
			SNodeBaseInfo					sNodeBaseInfo;
			std::vector<SServiceBaseInfo>	vecServiceBaseInfo;
			CCoreConnectionOtherNode*		pCoreConnectionOtherNode;
			std::unique_ptr<CTicker>		pTicker;
		};

		struct SServiceProxyInfo
		{
			SServiceBaseInfo				sServiceBaseInfo;
			CCoreConnectionOtherNode*		pCoreConnectionOtherNode;
		};

		std::map<uint16_t, SNodeProxyInfo>		m_mapNodeProxyInfo;
		std::map<uint16_t, SServiceProxyInfo>	m_mapServiceProxyInfo;
		std::map<std::string, uint16_t>			m_mapServiceName;
		CSerializeAdapter*						m_pDefaultSerializeAdapter;
		std::map<uint16_t, CSerializeAdapter*>	m_mapSerializeAdapter;
	};
}