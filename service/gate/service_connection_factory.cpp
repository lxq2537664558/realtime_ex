#include "stdafx.h"
#include "service_connection_factory.h"
#include "connection_from_client.h"

core::CBaseConnection* CServiceConnectionFactory::createBaseConnection(uint32_t nType, const std::string& szContext)
{
	switch (nType)
	{
	case eBCT_ConnectionFromClient:
		{
			CConnectionFromClient* pConnectionFromClient = new CConnectionFromClient();
			if (!pConnectionFromClient->init(szContext))
			{
				SAFE_RELEASE(pConnectionFromClient);
				return nullptr;
			}

			return pConnectionFromClient;
		}
		break;
	default:
		{
			PrintWarning("CServiceConnectionFactory::createBaseConnection error type: %d context: %s", nType, szContext.c_str());
		}
	}

	return nullptr;
}