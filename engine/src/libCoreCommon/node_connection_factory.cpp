#include "stdafx.h"
#include "node_connection_factory.h"
#include "service_base.h"
#include "base_connection_to_master.h"
#include "base_connection_other_node.h"
#include "core_common_define.h"

namespace core
{
	CBaseConnection* CNodeConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
	{
		switch (nType)
		{
		case eBCT_ConnectionToMaster:
			{
				CBaseConnectionToMaster* pBaseConnectionToMaster = new CBaseConnectionToMaster();
				if (!pBaseConnectionToMaster->init(nType, szContext))
				{
					SAFE_RELEASE(pBaseConnectionToMaster);
					return nullptr;
				}

				return pBaseConnectionToMaster;
			}
			break;
		case eBCT_ConnectionFromOtherNode:
		case eBCT_ConnectionToOtherNode:
			{
				CBaseConnectionOtherNode* pBaseConnectionOtherNode = new CBaseConnectionOtherNode();
				if (!pBaseConnectionOtherNode->init(nType, szContext))
				{
					SAFE_RELEASE(pBaseConnectionOtherNode);
					return nullptr;
				}

				return pBaseConnectionOtherNode;
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