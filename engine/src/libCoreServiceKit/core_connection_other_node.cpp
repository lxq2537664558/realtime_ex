#include "stdafx.h"
#include "core_connection_other_node.h"
#include "proto_system.h"
#include "message_dispatcher.h"
#include "core_service_app_impl.h"

namespace core
{
	CCoreConnectionOtherNode::CCoreConnectionOtherNode()
		: m_nNodeID(0)
	{

	}

	CCoreConnectionOtherNode::~CCoreConnectionOtherNode()
	{

	}

	bool CCoreConnectionOtherNode::init(uint32_t nType, const std::string& szContext)
	{
		if (nType == eBCT_ConnectionToOtherNode)
		{
			uint32_t nServiceID = 0;
			if (!base::crt::atoui(szContext.c_str(), nServiceID))
				return false;

			this->m_nNodeID = (uint16_t)nServiceID;
		}

		return CBaseConnection::init(nType, szContext);
	}

	void CCoreConnectionOtherNode::release()
	{
		delete this;
	}

	void CCoreConnectionOtherNode::onConnect()
	{
		if (this->getType() == eBCT_ConnectionToOtherNode)
		{
			if (!CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addCoreConnectionToOtherNode(this))
			{
				this->shutdown(base::eNCCT_Force, "dup node connection");
				return;
			}

			// 同步节点名字
			smt_notify_node_base_info netMsg;
			netMsg.nFromServiceID = CCoreServiceAppImpl::Inst()->getNodeBaseInfo().nID;
			base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
			netMsg.pack(writeBuf);

			this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

			auto& callback = CCoreServiceAppImpl::Inst()->getServiceConnectCallback();
			if (callback != nullptr)
				callback(this->getNodeID());
		}
	}

	void CCoreConnectionOtherNode::onDisconnect()
	{
		if (!this->getNodeID() != 0)
		{
			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->delCoreConnectionToOtherNode(this->getNodeID());
			auto& callback = CCoreServiceAppImpl::Inst()->getServiceDisconnectCallback();
			if (callback != nullptr)
				callback(this->getNodeID());
		}
	}

	void CCoreConnectionOtherNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
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
				if (netMsg.nFromServiceID == 0)
				{
					this->shutdown(base::eNCCT_Force, "empty node id");
					return;
				}

				if (!CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addCoreConnectionFromOtherNode(netMsg.nFromServiceID, this))
				{
					PrintWarning("dup node node_id %d", netMsg.nFromServiceID);
					this->shutdown(base::eNCCT_Force, "dup node connection");
					return;
				}

				this->m_nNodeID = netMsg.nFromServiceID;
				auto& callback = CCoreServiceAppImpl::Inst()->getServiceConnectCallback();
				if (callback != nullptr)
					callback(this->getNodeID());
			}

			return;
		}
		else
		{
			// 如果连节点名字都没有上报就发送其他包过来了，肯定非法，直接踢掉
			if (this->getNodeID() == 0)
			{
				this->shutdown(base::eNCCT_Force, "invalid connection");
				return;
			}

			CCoreServiceAppImpl::Inst()->getMessageDispatcher()->dispatch(this->getID(), this->getNodeID(), nMessageType, pData, nSize);
		}
	}

	uint16_t CCoreConnectionOtherNode::getNodeID() const
	{
		return this->m_nNodeID;
	}

}