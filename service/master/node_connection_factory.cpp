#include "stdafx.h"
#include "node_connection_factory.h"
#include "connection_from_node.h"

#include "libBaseCommon/debug_helper.h"

using namespace core;

CBaseConnection* CNodeConnectionFactory::createBaseConnection(const std::string& szType)
{
	DebugAstEx(szType == "CConnectionFromNode", nullptr);

	CConnectionFromNode* pConnectionFromNode = new CConnectionFromNode();

	return pConnectionFromNode;
}