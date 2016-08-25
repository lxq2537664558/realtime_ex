#include "stdafx.h"
#include "core_connection_from_other_node.h"
#include "proto_system.h"
#include "message_dispatcher.h"
#include "core_service_app_impl.h"

namespace core
{
	CCoreConnectionFromOtherNode::CCoreConnectionFromOtherNode()
		: m_nNodeID(0)
	{

	}

	CCoreConnectionFromOtherNode::~CCoreConnectionFromOtherNode()
	{

	}

	bool CCoreConnectionFromOtherNode::init(const std::string& szContext)
	{
		return true;
	}

	uint32_t CCoreConnectionFromOtherNode::getType() const
	{
		return eBCT_ConnectionFromOtherNode;
	}

	void CCoreConnectionFromOtherNode::release()
	{
		delete this;
	}

	void CCoreConnectionFromOtherNode::onConnect()
	{
		// 这个是被动连接，对方节点信息会由对方节点主动推过来
	}

	void CCoreConnectionFromOtherNode::onDisconnect()
	{
		if (!this->getNodeID() != 0)
			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->delCoreConnectionFromOtherNode(this->getNodeID());
	}

	void CCoreConnectionFromOtherNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_SYSTEM)
		{
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
			DebugAst(nSize > sizeof(core::message_header));

			if (pHeader->nMessageID == eSMT_notify_node_base_info)
			{
				DebugAst(this->getNodeID() == 0);

				smt_notify_node_base_info netMsg;
				netMsg.unpack(pData, nSize);
				if (netMsg.nFromNodeID == 0)
				{
					this->shutdown(true, "empty node id");
					return;
				}

				if (!CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addCoreConnectionFromOtherNode(netMsg.nFromNodeID, this))
				{
					PrintWarning("dup node node_id %d", netMsg.nFromNodeID);
					this->shutdown(true, "dup node connection");
					return;
				}

				this->m_nNodeID = netMsg.nFromNodeID;
			}

			return;
		}
		else
		{
			// 如果连节点名字都没有上报就发送其他包过来了，肯定非法，直接踢掉
			if (this->getNodeID() == 0)
			{
				this->shutdown(true, "invalid connection");
				return;
			}

			CCoreServiceAppImpl::Inst()->getMessageDispatcher()->dispatch(this->getID(), this->getNodeID(), nMessageType, pData, nSize);
		}
	}

	uint16_t CCoreConnectionFromOtherNode::getNodeID() const
	{
		return this->m_nNodeID;
	}

}