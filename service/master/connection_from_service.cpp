#include "stdafx.h"
#include "connection_from_service.h"
#include "master_app.h"

#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\proto_system.h"

DEFINE_OBJECT(CConnectionFromService, 100)

CConnectionFromService::CConnectionFromService()
{
	memset(&this->m_sServiceBaseInfo, 0, sizeof(this->m_sServiceBaseInfo));
}

CConnectionFromService::~CConnectionFromService()
{

}

const SServiceBaseInfo& CConnectionFromService::getServiceBaseInfo() const
{
	return this->m_sServiceBaseInfo;
}

void CConnectionFromService::onConnect(const std::string& szContext)
{
	// 连接连上来，将已经注册的服务同步给该链接
	std::vector<CBaseConnection*> vecBaseConnection;
	CMasterApp::Inst()->getBaseConnectionMgr()->getBaseConnection(this->getClassName(), vecBaseConnection);
	for (size_t i = 0; i < vecBaseConnection.size(); ++i)
	{
		CBaseConnection* pBaseConnection = vecBaseConnection[i];
		if (nullptr == pBaseConnection)
			continue;

		CConnectionFromService* pServiceConnection = dynamic_cast<CConnectionFromService*>(pBaseConnection);
		if (nullptr == pServiceConnection)
			continue;

		CSMT_SyncServiceInfo netMsg;
		netMsg.nType = CSMT_SyncServiceInfo::eAdd;
		base::crt::strncpy(netMsg.szName, _countof(netMsg.szName), pServiceConnection->m_sServiceBaseInfo.szName, _TRUNCATE);
		base::crt::strncpy(netMsg.szHost, _countof(netMsg.szHost), pServiceConnection->m_sServiceBaseInfo.szHost, _TRUNCATE);
		netMsg.nPort = pServiceConnection->m_sServiceBaseInfo.nPort;
		netMsg.nSendBufSize = pServiceConnection->m_sServiceBaseInfo.nSendBufSize;
		netMsg.nRecvBufSize = pServiceConnection->m_sServiceBaseInfo.nRecvBufSize;

		this->send(eMT_SYSTEM, &netMsg, sizeof(netMsg));
	}
}

void CConnectionFromService::onDisconnect()
{
	CMasterApp::Inst()->getServiceMgr()->delServiceConnection(this->m_sServiceBaseInfo.szName);

	if (this->m_sServiceBaseInfo.szName[0] != 0)
	{
		CSMT_SyncServiceInfo netMsg;
		netMsg.nType = CSMT_SyncServiceInfo::eDel;
		base::crt::strncpy(netMsg.szName, _countof(netMsg.szName), this->m_sServiceBaseInfo.szName, _TRUNCATE);
		memset(netMsg.szHost, 0, _countof(netMsg.szHost));
		netMsg.nPort = 0;
		netMsg.nSendBufSize = 0;
		netMsg.nRecvBufSize = 0;

		CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(this->getClassName(), eMT_SYSTEM, &netMsg, sizeof(netMsg));
	}
}

void CConnectionFromService::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
{
	DebugAst(nMsgType == eMT_SYSTEM);
	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	DebugAst(nSize > sizeof(core::message_header));

	if (pHeader->nMsgID == eSMT_SyncServiceInfo)
	{
		const CSMT_SyncServiceInfo* pInfo = reinterpret_cast<const CSMT_SyncServiceInfo*>(pData);
		DebugAst(pInfo->nType == CSMT_SyncServiceInfo::eAdd);

		base::crt::strncpy(this->m_sServiceBaseInfo.szName, _countof(this->m_sServiceBaseInfo.szName), pInfo->szName, _TRUNCATE);
		base::crt::strncpy(this->m_sServiceBaseInfo.szHost, _countof(this->m_sServiceBaseInfo.szHost), pInfo->szHost, _TRUNCATE);
		this->m_sServiceBaseInfo.nPort = pInfo->nPort;
		// 监听方跟主动连接方的接收缓存，发送缓存大小对调
		this->m_sServiceBaseInfo.nSendBufSize = pInfo->nRecvBufSize;
		this->m_sServiceBaseInfo.nRecvBufSize = pInfo->nSendBufSize;

		CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(this->getClassName(), eMT_SYSTEM, pData, nSize);
		
		CMasterApp::Inst()->getServiceMgr()->addServiceConnection(this);
	}
}