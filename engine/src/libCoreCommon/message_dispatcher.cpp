#include "stdafx.h"
#include "message_dispatcher.h"
#include "core_app.h"

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

	void CMessageDispatcher::dispatch(const std::string& szFromServiceName, uint16_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		// 先全局的过一遍，如果有一个返回false就直接跳过这个消息
		const std::vector<ServiceGlobalCallback>& vecServiceGlobalBeforeCallback = CCoreApp::Inst()->getMessageDirectory()->getServiceGlobalBeforeCallback();
		for (size_t i = 0; i < vecServiceGlobalBeforeCallback.size(); ++i)
		{
			if (vecServiceGlobalBeforeCallback[i] != nullptr && !vecServiceGlobalBeforeCallback[i](szFromServiceName, nMessageType, pData, nSize))
				return;
		}

		if ((nMessageType&eMT_TYPE_MASK) == eMT_REQUEST)
		{
			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			SServiceSessionInfo& sServiceSessionInfo = CCoreApp::Inst()->getMessageSend()->getServiceSessionInfo();
			sServiceSessionInfo.szServiceName = szFromServiceName;
			sServiceSessionInfo.nSessionID = pCookice->nSessionID;

			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			ServiceCallback& serviceCallback = CCoreApp::Inst()->getMessageDirectory()->getCallback(pHeader->nMessageID);
			if (serviceCallback != nullptr)
				serviceCallback(szFromServiceName, nMessageType, pHeader);

			sServiceSessionInfo.szServiceName.clear();
			sServiceSessionInfo.nSessionID = 0;
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);

			SResponseWaitInfo* pResponseWaitInfo = CCoreApp::Inst()->getMessageSend()->getResponseWaitInfo(pCookice->nSessionID, true);
			if (nullptr == pResponseWaitInfo)
			{
				PrintWarning("invalid session id from_service_name: %s session_id: "UINT64FMT, szFromServiceName.c_str(), pCookice->nSessionID);
				return;
			}

			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			if (pResponseWaitInfo->callback != nullptr)
				pResponseWaitInfo->callback(nMessageType, pHeader, (EResponseResultType)pCookice->nResult);

			SAFE_DELETE(pResponseWaitInfo);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_FROM_GATE)
		{
			const gate_cookice* pCookice = reinterpret_cast<const gate_cookice*>(pData);
			
			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			SClientSessionInfo session(szFromServiceName, pCookice->nSessionID);

			GateClientCallback& gateClientCallback = CCoreApp::Inst()->getMessageDirectory()->getGateClientCallback(pHeader->nMessageID);
			if (gateClientCallback != nullptr)
				gateClientCallback(session, nMessageType, pHeader);
		}

		const std::vector<ServiceGlobalCallback>& vecServiceGlobalAfterCallback = CCoreApp::Inst()->getMessageDirectory()->getServiceGlobalAfterCallback();
		for (size_t i = 0; i < vecServiceGlobalAfterCallback.size(); ++i)
		{
			if (vecServiceGlobalAfterCallback[i] != nullptr)
				vecServiceGlobalAfterCallback[i](szFromServiceName, nMessageType, pData, nSize);
		}
	}
}