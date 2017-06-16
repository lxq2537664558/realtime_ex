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
			// 同步节点名字
			smt_notify_node_base_info netMsg;
			netMsg.sNodeBaseInfo = CCoreServiceAppImpl::Inst()->getNodeBaseInfo();
			netMsg.vecServiceBaseInfo = CCoreServiceAppImpl::Inst()->getServiceBaseInfo();
			base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
			netMsg.pack(writeBuf);

			this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
		}
	}

	void CCoreConnectionOtherNode::onDisconnect()
	{
		if (!this->getNodeID() != 0)
		{
			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->delCoreConnectionOtherNodeByNodeID(this->getNodeID());
			auto& callback = CCoreServiceAppImpl::Inst()->getServiceDisconnectCallback();
			if (callback != nullptr)
			{
				std::vector<SServiceBaseInfo> vecServiceBaseInfo;
				CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getServiceBaseInfoByNodeID(this->getNodeID(), vecServiceBaseInfo);
				for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
				{
					callback(vecServiceBaseInfo[i].nID);
				}
			}
		}
	}

	void CCoreConnectionOtherNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_SYSTEM)
		{
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
			DebugAst(nSize > sizeof(core::message_header));

			switch (pHeader->nMessageID)
			{
			case eSMT_notify_node_base_info:
				{
					DebugAst(this->getNodeID() == 0);

					smt_notify_node_base_info netMsg;
					netMsg.unpack(pData, nSize);

					CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addNodeProxyInfo(netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo, false);

					if (!CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addCoreConnectionOtherNodeByNodeID(netMsg.sNodeBaseInfo.nID, this))
					{
						this->shutdown(base::eNCCT_Force, "dup node connection");
						return;
					}

					this->m_nNodeID = netMsg.sNodeBaseInfo.nID;

					smt_notify_ack_node_base_info netMsg1;
					netMsg1.nNodeID = CCoreServiceAppImpl::Inst()->getNodeBaseInfo().nID;
					base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
					netMsg1.pack(writeBuf);
					this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

					auto& callback = CCoreServiceAppImpl::Inst()->getServiceConnectCallback();
					if (callback != nullptr)
					{
						std::vector<SServiceBaseInfo> vecServiceBaseInfo;
						CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getServiceBaseInfoByNodeID(this->getNodeID(), vecServiceBaseInfo);
						for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
						{
							callback(vecServiceBaseInfo[i].nID);
						}
					}
				}
				break;

			case eSMT_notify_ack_node_base_info:
				{
					smt_notify_ack_node_base_info netMsg;
					netMsg.unpack(pData, nSize);

					if (!CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addCoreConnectionOtherNodeByNodeID(netMsg.nNodeID, this))
					{
						this->shutdown(base::eNCCT_Force, "dup node connection");
						return;
					}

					this->m_nNodeID = netMsg.nNodeID;
					auto& callback = CCoreServiceAppImpl::Inst()->getServiceConnectCallback();
					if (callback != nullptr) 
					{
						std::vector<SServiceBaseInfo> vecServiceBaseInfo;
						CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getServiceBaseInfoByNodeID(this->getNodeID(), vecServiceBaseInfo);
						for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
						{
							callback(vecServiceBaseInfo[i].nID);
						}
					}
				}
				break;
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