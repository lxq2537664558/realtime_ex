#include "stdafx.h"
#include "gate_message_dispatcher.h"
#include "connection_from_client.h"
#include "gate_session.h"
#include "gate_app.h"

#include "libCoreCommon\base_connection.h"
#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\base_app.h"

CGateMessageDispatcher::CGateMessageDispatcher()
{

}

CGateMessageDispatcher::~CGateMessageDispatcher()
{

}

bool CGateMessageDispatcher::init()
{
	return true;
}

void CGateMessageDispatcher::registCallback(uint32_t nMessageID, core::ClientCallback callback)
{
	this->m_mapClientCallback[nMessageID] = callback;
}

core::ClientCallback& CGateMessageDispatcher::getCallback(uint32_t nMessageID)
{
	auto iter = this->m_mapClientCallback.find(nMessageID);
	if (iter == this->m_mapClientCallback.end())
	{
		static core::ClientCallback s_callback;

		return s_callback;
	}

	return iter->second;
}

void CGateMessageDispatcher::dispatch(uint16_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(pData != nullptr);

	if ((nMessageType&eMT_TYPE_MASK) == eMT_CLIENT)
	{
		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);

		auto iter = this->m_mapClientCallback.find(pHeader->nMessageID);
		if (iter == this->m_mapClientCallback.end())
			return;

		core::ClientCallback& callback = iter->second;
		if (callback == nullptr)
			return;

		callback(pHeader);
	}
	else if ((nMessageType&eMT_TYPE_MASK) == eMT_TO_GATE)
	{
		DebugAst(nSize > sizeof(gate_cookice));

		const gate_cookice* pCookice = reinterpret_cast<const gate_cookice*>(pData);
		CGateSession* pGateSession = CGateApp::Inst()->getGateSessionMgr()->getSession(pCookice->nSessionID);
		if (nullptr == pGateSession)
			return;

		core::CBaseConnection*  pBaseConnection = CGateApp::Inst()->getBaseConnectionMgr()->getBaseConnection(pGateSession->getSocketID());
		if (nullptr == pBaseConnection)
			return;

		CConnectionFromClient* pConnectionFromClient = dynamic_cast<CConnectionFromClient*>(pBaseConnection);
		DebugAst(nullptr != pConnectionFromClient);

		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);
		int16_t nMessageSize = nSize - sizeof(gate_cookice);
		DebugAst(nMessageSize == pHeader->nMessageSize);
		if (nMessageSize < 0)
		{
			PrintWarning("invalid message size client_id: "UINT64FMT" message_id: %d message_size: %d", pCookice->nSessionID, pHeader->nMessageID, pHeader->nMessageSize);
			return;
		}

		// 消息长度打上protobuf标记
		if ((nMessageType&eMT_PROTOBUF) != 0)
			const_cast<core::message_header*>(pHeader)->nMessageSize |= eMT_PROTOBUF;

		pConnectionFromClient->send(eMT_CLIENT, pHeader, nMessageSize);
	}
}