#include "stdafx.h"
#include "service_connection_factory.h"
#include "connection_from_service.h"

core::CBaseConnection* CServiceConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
{
	switch (nType)
	{
	case eBCT_ConnectionFromService:
		{
			CConnectionFromService* pConnectionFromService = new CConnectionFromService();
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