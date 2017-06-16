#include "stdafx.h"
#include "node_connection_factory.h"
#include "service_base.h"
#include "core_connection_to_master.h"
#include "core_connection_other_node.h"

namespace core
{
	CBaseConnection* CNodeConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
	{
		switch (nType)
		{
		case eBCT_ConnectionToMaster:
			{
				CCoreConnectionToMaster* pCoreConnectionToMaster = new CCoreConnectionToMaster();
				if (!pCoreConnectionToMaster->init(nType, szContext))
				{
					SAFE_RELEASE(pCoreConnectionToMaster);
					return nullptr;
				}

				return pCoreConnectionToMaster;
			}
			break;
		case eBCT_ConnectionFromOtherNode:
		case eBCT_ConnectionToOtherNode:
			{
				CCoreConnectionOtherNode* pCoreConnectionOtherNode = new CCoreConnectionOtherNode();
				if (!pCoreConnectionOtherNode->init(nType, szContext))
				{
					SAFE_RELEASE(pCoreConnectionOtherNode);
					return nullptr;
				}

				return pCoreConnectionOtherNode;
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