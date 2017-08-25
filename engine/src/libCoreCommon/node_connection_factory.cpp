#include "stdafx.h"
#include "node_connection_factory.h"
#include "service_base.h"
#include "base_connection_to_master.h"
#include "base_connection_other_node.h"
#include "core_common_define.h"

namespace core
{
	CBaseConnection* CNodeConnectionFactory::createBaseConnection(const std::string& szType)
	{
		if (szType == "CBaseConnectionToMaster")
		{
			CBaseConnectionToMaster* pBaseConnectionToMaster = new CBaseConnectionToMaster();

			return pBaseConnectionToMaster;
		}
		else if(szType == "CBaseConnectionOtherNode")
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = new CBaseConnectionOtherNode();

			return pBaseConnectionOtherNode;
		}
		else
		{
			PrintWarning("CNodeConnectionFactory::createBaseConnection error type: {}", szType);
		}

		return nullptr;
	}
}