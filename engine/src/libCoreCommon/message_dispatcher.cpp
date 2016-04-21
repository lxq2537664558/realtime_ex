#include "stdafx.h"
#include "message_dispatcher.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

static google::protobuf::Message* create_protobuf_message(const std::string& szMessageName)
{
	const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
	if (pDescriptor == nullptr)
		return nullptr;

	const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
	if (pProtoType == nullptr)
		return nullptr;

	return pProtoType->New();
}

static google::protobuf::Message* unserialize_protobuf_message_from_buf(const std::string& szMessageName, const core::message_header* pHeader)
{
	google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
	if (nullptr == pMessage)
		return nullptr;


	if (!pMessage->ParseFromArray(pHeader + 1, pHeader->nMessageSize - sizeof(core::message_header)))
	{
		SAFE_DELETE(pMessage);
		return nullptr;
	}

	return pMessage;
}

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

		// 先前置过滤器过一遍，如果有一个返回false就直接跳过这个消息
		const std::vector<ServiceGlobalFilter>& vecServiceGlobalBeforeFilter = CCoreApp::Inst()->getMessageDirectory()->getGlobalBeforeFilter();
		for (size_t i = 0; i < vecServiceGlobalBeforeFilter.size(); ++i)
		{
			if (vecServiceGlobalBeforeFilter[i] != nullptr && !vecServiceGlobalBeforeFilter[i](szFromServiceName, nMessageType, pData, nSize))
				return;
		}

		if ((nMessageType&eMT_TYPE_MASK) == eMT_REQUEST)
		{
			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			SServiceSessionInfo& sServiceSessionInfo = CCoreApp::Inst()->getTransport()->getServiceSessionInfo();
			sServiceSessionInfo.szServiceName = szFromServiceName;
			sServiceSessionInfo.nSessionID = pCookice->nSessionID;

			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			const std::string& szMessageName = CCoreApp::Inst()->getMessageDirectory()->getMessageName(pHeader->nMessageID);
			ServiceCallback& callback = CCoreApp::Inst()->getMessageDirectory()->getCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf(szMessageName, pHeader);
				if (nullptr == pMessage)
					PrintWarning("create message error from_service_name: %s message_name: %s message_id: %d", szFromServiceName.c_str(), szMessageName.c_str(), pHeader->nMessageID);
				else
					callback(szFromServiceName, nMessageType, pMessage);

				SAFE_DELETE(pMessage);
			}
			sServiceSessionInfo.szServiceName.clear();
			sServiceSessionInfo.nSessionID = 0;
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);

			SResponseWaitInfo* pResponseWaitInfo = CCoreApp::Inst()->getTransport()->getResponseWaitInfo(pCookice->nSessionID, true);
			if (nullptr == pResponseWaitInfo)
			{
				PrintWarning("invalid session id from_service_name: %s session_id: "UINT64FMT, szFromServiceName.c_str(), pCookice->nSessionID);
				return;
			}

			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			const std::string& szMessageName = CCoreApp::Inst()->getMessageDirectory()->getMessageName(pHeader->nMessageID);

			if (pResponseWaitInfo->callback != nullptr)
			{
				google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf(szMessageName, pHeader);
				if (nullptr == pMessage)
					PrintWarning("create message error from_service_name: %s message_name: %s message_id: %d", szFromServiceName.c_str(), szMessageName.c_str(), pHeader->nMessageID);
				else
					pResponseWaitInfo->callback(nMessageType, pMessage, (EResponseResultType)pCookice->nResult);

				SAFE_DELETE(pMessage);
			}
			SAFE_DELETE(pResponseWaitInfo);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_FROM_GATE)
		{
			const gate_cookice* pCookice = reinterpret_cast<const gate_cookice*>(pData);

			// 剥掉cookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			const std::string& szMessageName = CCoreApp::Inst()->getMessageDirectory()->getMessageName(pHeader->nMessageID);

			SClientSessionInfo session(szFromServiceName, pCookice->nSessionID);

			GateClientCallback& callback = CCoreApp::Inst()->getMessageDirectory()->getGateClientCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf(szMessageName, pHeader);
				if (nullptr == pMessage)
					PrintWarning("create message error from_service_name: %s message_name: %s message_id: %d", szFromServiceName.c_str(), szMessageName.c_str(), pHeader->nMessageID);
				else
					callback(session, nMessageType, pMessage);

				SAFE_DELETE(pMessage);
			}
		}

		const std::vector<ServiceGlobalFilter>& vecServiceGlobalAfterFilter = CCoreApp::Inst()->getMessageDirectory()->getGlobalAfterFilter();
		for (size_t i = 0; i < vecServiceGlobalAfterFilter.size(); ++i)
		{
			if (vecServiceGlobalAfterFilter[i] != nullptr)
				vecServiceGlobalAfterFilter[i](szFromServiceName, nMessageType, pData, nSize);
		}
	}
}