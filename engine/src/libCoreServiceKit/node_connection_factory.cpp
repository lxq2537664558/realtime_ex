#include "stdafx.h"
#include "node_connection_factory.h"
#include "service_base.h"
#include "core_connection_to_master.h"
#include "core_connection_to_other_node.h"
#include "core_connection_from_other_node.h"

namespace core
{
	CBaseConnection* CNodeConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
	{
		switch (nType)
		{
		case eBCT_ConnectionToMaster:
			{
				CCoreConnectionToMaster* pCoreConnectionToMaster = new CCoreConnectionToMaster();
				if (!pCoreConnectionToMaster->init(szContext))
				{
					SAFE_RELEASE(pCoreConnectionToMaster);
					return nullptr;
				}

				return pCoreConnectionToMaster;
			}
			break;
		case eBCT_ConnectionFromOtherNode:
			{
				CCoreConnectionFromOtherNode* pCoreConnectionFromOtherNode = new CCoreConnectionFromOtherNode();
				if (!pCoreConnectionFromOtherNode->init(szContext))
				{
					SAFE_RELEASE(pCoreConnectionFromOtherNode);
					return nullptr;
				}

				return pCoreConnectionFromOtherNode;
			}
			break;

		case eBCT_ConnectionToOtherNode:
			{
				CCoreConnectionToOtherNode* pCoreConnectionToOtherNode = new CCoreConnectionToOtherNode();
				if (!pCoreConnectionToOtherNode->init(szContext))
				{
					SAFE_RELEASE(pCoreConnectionToOtherNode);
					return nullptr;
				}

				return pCoreConnectionToOtherNode;
			}
			break;
		default:
			{
				PrintWarning("CNodeConnectionFactory::createBaseConnection error type: %d context: %s", nType, szContext.c_str());
			}
		}

		return nullptr;
	}
}