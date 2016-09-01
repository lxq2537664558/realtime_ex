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

	void CMessageDispatcher::dispatch(uint64_t nFromSocketID, uint16_t nFromNodeID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		bool bFilter = false;
		const std::vector<GlobalBeforeFilter>& vecGlobalBeforeFilter = CCoreServiceAppImpl::Inst()->getGlobalBeforeFilter();
		for (size_t i = 0; i < vecGlobalBeforeFilter.size(); ++i)
		{
			if (!vecGlobalBeforeFilter[i](nFromSocketID, nFromNodeID, nMessageType, pData, nSize))
			{
				bFilter = true;
				break;
			}
		}
		if (bFilter)
			return;

		if ((nMessageType&eMT_TYPE_MASK) == eMT_REQUEST)
		{
			DebugAst(nSize > sizeof(request_cookice));

			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			SNodeSessionInfo& sNodeSessionInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getNodeSessionInfo();
			sNodeSessionInfo.nNodeID = nFromNodeID;
			sNodeSessionInfo.nSessionID = pCookice->nSessionID;

			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceBeginRecv(pCookice->nTraceID, pHeader->nMessageID, nFromNodeID);
				
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromNodeID);
				DebugAst(pSerializeAdapter != nullptr);
				
				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
				bFilter = !callback(nFromNodeID, pMessage);
				
				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceEndRecv();
			}
			sNodeSessionInfo.nNodeID = 0;
			sNodeSessionInfo.nSessionID = 0;
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);
			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			if (pCookice->nActorID == 0)
			{
				SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(pCookice->nSessionID, true);
				if (nullptr == pResponseWaitInfo)
					return;

				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceSend(pResponseWaitInfo->nTraceID, pResponseWaitInfo->nMessageID, pResponseWaitInfo->nToID, pResponseWaitInfo->nBeginTime);
				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceBeginRecv(pResponseWaitInfo->nTraceID, pHeader->nMessageID, nFromNodeID);
				Defer(delete pResponseWaitInfo);

				if (pCookice->nResult == eRRT_OK)
				{
					CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromNodeID);
					DebugAst(pSerializeAdapter != nullptr);

					CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
					pResponseWaitInfo->callback(pMessage, eRRT_OK);
				}
				else if (pCookice->nResult != eRRT_OK)
				{
					pResponseWaitInfo->callback(nullptr, (EResponseResultType)pCookice->nResult);
				}
				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceEndRecv();
			}
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
		{
			const gate_forward_cookice* pCookice = reinterpret_cast<const gate_forward_cookice*>(pData);
			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			
			SClientSessionInfo session(nFromNodeID, pCookice->nSessionID);

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getGateForwardCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceBeginRecv(pCookice->nTraceID, pHeader->nMessageID, nFromNodeID);
				
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromNodeID);
				DebugAst(pSerializeAdapter != nullptr);

				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
				bFilter = !callback(session, pMessage);
				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceEndRecv();
			}
		}

		if (bFilter)
			return;

		const std::vector<GlobalAfterFilter>& vecGlobalAfterFilter = CCoreServiceAppImpl::Inst()->getGlobalAfterFilter();
		for (size_t i = 0; i < vecGlobalAfterFilter.size(); ++i)
		{
			vecGlobalAfterFilter[i](nFromSocketID, nFromNodeID, nMessageType, pData, nSize);
		}
	}
}