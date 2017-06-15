#include "stdafx.h"
#include "service_connection_factory.h"
#include "service_base.h"
#include "core_connection_to_master.h"
#include "core_connection_other_service.h"

namespace core
{
	CBaseConnection* CServiceConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
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
				CCoreConnectionOtherNode* pCoreConnectionOtherService = new CCoreConnectionOtherNode();
				if (!pCoreConnectionOtherService->init(nType, szContext))
				{
					SAFE_RELEASE(pCoreConnectionOtherService);
					return nullptr;
				}

				return pCoreConnectionOtherService;
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