#include "stdafx.h"
#include "node_connection_factory.h"
#include "service_base.h"
#include "base_connection_to_master.h"
#include "base_connection_other_node.h"
#include "core_common_define.h"

namespace core
{
	CBaseConnection* CNodeConnectionFactory::createBaseConnection(uint32_t nType)
	{
		switch (nType)
		{
		case eBCT_ConnectionToMaster:
			{
				CBaseConnectionToMaster* pBaseConnectionToMaster = new CBaseConnectionToMaster();
				
				return pBaseConnectionToMaster;
			}
			break;

		case eBCT_ConnectionToOtherNode:
		case eBCT_ConnectionFromOtherNode:
			{
				CBaseConnectionOtherNode* pBaseConnectionOtherNode = new CBaseConnectionOtherNode();

				return pBaseConnectionOtherNode;
			}
			break;

		default:
			{
				PrintWarning("CNodeConnectionFactory::createBaseConnection error type: %d", nType);
			}
		}

		return nullptr;
	}
}