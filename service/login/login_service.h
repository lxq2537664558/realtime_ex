#pragma once
#include "libCoreCommon/service_base.h"

#include "login_client_connection_factory.h"
#include "login_client_message_dispatcher.h"
#include "login_client_message_handler.h"

#include "libCoreCommon/json_protobuf_factory.h"

class CLoginService :
	public core::CServiceBase
{
public:
	CLoginService();
	virtual ~CLoginService();

	CLoginClientMessageDispatcher*	getLoginClientMessageDispatcher() const;

	virtual core::CProtobufFactory*	getServiceProtobufFactory() const;
	virtual core::CProtobufFactory*	getForwardProtobufFactory() const;
	
	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

private:
	CLoginClientConnectionFactory*		m_pLoginClientConnectionFactory;
	CLoginClientMessageDispatcher*		m_pLoginClientMessageDispatcher;
	CLoginClientMessageHandler*			m_pLoginClientMessageHandler;

	core::CNormalProtobufFactory*		m_pNormalProtobufFactory;
	core::CJsonProtobufFactory*			m_pJsonProtobufFactory;
};