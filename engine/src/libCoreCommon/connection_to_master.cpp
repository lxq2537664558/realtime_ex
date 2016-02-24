#include "stdafx.h"
#include "libCoreCommon/proto_system.h"
#include "libCoreCommon/core_app.h"

#include "connection_to_master.h"
#include "service_mgr.h"

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
		CSMT_SyncServiceInfo netMsg;
		netMsg.nType = CSMT_SyncServiceInfo::eAdd;
		base::crt::strncpy(netMsg.szName, _countof(netMsg.szName), CCoreApp::Inst()->getServiceBaseInfo().szName, _TRUNCATE);
		base::crt::strncpy(netMsg.szHost, _countof(netMsg.szHost), CCoreApp::Inst()->getServiceBaseInfo().szHost, _TRUNCATE);
		netMsg.nPort = CCoreApp::Inst()->getServiceBaseInfo().nPort;

		this->send(eMT_SYSTEM, &netMsg, sizeof(netMsg));
	}

	void CConnectionToMaster::onDisconnect()
	{

	}

	void CConnectionToMaster::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
	{
		DebugAst(nMsgType == eMT_SYSTEM);

		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
		DebugAst(nSize > sizeof(core::message_header));

		if (pHeader->nMsgID == eSMT_SyncServiceInfo)
		{
			const CSMT_SyncServiceInfo* pInfo = reinterpret_cast<const CSMT_SyncServiceInfo*>(pData);
			SServiceBaseInfo sServiceBaseInfo;
			base::crt::strncpy(sServiceBaseInfo.szName, _countof(sServiceBaseInfo.szName), pInfo->szName, _TRUNCATE);
			base::crt::strncpy(sServiceBaseInfo.szHost, _countof(sServiceBaseInfo.szHost), pInfo->szHost, _TRUNCATE);
			sServiceBaseInfo.nPort = pInfo->nPort;
			sServiceBaseInfo.nSendBufSize = pInfo->nSendBufSize;
			sServiceBaseInfo.nRecvBufSize = pInfo->nRecvBufSize;
			
			if (pInfo->nType == CSMT_SyncServiceInfo::eAdd)
				CServiceMgr::Inst()->addServiceBaseInfo(sServiceBaseInfo);
			else
				CServiceMgr::Inst()->delServiceBaseInfo(sServiceBaseInfo.szName);
		}
	}
}