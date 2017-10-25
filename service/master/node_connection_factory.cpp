#include "stdafx.h"
#include "node_connection_factory.h"
#include "connection_from_node.h"

#include "libBaseCommon/debug_helper.h"

using namespace core;

CNodeConnectionFactory::CNodeConnectionFactory(CMasterService* pMasterService)
	: m_pMasterService(pMasterService)
{

}

CNodeConnectionFactory::~CNodeConnectionFactory()
{

}

CBaseConnection* CNodeConnectionFactory::createBaseConnection(const std::string& szType)
{
	DebugAstEx(szType == "CConnectionFromNode", nullptr);

	CConnectionFromNode* pConnectionFromNode = new CConnectionFromNode(this->m_pMasterService);

	return pConnectionFromNode;
}