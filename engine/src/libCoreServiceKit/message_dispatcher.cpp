#include "stdafx.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_app_impl.h"
#include "core_service_kit_define.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"
#include "libCoreCommon/coroutine.h"

namespace core
{
	CMessageDispatcher::CMessageDispatcher()
	{

	}

	CMessageDispatcher::~CMessageDispatcher()
	{

	}

	bool CMessageDispatcher::init()
	{
		return true;
	}

	bool CMessageDispatcher::dispatch(uint16_t nFromServiceID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAstEx(pData != nullptr, true);

		const std::vector<ServiceGlobalFilter>& vecGlobalBeforeFilter = CCoreServiceAppImpl::Inst()->getGlobalBeforeFilter();
		for (size_t i = 0; i < vecGlobalBeforeFilter.size(); ++i)
		{
			vecGlobalBeforeFilter[i](nFromServiceID, nMessageType, pData, nSize);
		}

		CMessage pMessage = nullptr;
		if ((nMessageType&eMT_TYPE_MASK) == eMT_REQUEST)
		{
			DebugAstEx(nSize > sizeof(request_cookice), true);

			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			SServiceSessionInfo& sServiceSessionInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getServiceSessionInfo();
			sServiceSessionInfo.nServiceID = nFromServiceID;
			sServiceSessionInfo.nSessionID = pCookice->nSessionID;

			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			CCoreServiceAppImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, pHeader->nMessageID, nFromServiceID);

			ServiceCallback& callback = CCoreServiceAppImpl::Inst()->getCoreServiceInvoker()->getCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				pMessage = CMessage(const_cast<message_header*>(pHeader));
				callback(nFromServiceID, nMessageType, pMessage);
			}
			sServiceSessionInfo.nServiceID = 0;
			sServiceSessionInfo.nSessionID = 0;

			CCoreServiceAppImpl::Inst()->getInvokerTrace()->endRecv();
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);
			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			CCoreServiceAppImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, pHeader->nMessageID, nFromServiceID);

			if (pCookice->nActorID == 0)
			{
				SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(pCookice->nSessionID, true);
				if (nullptr == pResponseWaitInfo)
				{
					CCoreServiceAppImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("invalid session id by message dispatcher"UINT64FMT, pCookice->nSessionID);
					return true;
				}

				Defer(delete pResponseWaitInfo);

				if (pCookice->nResult == eRRT_OK)
				{
					pMessage = CMessage(const_cast<message_header*>(pHeader));
					pResponseWaitInfo->callback(pResponseWaitInfo, pMessage, eRRT_OK);
				}
				else if (pCookice->nResult != eRRT_OK)
				{
					pResponseWaitInfo->callback(pResponseWaitInfo, nullptr, (EResponseResultType)pCookice->nResult);
				}
			}

			CCoreServiceAppImpl::Inst()->getInvokerTrace()->endRecv();
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
		{
			const gate_forward_cookice* pCookice = reinterpret_cast<const gate_forward_cookice*>(pData);
			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			
			CCoreServiceAppImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, pHeader->nMessageID, nFromServiceID);
			
			SClientSessionInfo session(nFromServiceID, pCookice->nSessionID);

			GateForwardCallback& callback = CCoreServiceAppImpl::Inst()->getCoreServiceInvoker()->getGateClientCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				pMessage = CMessage(const_cast<message_header*>(pHeader));
				callback(session, nMessageType, pMessage);
			}
			CCoreServiceAppImpl::Inst()->getInvokerTrace()->endRecv();
		}

		const std::vector<ServiceGlobalFilter>& vecGlobalAfterFilter = CCoreServiceAppImpl::Inst()->getGlobalAfterFilter();
		for (size_t i = 0; i < vecGlobalAfterFilter.size(); ++i)
		{
			vecGlobalAfterFilter[i](nFromServiceID, nMessageType, pData, nSize);
		}

		if (this->m_forwardCallback == nullptr)
			return true;

		return this->m_forwardCallback(nFromServiceID, nMessageType, pData, nSize);
	}

	void CMessageDispatcher::setForwardCallback(std::function<bool(uint16_t, uint8_t, const void*, uint16_t)> callback)
	{
		this->m_forwardCallback = callback;
	}
}