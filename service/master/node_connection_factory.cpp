#include "stdafx.h"
#include "node_connection_factory.h"
#include "connection_from_node.h"

#include "libBaseCommon\debug_helper.h"

core::CBaseConnection* CNodeConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
{
	switch (nType)
	{
	case eBCT_ConnectionFromService:
		{
			CConnectionFromNode* pConnectionFromService = new CConnectionFromNode();
			if (!pConnectionFromService->init(szContext))
			{
				SAFE_RELEASE(pConnectionFromService);
				return nullptr;
			}

			return pConnectionFromService;
		}
		break;
	default:
		{
			PrintWarning("CServiceConnectionFactory::createBaseConnection error type: %d context: %s", nType, szContext.c_str());
		}
	}

	return nullptr;
}