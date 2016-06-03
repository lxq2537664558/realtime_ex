#include "stdafx.h"
#include "gate_message_handler.h"
#include "gate_message_dispatcher.h"
#include "gate_session.h"
#include "gate_session_mgr.h"
#include "gate_app.h"

#include "proto_src/login_msg.pb.h"

#include "libCoreServiceKit/protobuf_helper.h"

CGateMessageHandler::CGateMessageHandler()
{

}

CGateMessageHandler::~CGateMessageHandler()
{

}

bool CGateMessageHandler::init()
{
	CGateMessageDispatcher::Inst()->registerCallback("gate.login_msg", std::bind(&CGateMessageHandler::onLogin, this, std::placeholders::_1, std::placeholders::_2));
	return true;
}

void CGateMessageHandler::onLogin(uint64_t nSocketID, const core::client_message_header* pHeader)
{
	DebugAst(pHeader != nullptr);

	google::protobuf::Message* pMessage = core::unserialize_protobuf_message_from_buf("gate.login_msg", pHeader + 1, pHeader->nMessageSize - sizeof(core::client_message_header));
	DebugAst(pMessage != nullptr);

	const gate::login_msg* pLoginMsg = dynamic_cast<const gate::login_msg*>(pMessage);
	DebugAst(pLoginMsg != nullptr);

	CGateSession* pGateSession = CGateApp::Inst()->getGateSessionMgr()->createSession(nSocketID, pLoginMsg->id());
}