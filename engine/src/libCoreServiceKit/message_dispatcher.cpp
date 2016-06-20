#include "stdafx.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_kit_impl.h"
#include "core_service_kit_define.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

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

	void CMessageDispatcher::dispatch(const std::string& szFromServiceName, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		// 先前置过滤器过一遍，如果有一个返回false就直接跳过这个消息
		const std::vector<ServiceGlobalFilter>& vecServiceGlobalBeforeFilter = CCoreServiceKitImpl::Inst()->getGlobalBeforeFilter();
		for (size_t i = 0; i < vecServiceGlobalBeforeFilter.size(); ++i)
		{
			if (vecServiceGlobalBeforeFilter[i] != nullptr && !vecServiceGlobalBeforeFilter[i](szFromServiceName, nMessageType, pData, nSize))
				return;
		}

		if ((nMessageType&eMT_TYPE_MASK) == eMT_REQUEST)
		{
			DebugAst(nSize > sizeof(request_cookice));

			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			SServiceSessionInfo& sServiceSessionInfo = CCoreServiceKitImpl::Inst()->getTransporter()->getServiceSessionInfo();
			sServiceSessionInfo.szServiceName = szFromServiceName;
			sServiceSessionInfo.nSessionID = pCookice->nSessionID;

			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, pHeader->nMessageID, szFromServiceName);

			ServiceCallback& callback = CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->getCallback(pHeader->nMessageID);
			if (callback != nullptr)
				callback(szFromServiceName, nMessageType, pHeader);
			
			sServiceSessionInfo.szServiceName.clear();
			sServiceSessionInfo.nSessionID = 0;

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->endRecv();
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);
			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, pHeader->nMessageID, szFromServiceName);

			SResponseWaitInfo* pResponseWaitInfo = CCoreServiceKitImpl::Inst()->getTransporter()->getResponseWaitInfo(pCookice->nSessionID, true);
			if (nullptr == pResponseWaitInfo)
			{
				CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("invalid session id by message dispatcher"UINT64FMT, pCookice->nSessionID);
				return;
			}

			Defer(delete pResponseWaitInfo);

			if (pResponseWaitInfo->callback != nullptr)
				pResponseWaitInfo->callback(nMessageType, pHeader, (EResponseResultType)pCookice->nResult);

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->endRecv();
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
		{
			const gate_cookice* pCookice = reinterpret_cast<const gate_cookice*>(pData);
			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			
			CCoreServiceKitImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, pHeader->nMessageID, szFromServiceName);

			SClientSessionInfo session(szFromServiceName, pCookice->nSessionID);

			GateForwardCallback& callback = CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->getGateClientCallback(pHeader->nMessageID);
			if (callback != nullptr)
				callback(session, nMessageType, pHeader);

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->endRecv();
		}

		const std::vector<ServiceGlobalFilter>& vecServiceGlobalAfterFilter = CCoreServiceKitImpl::Inst()->getGlobalAfterFilter();
		for (size_t i = 0; i < vecServiceGlobalAfterFilter.size(); ++i)
		{
			if (vecServiceGlobalAfterFilter[i] != nullptr)
				vecServiceGlobalAfterFilter[i](szFromServiceName, nMessageType, pData, nSize);
		}
	}
}