#include "stdafx.h"
#include "libCoreCommon/proto_system.h"
#include "libCoreCommon/base_app.h"

#include "connection_to_master.h"
#include "core_app.h"

namespace core
{

	DEFINE_OBJECT(CConnectionToMaster, 1)

	CConnectionToMaster::CConnectionToMaster()
	{

	}

	CConnectionToMaster::~CConnectionToMaster()
	{

	}

	void CConnectionToMaster::onConnect(const std::string& szContext)
	{
		smt_register_service_base_info netMsg;
		netMsg.sServiceBaseInfo = CBaseApp::Inst()->getServiceBaseInfo();

		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		
		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		CCoreApp::Inst()->getMessageDirectory()->onConnectToMaster();
	}

	void CConnectionToMaster::onDisconnect()
	{

	}

	void CConnectionToMaster::onDispatch(uint32_t nMsgType, const void* pData, uint16_t nSize)
	{
		DebugAst(nMsgType == eMT_SYSTEM);

		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
		DebugAst(nSize > sizeof(core::message_header));

		if (pHeader->nMessageID == eSMT_sync_service_base_info)
		{
			smt_sync_service_base_info netMsg;
			netMsg.unpack(pData, nSize);
			
			CCoreApp::Inst()->getServiceMgr()->addOtherService(netMsg.sServiceBaseInfo);
		}
		else if (pHeader->nMessageID == eSMT_remove_service_base_info)
		{
			smt_remove_service_base_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreApp::Inst()->getServiceMgr()->delOtherService(netMsg.szName);
		}
		else if (pHeader->nMessageID == eSMT_sync_service_message_info)
		{
			smt_sync_service_message_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreApp::Inst()->getMessageDirectory()->addOtherServiceMessageInfo(netMsg.szServiceName, netMsg.vecMessageSyncInfo);
		}
	}
}