#include "stdafx.h"
#include "client_connection_factory.h"
#include "login_connection_from_client.h"

#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"

using namespace core;

CBaseConnection* CClientConnectionFactory::createBaseConnection(const std::string& szType)
{
	DebugAstEx(szType == "CLoginConnectionFromClient", nullptr);

	CLoginConnectionFromClient* pLoginConnectionFromClient = new CLoginConnectionFromClient();

	return pLoginConnectionFromClient;
}