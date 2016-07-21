#include "stdafx.h"
#include "core_connection_from_service.h"
#include "proto_system.h"
#include "message_dispatcher.h"
#include "core_service_kit_impl.h"

namespace core
{
	CCoreConnectionFromService::CCoreConnectionFromService()
	{

	}

	CCoreConnectionFromService::~CCoreConnectionFromService()
	{

	}

	bool CCoreConnectionFromService::init(const std::string& szContext)
	{
		return true;
	}

	uint32_t CCoreConnectionFromService::getType() const
	{
		return eBCT_ConnectionFromService;
	}

	void CCoreConnectionFromService::release()
	{
		delete this;
	}

	void CCoreConnectionFromService::onConnect()
	{
		// 这个是被动连接，对方服务信息会由对方服务主动推过来
	}

	void CCoreConnectionFromService::onDisconnect()
	{
		if (!this->m_szServiceName.empty())
			CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->delCoreConnectionFromService(this->m_szServiceName);
	}

	bool CCoreConnectionFromService::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_SYSTEM)
		{
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
			DebugAstEx(nSize > sizeof(core::message_header), true);

			if (pHeader->nMessageID == eSMT_notify_service_base_info)
			{
				DebugAstEx(this->m_szServiceName.empty(), true);

				smt_notify_service_base_info netMsg;
				netMsg.unpack(pData, nSize);
				if (netMsg.szFromServiceName.empty())
				{
					this->shutdown(true, "empty service name");
					return true;
				}

				if (!CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->addCoreConnectionFromService(netMsg.szFromServiceName, this))
				{
					PrintWarning("dup service service_name: %s", netMsg.szFromServiceName.c_str());
					this->shutdown(true, "dup service connection");
					return true;
				}

				this->m_szServiceName = netMsg.szFromServiceName;
			}

			return true;
		}
		else
		{
			// 如果连服务名字都没有上报就发送其他包过来了，肯定非法，直接踢掉
			if (this->m_szServiceName.empty())
			{
				this->shutdown(true, "invalid connection");
				return true;
			}
			return CMessageDispatcher::Inst()->dispatch(this->getServiceName(), nMessageType, pData, nSize);
		}
	}

	const std::string& CCoreConnectionFromService::getServiceName() const
	{
		return this->m_szServiceName;
	}

}