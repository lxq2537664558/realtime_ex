#include "stdafx.h"
#include "connection_from_service.h"
#include "core_app.h"
#include "base_connection_mgr.h"
#include "proto_system.h"
#include "base_app.h"
#include "message_dispatcher.h"

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
			CCoreApp::Inst()->getServiceMgr()->delConnectionFromService(this->m_szServiceName);
	}

	void CConnectionFromService::onDispatch(uint16_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_SYSTEM)
		{
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
			DebugAst(nSize > sizeof(core::message_header));

			if (pHeader->nMessageID == eSMT_notify_service_base_info)
			{
				DebugAst(this->m_szServiceName.empty());

				smt_notify_service_base_info netMsg;
				netMsg.unpack(pData, nSize);

				// 这里对其他服务的监听地址不感兴趣
				this->m_szServiceName = netMsg.szName;
				CCoreApp::Inst()->getServiceMgr()->addConnectionFromService(this);
			}
		}
		else
		{
			CMessageDispatcher::Inst()->dispatch(this->getServiceName(), nMessageType, pData, nSize);
		}
	}

	const std::string& CConnectionFromService::getServiceName() const
	{
		return this->m_szServiceName;
	}

}