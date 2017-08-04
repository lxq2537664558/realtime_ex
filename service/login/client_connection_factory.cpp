#include "stdafx.h"
#include "client_connection_factory.h"
#include "connection_from_client.h"

#include "libBaseCommon\logger.h"

using namespace core;

CBaseConnection* CClientConnectionFactory::createBaseConnection(uint32_t nType)
{
	switch (nType)
	{
	case eBCT_ConnectionFromClient:
		{
			CConnectionFromClient* pConnectionFromClient = new CConnectionFromClient();
			
			return pConnectionFromClient;
		}
		break;

	default:
		{
			PrintWarning("CClientConnectionFactory::createBaseConnection error type: %d", nType);
		}
	}

	return nullptr;
}