#include "stdafx.h"
#include "connection_from_node.h"
#include "master_app.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreServiceKit/proto_system.h"


CConnectionFromNode::CConnectionFromNode()
	: m_nNodeID(0)
{
}

CConnectionFromNode::~CConnectionFromNode()
{

}

bool CConnectionFromNode::init(uint32_t nType, const std::string& szContext)
{
	return core::CBaseConnection::init(nType, szContext);
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
}

void CConnectionFromNode::onDisconnect()
{
	PrintInfo("CConnectionFromNode::onDisconnect node_id: %d", this->getNodeID());
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
		if (!CMasterApp::Inst()->getServiceMgr()->registerNode(this, netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo))
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

		CMasterApp::Inst()->getServiceMgr()->unregisterNode(this->m_nNodeID);
	}
}