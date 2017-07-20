#include "stdafx.h"
#include "connection_from_node.h"
#include "master_app.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/proto_system.h"


CConnectionFromNode::CConnectionFromNode()
	: m_nNodeID(0)
{
}

CConnectionFromNode::~CConnectionFromNode()
{

}

bool CConnectionFromNode::init(const std::string& szContext)
{
	return true;
}

uint32_t CConnectionFromNode::getType() const
{
	return eBCT_ConnectionFromService;
}

void CConnectionFromNode::release()
{
	delete this;
}

uint16_t CConnectionFromNode::getNodeID() const
{
	return this->m_nNodeID;
}

void CConnectionFromNode::onConnect()
{
	core::smt_sync_master_info netMsg;
	netMsg.nMasterID = CMasterApp::Inst()->getMasterID();
	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();
	netMsg.pack(writeBuf);
	this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
}

void CConnectionFromNode::onDisconnect()
{
	CMasterApp::Inst()->getServiceRegistry()->delNode(this->m_nNodeID);
}

void CConnectionFromNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_SYSTEM);

	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	DebugAst(nSize > sizeof(core::message_header));

	if (pHeader->nMessageID == eSMT_register_node_base_info)
	{
		core::smt_register_node_base_info netMsg;
		netMsg.unpack(pData, nSize);
		
		this->m_nNodeID = netMsg.sNodeBaseInfo.nID;
		if (!CMasterApp::Inst()->getServiceRegistry()->addNode(this, netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo))
		{
			this->m_nNodeID = 0;
			this->shutdown(base::eNCCT_Force, "dup node connection");
			return;
		}
	}
	else if (pHeader->nMessageID == eSMT_unregister_node_base_info)
	{
		core::smt_unregister_node_base_info netMsg;
		netMsg.unpack(pData, nSize);

		CMasterApp::Inst()->getServiceRegistry()->delNode(this->m_nNodeID);
	}
}