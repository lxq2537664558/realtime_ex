#pragma once
#include "libCoreCommon/service_base.h"

#include "client_connection_factory.h"
#include "client_message_dispatcher.h"
#include "client_message_handler.h"

class CLoginService :
	public core::CServiceBase
{
public:
	CLoginService();
	virtual ~CLoginService();

	CClientMessageDispatcher*		getClientMessageDispatcher() const;
	virtual core::CProtobufFactory*	getProtobufFactory() const;
	
	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

private:
	CClientConnectionFactory*		m_pClientConnectionFactory;
	CClientMessageDispatcher*		m_pClientMessageDispatcher;
	CClientMessageHandler*			m_pClientMessageHandler;
	core::CDefaultProtobufFactory*	m_pDefaultProtobufFactory;
};