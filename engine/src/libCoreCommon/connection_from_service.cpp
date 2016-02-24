#include "stdafx.h"
#include "connection_from_service.h"
#include "service_mgr.h"

#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\proto_system.h"
#include "libCoreCommon\core_app.h"

namespace core
{

	DEFINE_OBJECT(CConnectionFromService, 100)

	CConnectionFromService::CConnectionFromService()
	{

	}

	CConnectionFromService::~CConnectionFromService()
	{

	}

	void CConnectionFromService::onConnect(const std::string& szContext)
	{
		// 这个是被动连接，对方服务信息会由对方服务主动推过来
	}

	void CConnectionFromService::onDisconnect()
	{
		if (!this->m_szServiceName.empty())
			CServiceMgr::Inst()->delConnectionFromService(this->m_szServiceName);
	}

	void CConnectionFromService::onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize)
	{
		if (nMsgType == eMT_SYSTEM)
		{
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
			DebugAst(nSize > sizeof(core::message_header));

			if (pHeader->nMsgID == eSMT_SyncServiceInfo)
			{
				DebugAst(this->m_szServiceName.empty());

				const CSMT_SyncServiceInfo* pInfo = reinterpret_cast<const CSMT_SyncServiceInfo*>(pData);

				// 这里对其他服务的监听地址不感兴趣
				this->m_szServiceName = pInfo->szName;
				CServiceMgr::Inst()->addConnectionFromService(this);
			}
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

	const std::string& CConnectionFromService::getServiceName() const
	{
		return this->m_szServiceName;
	}

}