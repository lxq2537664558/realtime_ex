#include "stdafx.h"
#include "core_connection_to_master.h"
#include "proto_system.h"
#include "core_service_app_impl.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

namespace core
{
	CCoreConnectionToMaster::CCoreConnectionToMaster()
	{

	}

	CCoreConnectionToMaster::~CCoreConnectionToMaster()
	{

	}

	bool CCoreConnectionToMaster::init(uint32_t nType, const std::string& szContext)
	{
		return CBaseConnection::init(nType, szContext);
	}

	void CCoreConnectionToMaster::release()
	{
		delete this;
	}

	void CCoreConnectionToMaster::onConnect()
	{
		smt_register_node_base_info netMsg;
		netMsg.sNodeBaseInfo = CCoreServiceAppImpl::Inst()->getNodeBaseInfo();
		netMsg.vecServiceBaseInfo = CCoreServiceAppImpl::Inst()->getServiceBaseInfo();

		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		
		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		CCoreServiceAppImpl::Inst()->setCoreConnectionToMaster(this);

		CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->onConnectToMaster();
	}

	void CCoreConnectionToMaster::onDisconnect()
	{
		CCoreServiceAppImpl::Inst()->setCoreConnectionToMaster(nullptr);
	}

	void CCoreConnectionToMaster::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(nMessageType == eMT_SYSTEM);

		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
		DebugAst(nSize > sizeof(core::message_header));

		if (pHeader->nMessageID == eSMT_sync_node_base_info)
		{
			smt_sync_node_base_info netMsg;
			netMsg.unpack(pData, nSize);
			
			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addNodeProxyInfo(netMsg.sNodeBaseInfo, netMsg.vecServiceBaseInfo, true);
		}
		else if (pHeader->nMessageID == eSMT_remove_node_base_info)
		{
			smt_remove_node_base_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->delNodeProxyInfo(netMsg.nNodeID);
		}
	}
}