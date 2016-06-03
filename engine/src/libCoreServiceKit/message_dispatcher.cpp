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

			const std::string& szMessageName = CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->getMessageName(pCookice->nMessageID);
			
			CCoreServiceKitImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, szMessageName, szFromServiceName);

			ServiceCallback& callback = CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->getCallback(pCookice->nMessageID);
			if (callback != nullptr)
			{
				google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf(szMessageName, pCookice + 1, nSize - sizeof(request_cookice));
				if (nullptr == pMessage)
					CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("unserialize protobuf message from buf error");
				else
					callback(szFromServiceName, nMessageType, pMessage);

				SAFE_DELETE(pMessage);
			}
			sServiceSessionInfo.szServiceName.clear();
			sServiceSessionInfo.nSessionID = 0;

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->endRecv();
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, pCookice->szMessageName, szFromServiceName);

			SResponseWaitInfo* pResponseWaitInfo = CCoreServiceKitImpl::Inst()->getTransporter()->getResponseWaitInfo(pCookice->nSessionID, true);
			if (nullptr == pResponseWaitInfo)
			{
				CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("invalid session id by message dispatcher"UINT64FMT, pCookice->nSessionID);
				return;
			}

			Defer(delete pResponseWaitInfo);

			if (pResponseWaitInfo->callback != nullptr)
			{
				const std::string& szMessageName = pCookice->szMessageName;
				google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
				if (nullptr == pMessage)
				{
					CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("create protobuf message error");
					return;
				}

				Defer(delete pMessage);

				const void* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
				DebugAst(nSize > sizeof(response_cookice) + pCookice->nMessageNameLen);
				if (!pMessage->ParseFromArray(pMessageData, nSize - sizeof(response_cookice) - pCookice->nMessageNameLen))
				{
					CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("parse message from array error");
					return;
				}

				pResponseWaitInfo->callback(nMessageType, pMessage, (EResponseResultType)pCookice->nResult);
			}

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->endRecv();
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
		{
			const gate_cookice* pCookice = reinterpret_cast<const gate_cookice*>(pData);
			
			const std::string& szMessageName = CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->getMessageName(pCookice->nMessageID);

			CCoreServiceKitImpl::Inst()->getInvokerTrace()->beginRecv(pCookice->nTraceID, szMessageName, szFromServiceName);

			SClientSessionInfo session(szFromServiceName, pCookice->nSessionID);

			GateForwardCallback& callback = CCoreServiceKitImpl::Inst()->getCoreServiceInvoker()->getGateClientCallback(pCookice->nMessageID);
			if (callback != nullptr)
			{
				google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf(szMessageName, pCookice + 1, nSize - sizeof(gate_cookice));
				if (nullptr == pMessage)
					CCoreServiceKitImpl::Inst()->getInvokerTrace()->addTraceExtraInfo("unserialize protobuf message from buf error");
				else
					callback(session, nMessageType, pMessage);

				SAFE_DELETE(pMessage);
			}

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