#include "stdafx.h"
#include "node_connection_factory.h"
#include "connection_from_node.h"

#include "libBaseCommon\debug_helper.h"

core::CBaseConnection* CNodeConnectionFactory::createBaseConnection(uint32_t nType)
{
	switch (nType)
	{
	case eBCT_ConnectionFromService:
		{
			CConnectionFromNode* pConnectionFromService = new CConnectionFromNode();

			return pConnectionFromService;
		}
		break;
	default:
		{
			PrintWarning("CServiceConnectionFactory::createBaseConnection error type: %d", nType);
		}
	}

	return nullptr;
}