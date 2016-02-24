#include "stdafx.h"
#include "connection_to_service.h"
#include "service_mgr.h"

#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\proto_system.h"
#include "libCoreCommon\core_app.h"

namespace core
{

	DEFINE_OBJECT(CConnectionToService, 100)

	CConnectionToService::CConnectionToService()
	{

	}

	CConnectionToService::~CConnectionToService()
	{

	}

	void CConnectionToService::onConnect(const std::string& szContext)
	{
		// szContext中存的是服务名字
		// 这里不需要告诉其他服务，因为是本服务主动连其他服务的，所以没有必要知道地址
		CSMT_SyncServiceInfo netMsg;
		netMsg.nType = CSMT_SyncServiceInfo::eSync;
		base::crt::strncpy(netMsg.szName, _countof(netMsg.szName), CCoreApp::Inst()->getServiceBaseInfo().szName, _TRUNCATE);
		memset(netMsg.szHost, 0, _countof(netMsg.szHost));
		netMsg.nPort = 0;

		this->send(eMT_SYSTEM, &netMsg, sizeof(netMsg));

		this->m_szServiceName = szContext;

		CServiceMgr::Inst()->addConnectionToService(this);
	}

	void CConnectionToService::onDisconnect()
	{
		if (!this->m_szServiceName.empty())
			CServiceMgr::Inst()->delConnectionToService(this->m_szServiceName);
	}

	void CConnectionToService::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
	{
		if (nMsgType == eMT_SYSTEM)
		{
			//
		}
		else if (nMsgType == eMT_REQUEST)
		{
			CServiceMgr::Inst()->onDispatch(nMsgType, pData, nSize);
		}
		else if (nMsgType == eMT_RESPONSE)
		{
			CServiceMgr::Inst()->onDispatch(nMsgType, pData, nSize);
		}
	}

	const std::string& CConnectionToService::getServiceName() const
	{
		return this->m_szServiceName;
	}

}