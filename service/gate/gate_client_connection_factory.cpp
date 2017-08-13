#include "stdafx.h"
#include "gate_client_connection_factory.h"
#include "gate_connection_from_client.h"

#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

using namespace core;

CBaseConnection* CGateClientConnectionFactory::createBaseConnection(const std::string& szType)
{
	DebugAstEx(szType == "CGateConnectionFromClient", nullptr);

	CGateConnectionFromClient* pGateConnectionFromClient = new CGateConnectionFromClient();

	return pGateConnectionFromClient;
}