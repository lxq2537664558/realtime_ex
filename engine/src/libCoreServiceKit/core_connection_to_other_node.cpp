#include "stdafx.h"
#include "core_connection_to_other_node.h"
#include "proto_system.h"
#include "message_dispatcher.h"
#include "core_service_app_impl.h"
#include "core_other_node_proxy.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"

namespace core
{
	CCoreConnectionToOtherNode::CCoreConnectionToOtherNode()
		: m_nNodeID(0)
	{

	}

	CCoreConnectionToOtherNode::~CCoreConnectionToOtherNode()
	{

	}

	bool CCoreConnectionToOtherNode::init(const std::string& szContext)
	{
		uint32_t nNodeID = 0;
		if (!base::crt::atoui(szContext.c_str(), nNodeID))
			return false;

		this->m_nNodeID = (uint16_t)nNodeID;
		return true;
	}

	uint32_t CCoreConnectionToOtherNode::getType() const
	{
		return eBCT_ConnectionToOtherNode;
	}

	void CCoreConnectionToOtherNode::release()
	{
		delete this;
	}

	void CCoreConnectionToOtherNode::onConnect()
	{
		if (CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getNodeBaseInfo(this->getNodeID()) == nullptr)
		{
			PrintWarning("unknown node node_id: %d", this->getNodeID());
			this->shutdown(true, "unknown node");
			return;
		}

		// 同步节点名字
		smt_notify_node_base_info netMsg;
		netMsg.nFromNodeID = CCoreServiceAppImpl::Inst()->getNodeBaseInfo().nID;
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);

		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		if (!CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addCoreConnectionToOtherNode(this))
		{
			this->shutdown(true, "dup node connection");
			return;
		}

		auto& callback = CCoreServiceAppImpl::Inst()->getNodeConnectCallback();
		if (callback != nullptr)
			callback(this->getNodeID());
	}

	void CCoreConnectionToOtherNode::onDisconnect()
	{
		if (!this->getNodeID() != 0)
		{
			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->delCoreConnectionToOtherNode(this->getNodeID());
			auto& callback = CCoreServiceAppImpl::Inst()->getNodeDisconnectCallback();
			if (callback != nullptr)
				callback(this->getNodeID());
		}
	}

	void CCoreConnectionToOtherNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType != eMT_SYSTEM)
		{
			CCoreServiceAppImpl::Inst()->getMessageDispatcher()->dispatch(this->getID(), this->getNodeID(), nMessageType, pData, nSize);
		}
	}

	uint16_t CCoreConnectionToOtherNode::getNodeID() const
	{
		return this->m_nNodeID;
	}
}