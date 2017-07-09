#include "stdafx.h"
#include "base_connection_to_master.h"
#include "base_connection_mgr.h"
#include "proto_system.h"
#include "core_app.h"

namespace core
{
	CBaseConnectionToMaster::CBaseConnectionToMaster()
	{

	}

	CBaseConnectionToMaster::~CBaseConnectionToMaster()
	{

	}

	bool CBaseConnectionToMaster::init(uint32_t nType, const std::string& szContext)
	{
		return CBaseConnection::init(nType, szContext);
	}

	void CBaseConnectionToMaster::release()
	{
		delete this;
	}

	void CBaseConnectionToMaster::onConnect()
	{
		smt_register_node_base_info netMsg;
		netMsg.sNodeBaseInfo = CCoreApp::Inst()->getNodeBaseInfo();
		netMsg.vecServiceBaseInfo = CCoreApp::Inst()->getServiceBaseInfo();

		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		
		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		CCoreApp::Inst()->setCoreConnectionToMaster(this);

		CCoreApp::Inst()->getCoreMessageRegistry()->onConnectToMaster();
	}

	void CBaseConnectionToMaster::onDisconnect()
	{
		CCoreApp::Inst()->setCoreConnectionToMaster(nullptr);
	}

	void CBaseConnectionToMaster::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(nMessageType == eMT_SYSTEM);

		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
		DebugAst(nSize > sizeof(core::message_header));

		if (pHeader->nMessageID == eSMT_sync_node_base_info)
		{
			smt_sync_node_base_info netMsg;
			netMsg.unpack(pData, nSize);
			
			CCoreApp::Inst()->getCoreOtherNodeProxy()->addNodeProxyInfo(netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo, true);
		}
		else if (pHeader->nMessageID == eSMT_remove_node_base_info)
		{
			smt_remove_node_base_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreApp::Inst()->getCoreOtherNodeProxy()->delNodeProxyInfo(netMsg.nNodeID);
		}
	}
}