#pragma once

#include "libCoreCommon/core_common.h"
#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "core_service_kit_define.h"
#include "core_connection_to_other_node.h"
#include "core_connection_from_other_node.h"
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
		
		void							addNodeBaseInfo(const SNodeBaseInfo& sNodeBaseInfo);
		void							delNodeBaseInfo(uint16_t nID, bool bForce);

		void							setSerializeAdapter(uint16_t nID, CSerializeAdapter* pSerializeAdapter);
		CSerializeAdapter*				getSerializeAdapter(uint16_t nID) const;

		uint16_t						getNodeID(const std::string& szName) const;

		const SNodeBaseInfo*			getNodeBaseInfo(uint16_t nID) const;
		
		CCoreConnectionToOtherNode*		getCoreConnectionToOtherNode(uint16_t nServiceID) const;
		CCoreConnectionFromOtherNode*	getCoreConnectionFromOtherNode(uint16_t nServiceID) const;
		
		bool							addCoreConnectionToOtherNode(CCoreConnectionToOtherNode* pCoreConnectionToOtherNode);
		bool							addCoreConnectionFromOtherNode(uint16_t nID, CCoreConnectionFromOtherNode* pCoreConnectionFromOtherNode);
		void							delCoreConnectionToOtherNode(uint16_t nID);
		void							delCoreConnectionFromOtherNode(uint16_t nID);

	private:
		struct SNodeInfo
		{
			SNodeBaseInfo					sNodeBaseInfo;
			CCoreConnectionToOtherNode*		pCoreConnectionToOtherNode;
			CCoreConnectionFromOtherNode*	pCoreConnectionFromOtherNode;
			std::unique_ptr<CTicker>		pTicker;
		};

		std::map<uint16_t, SNodeInfo>			m_mapNodeInfo;
		std::map<std::string, uint16_t>			m_mapNodeName;
		CSerializeAdapter*						m_pDefaultSerializeAdapter;
		std::map<uint16_t, CSerializeAdapter*>	m_mapSerializeAdapter;
	};
}