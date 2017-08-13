#include "stdafx.h"
#include "client_connection_factory.h"
#include "connection_from_client.h"

#include "libBaseCommon/debug_helper.h"

using namespace core;

CBaseConnection* CClientConnectionFactory::createBaseConnection(const std::string& szType)
{
	DebugAstEx(szType == "CConnectionFromClient", nullptr);

	CConnectionFromClient* pConnectionFromClient = new CConnectionFromClient();

	return pConnectionFromClient;
}