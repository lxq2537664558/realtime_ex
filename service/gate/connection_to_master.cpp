#include "stdafx.h"
#include "libCoreCommon/proto_system.h"

#include "connection_to_master.h"
#include "gate_app.h"

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
	base::crt::strncpy(netMsg.szName, _countof(netMsg.szName), CGateApp::Inst()->getServiceBaseInfo().szName, _TRUNCATE);
	base::crt::strncpy(netMsg.szHost, _countof(netMsg.szHost), CGateApp::Inst()->getServiceBaseInfo().szHost, _TRUNCATE);
	netMsg.nPort = CGateApp::Inst()->getServiceBaseInfo().nPort;
	netMsg.nSendBufSize = CGateApp::Inst()->getServiceBaseInfo().nSendBufSize;
	netMsg.nRecvBufSize = CGateApp::Inst()->getServiceBaseInfo().nRecvBufSize;

	this->send(eMT_SYSTEM, &netMsg, sizeof(netMsg));
}

void CConnectionToMaster::onDisconnect()
{

}

void CConnectionToMaster::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
{
	DebugAst(nMsgType == eMT_SYSTEM);

	// 这里对其他服务不感兴趣，如果其他服务需要网关服务，那么就主动来连接
}