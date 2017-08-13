#include "stdafx.h"
#include "connection_from_client.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/proto_system.h"
#include "libCoreCommon/base_app.h"

using namespace core;

CConnectionFromClient::CConnectionFromClient()
{
}

CConnectionFromClient::~CConnectionFromClient()
{

}

void CConnectionFromClient::release()
{
	delete this;
}

void CConnectionFromClient::onConnect()
{
	
}

void CConnectionFromClient::onDisconnect()
{
	
}

void CConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	this->send(nMessageType, pData, nSize);
}