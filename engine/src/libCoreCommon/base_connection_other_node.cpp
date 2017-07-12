#include "stdafx.h"
#include "base_connection_other_node.h"
#include "message_dispatcher.h"
#include "core_common_define.h"
#include "proto_system.h"
#include "core_app.h"
#include "message_command.h"

namespace core
{
	CBaseConnectionOtherNode::CBaseConnectionOtherNode()
		: m_nNodeID(0)
	{

	}

	CBaseConnectionOtherNode::~CBaseConnectionOtherNode()
	{

	}

	bool CBaseConnectionOtherNode::init(uint32_t nType, const std::string& szContext)
	{
		return CBaseConnection::init(nType, szContext);
	}

	void CBaseConnectionOtherNode::release()
	{
		delete this;
	}

	void CBaseConnectionOtherNode::onConnect()
	{
		if (this->getType() == eBCT_ConnectionToOtherNode)
		{
			// 同步节点名字
			smt_notify_node_base_info netMsg;
			netMsg.sNodeBaseInfo = CCoreApp::Inst()->getNodeBaseInfo();
			netMsg.vecServiceBaseInfo = CCoreApp::Inst()->getServiceBaseInfo();
			base::CWriteBuf& writeBuf = CCoreApp::Inst()->getWriteBuf();
			netMsg.pack(writeBuf);

			this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
		}
	}

	void CBaseConnectionOtherNode::onDisconnect()
	{
		if (!this->getNodeID() != 0)
		{
			CCoreApp::Inst()->getCoreOtherNodeProxy()->delBaseConnectionOtherNodeByNodeID(this->getNodeID());
			auto& callback = CCoreApp::Inst()->getServiceDisconnectCallback();
			if (callback != nullptr)
			{
				std::vector<SServiceBaseInfo> vecServiceBaseInfo;
				CCoreApp::Inst()->getCoreOtherNodeProxy()->getServiceBaseInfoByNodeID(this->getNodeID(), vecServiceBaseInfo);
				for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
				{
					callback(vecServiceBaseInfo[i].nID);
				}
			}
		}
	}

	void CBaseConnectionOtherNode::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pContext)
	{
		if (nMessageType == eMT_SYSTEM)
		{
			const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
			DebugAst(nSize > sizeof(message_header));

			switch (pHeader->nMessageID)
			{
			case eSMT_notify_node_base_info:
				{
					DebugAst(this->getNodeID() == 0);

					smt_notify_node_base_info netMsg;
					netMsg.unpack(pData, nSize);

					CCoreApp::Inst()->getCoreOtherNodeProxy()->addNodeProxyInfo(netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo, false);

					if (!CCoreApp::Inst()->getCoreOtherNodeProxy()->addBaseConnectionOtherNodeByNodeID(netMsg.sNodeBaseInfo.nID, this))
					{
						this->shutdown(base::eNCCT_Force, "dup node connection");
						return;
					}

					this->m_nNodeID = netMsg.sNodeBaseInfo.nID;

					smt_notify_ack_node_base_info netMsg1;
					netMsg1.nNodeID = CCoreApp::Inst()->getNodeBaseInfo().nID;
					base::CWriteBuf& writeBuf = CCoreApp::Inst()->getWriteBuf();
					netMsg1.pack(writeBuf);
					this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

					auto& callback = CCoreApp::Inst()->getServiceConnectCallback();
					if (callback != nullptr)
					{
						std::vector<SServiceBaseInfo> vecServiceBaseInfo;
						CCoreApp::Inst()->getCoreOtherNodeProxy()->getServiceBaseInfoByNodeID(this->getNodeID(), vecServiceBaseInfo);
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

					if (!CCoreApp::Inst()->getCoreOtherNodeProxy()->addBaseConnectionOtherNodeByNodeID(netMsg.nNodeID, this))
					{
						this->shutdown(base::eNCCT_Force, "dup node connection");
						return;
					}

					this->m_nNodeID = netMsg.nNodeID;
					auto& callback = CCoreApp::Inst()->getServiceConnectCallback();
					if (callback != nullptr) 
					{
						std::vector<SServiceBaseInfo> vecServiceBaseInfo;
						CCoreApp::Inst()->getCoreOtherNodeProxy()->getServiceBaseInfoByNodeID(this->getNodeID(), vecServiceBaseInfo);
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

			CCoreApp::Inst()->getMessageDispatcher()->dispatch(this->getID(), this->getNodeID(), nMessageType, reinterpret_cast<const google::protobuf::Message*>(pData), reinterpret_cast<const SMCT_RECV_SOCKET_DATA*>(pContext));
		}
	}

	uint16_t CBaseConnectionOtherNode::getNodeID() const
	{
		return this->m_nNodeID;
	}
}