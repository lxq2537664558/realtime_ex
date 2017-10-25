#include "stdafx.h"
#include "base_connection_to_master.h"
#include "proto_system.h"
#include "core_app.h"

namespace core
{
	CBaseConnectionToMaster::CBaseConnectionToMaster()
		: m_nMasterID(0)
	{

	}

	CBaseConnectionToMaster::~CBaseConnectionToMaster()
	{

	}

	void CBaseConnectionToMaster::release()
	{
		delete this;
	}

	uint32_t CBaseConnectionToMaster::getMasterID() const
	{
		return this->m_nMasterID;
	}

	void CBaseConnectionToMaster::onConnect()
	{
	}

	void CBaseConnectionToMaster::onDisconnect()
	{
		if (this->m_nMasterID != 0)
			CCoreApp::Inst()->getServiceRegistryProxy()->delBaseConnectionToMaster(this->m_nMasterID);
	}

	void CBaseConnectionToMaster::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(nMessageType == eMT_SYSTEM);

		const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
		DebugAst(nSize > sizeof(message_header));

		switch (pHeader->nMessageID)
		{
		case eSMT_sync_master_info:
		{
			smt_sync_master_info netMsg;
			netMsg.unpack(pData, nSize);

			this->m_nMasterID = netMsg.nMasterID;
			if (!CCoreApp::Inst()->getServiceRegistryProxy()->addBaseConnectionToMaster(this))
			{
				this->shutdown(true, "dup master connection");
				return;
			}

			smt_register_node_base_info netMsg1;
			netMsg1.sNodeBaseInfo = CCoreApp::Inst()->getNodeBaseInfo();
			netMsg1.vecServiceBaseInfo = CCoreApp::Inst()->getCoreServiceMgr()->getServiceBaseInfo();
			netMsg1.setConnectServiceName = CCoreApp::Inst()->getServiceRegistryProxy()->getConnectServiceName();
			netMsg1.setConnectServiceType = CCoreApp::Inst()->getServiceRegistryProxy()->getConnectServiceType();

			base::CWriteBuf& writeBuf = CCoreApp::Inst()->getWriteBuf();
			netMsg1.pack(writeBuf);

			this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
		}
		break;
		
		case eSMT_sync_node_base_info:
		{
			smt_sync_node_base_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreApp::Inst()->getServiceRegistryProxy()->addNodeProxyInfo(netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo, !!netMsg.bExcludeConnect);
		}
		break;

		case eSMT_sync_all_node_base_info:
		{
			smt_sync_all_node_base_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreApp::Inst()->getServiceRegistryProxy()->setNodeProxyInfo(netMsg.mapNodeInfo, netMsg.setExcludeConnectNodeID);
		}
		break;

		case eSMT_remove_node_base_info:
		{
			smt_remove_node_base_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreApp::Inst()->getServiceRegistryProxy()->delNodeProxyInfo(netMsg.nNodeID);
		}
		break;

		default:
		{
			PrintWarning("CBaseConnectionToMaster::onDispatch invalid msg type: {}", pHeader->nMessageID);
		}
		}
	}
}