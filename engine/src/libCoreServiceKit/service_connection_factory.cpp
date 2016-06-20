#include "stdafx.h"
#include "service_connection_factory.h"
#include "service_base.h"
#include "core_connection_to_master.h"
#include "core_service_connection.h"


namespace core
{
	CBaseConnection* CServiceConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
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
		case eBCT_ConnectionService:
			{
				CCoreServiceConnection* pCoreConnectionToService = new CCoreServiceConnection();
				if (!pCoreConnectionToService->init(szContext))
				{
					SAFE_RELEASE(pCoreConnectionToService);
					return nullptr;
				}

				return pCoreConnectionToService;
			}
			break;
		default:
			{
				PrintWarning("CServiceConnectionFactory::createBaseConnection error type: %d context: %s", nType, szContext.c_str());
			}
		}

		return nullptr;
	}
}