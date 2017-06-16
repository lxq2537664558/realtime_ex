#include "stdafx.h"
#include "node_connection_factory.h"
#include "connection_from_node.h"

core::CBaseConnection* CNodeConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
{
	switch (nType)
	{
	case eBCT_ConnectionFromNode:
	{
		CConnectionFromNode* pConnectionFromNode = new CConnectionFromNode();
		if (!pConnectionFromNode->init(nType, szContext))
		{
			SAFE_RELEASE(pConnectionFromNode);
			return nullptr;
		}

		return pConnectionFromNode;
	}
	break;
	default:
	{
		PrintWarning("CNodeConnectionFactory::createBaseConnection error type: %d context: %s", nType, szContext.c_str());
	}
	}

	return nullptr;
}