#include "stdafx.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_app_impl.h"
#include "core_service_kit_define.h"
#include "coroutine.h"

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

	void CMessageDispatcher::dispatch(uint64_t nFromSocketID, uint16_t nFromServiceID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		bool bFilter = false;
		const std::vector<GlobalBeforeFilter>& vecGlobalBeforeFilter = CCoreServiceAppImpl::Inst()->getGlobalBeforeFilter();
		for (size_t i = 0; i < vecGlobalBeforeFilter.size(); ++i)
		{
			if (!vecGlobalBeforeFilter[i](nFromSocketID, nFromServiceID, nMessageType, pData, nSize))
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

			SServiceSessionInfo& sServiceSessionInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getServiceSessionInfo();
			sServiceSessionInfo.nServiceID = nFromServiceID;
			sServiceSessionInfo.nSessionID = pCookice->nSessionID;

			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromServiceID);
				DebugAst(pSerializeAdapter != nullptr);
				
				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
				bFilter = !callback(nFromServiceID, pMessage);
			}
			sServiceSessionInfo.nServiceID = 0;
			sServiceSessionInfo.nSessionID = 0;
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

				Defer(delete pResponseWaitInfo);

				if (pCookice->nResult == eRRT_OK)
				{
					CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromServiceID);
					DebugAst(pSerializeAdapter != nullptr);

					CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
					pResponseWaitInfo->callback(pMessage, eRRT_OK);
				}
				else if (pCookice->nResult != eRRT_OK)
				{
					pResponseWaitInfo->callback(nullptr, (EResponseResultType)pCookice->nResult);
				}
			}
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
		{
			const gate_forward_cookice* pCookice = reinterpret_cast<const gate_forward_cookice*>(pData);
			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			
			SClientSessionInfo session(nFromServiceID, pCookice->nSessionID);

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getGateForwardCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromServiceID);
				DebugAst(pSerializeAdapter != nullptr);

				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
				bFilter = !callback(session, pMessage);
			}
		}

		if (bFilter)
			return;

		const std::vector<GlobalAfterFilter>& vecGlobalAfterFilter = CCoreServiceAppImpl::Inst()->getGlobalAfterFilter();
		for (size_t i = 0; i < vecGlobalAfterFilter.size(); ++i)
		{
			vecGlobalAfterFilter[i](nFromSocketID, nFromServiceID, nMessageType, pData, nSize);
		}
	}
}