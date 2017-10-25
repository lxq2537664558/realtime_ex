#include "stdafx.h"
#include "connection_from_node.h"
#include "master_service.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/proto_system.h"
#include "libCoreCommon/base_app.h"
#include "libBaseCommon/string_util.h"

using namespace core;

CConnectionFromNode::CConnectionFromNode(CMasterService* pMasterService)
	: m_nNodeID(0)
	, m_pMasterService(pMasterService)
{
}

CConnectionFromNode::~CConnectionFromNode()
{

}

void CConnectionFromNode::release()
{
	delete this;
}

uint32_t CConnectionFromNode::getNodeID() const
{
	return this->m_nNodeID;
}

void CConnectionFromNode::onConnect()
{
	smt_sync_master_info netMsg;
	netMsg.nMasterID = this->m_pMasterService->getMasterID();
	base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();
	netMsg.pack(writeBuf);
	this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
}

void CConnectionFromNode::onDisconnect()
{
	if (this->m_nNodeID != 0 && this->m_pMasterService != nullptr)
		this->m_pMasterService->getServiceRegistry()->delNode(this->m_nNodeID);
}

void CConnectionFromNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_SYSTEM);

	const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
	DebugAst(nSize > sizeof(message_header));

	if (pHeader->nMessageID == eSMT_register_node_base_info)
	{
		smt_register_node_base_info netMsg;
		netMsg.unpack(pData, nSize);
		
		if (!this->m_pMasterService->getServiceRegistry()->addNode(this, netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo, netMsg.setConnectServiceName, netMsg.setConnectServiceType))
		{
			this->shutdown(true, "dup node connection");
			return;
		}

		this->m_nNodeID = netMsg.sNodeBaseInfo.nID;
	}
	else if (pHeader->nMessageID == eSMT_unregister_node_base_info)
	{
		smt_unregister_node_base_info netMsg;
		netMsg.unpack(pData, nSize);

		this->m_pMasterService->getServiceRegistry()->delNode(this->m_nNodeID);
	}
}