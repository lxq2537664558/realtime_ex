#include "stdafx.h"
#include "connection_from_client.h"
#include "connection_from_service.h"

DEFINE_OBJECT(CConnectionFromClient, 100)

CConnectionFromClient::CConnectionFromClient()
	: m_pConnectionFromService(nullptr)
{

}

CConnectionFromClient::~CConnectionFromClient()
{

}

void CConnectionFromClient::onConnect(const std::string& szContext)
{

}

void CConnectionFromClient::onDisconnect()
{

}

void CConnectionFromClient::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
{
	DebugAst(nMsgType == eMT_CLIENT);

	if (this->m_pConnectionFromService == nullptr)
		return;

	gate_header header;
	header.nID = this->getID();
	this->m_pConnectionFromService->send(eMT_GATE, &header, sizeof(header), pData, nSize);
}