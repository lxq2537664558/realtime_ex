#include "stdafx.h"
#include "base_connection_other_node.h"
#include "core_common_define.h"
#include "message_command.h"
#include "proto_system.h"
#include "core_app.h"

namespace core
{
	CBaseConnectionOtherNode::CBaseConnectionOtherNode()
		: m_nNodeID(0)
	{

	}

	CBaseConnectionOtherNode::~CBaseConnectionOtherNode()
	{

	}

	void CBaseConnectionOtherNode::release()
	{
		delete this;
	}

	void CBaseConnectionOtherNode::onConnect()
	{
		if (this->getMode() == base::net::eNCM_Initiative)
		{
			// 向目标节点发送握手请求
			smt_node_handshake_request netMsg;
			netMsg.nNodeID = CCoreApp::Inst()->getNodeID();
			base::CWriteBuf& writeBuf = CCoreApp::Inst()->getWriteBuf();
			netMsg.pack(writeBuf);

			this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
		}
	}

	void CBaseConnectionOtherNode::onDisconnect()
	{
		if (this->getNodeID() != 0)
		{
			CCoreApp::Inst()->getServiceRegistryProxy()->setOtherNodeSocketIDByNodeID(this->getNodeID(), 0);
			
			CCoreApp::Inst()->getServiceRegistryProxy()->onNodeDisconnect(this->getNodeID());
		}
	}

	void CBaseConnectionOtherNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(nMessageType == eMT_SYSTEM);

		const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
		DebugAst(nSize > sizeof(message_header));

		switch (pHeader->nMessageID)
		{
		case eSMT_node_handshake_request:
		{
			DebugAst(this->getNodeID() == 0);

			smt_node_handshake_request netMsg;
			netMsg.unpack(pData, nSize);

			std::vector<SServiceBaseInfo> vecSServiceBaseInfo;
			if (!CCoreApp::Inst()->getServiceRegistryProxy()->getServiceBaseInfoByNodeID(netMsg.nNodeID, vecSServiceBaseInfo))
			{
				// 有可能节点信息不存（master还没有同步过来），暂时就踢掉等待对方再次连接
				this->shutdown(true, "node info not find");
				PrintWarning("node info not find node_id: {}", netMsg.nNodeID);
				return;
			}

			if (!CCoreApp::Inst()->getServiceRegistryProxy()->setOtherNodeSocketIDByNodeID(netMsg.nNodeID, this->getID()))
			{
				this->shutdown(true, "dup node connection passive mode");
				return;
			}

			this->m_nNodeID = netMsg.nNodeID;

			this->responseHandshake();
		}
		break;

		case eSMT_node_handshake_response:
		{
			smt_node_handshake_response netMsg;
			netMsg.unpack(pData, nSize);
			if (!CCoreApp::Inst()->getServiceRegistryProxy()->setOtherNodeSocketIDByNodeID(netMsg.nNodeID, this->getID()))
			{
				this->shutdown(true, "dup node connection initiative mode");
				return;
			}

			this->m_nNodeID = netMsg.nNodeID;

			CCoreApp::Inst()->getServiceRegistryProxy()->onNodeConnect(this->getNodeID());
		}
		break;

		default:
		{
			PrintWarning("CBaseConnectionOtherNode::onDispatch invalid msg type: {}", pHeader->nMessageID);
		}
		}
	}

	uint32_t CBaseConnectionOtherNode::getNodeID() const
	{
		return this->m_nNodeID;
	}

	void CBaseConnectionOtherNode::setNodeID(uint32_t nNodeID)
	{
		this->m_nNodeID = nNodeID;
	}

	void CBaseConnectionOtherNode::responseHandshake()
	{
		// 响应握手
		smt_node_handshake_response netMsg1;
		netMsg1.nNodeID = CCoreApp::Inst()->getNodeID();
		base::CWriteBuf& writeBuf = CCoreApp::Inst()->getWriteBuf();
		netMsg1.pack(writeBuf);
		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		CCoreApp::Inst()->getServiceRegistryProxy()->onNodeConnect(this->getNodeID());
	}

}