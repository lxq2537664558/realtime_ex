#pragma once
#include "libCoreCommon\service_base.h"

#include "client_connection_factory.h"
#include "client_session_mgr.h"
#include "client_message_dispatcher.h"
#include "client_message_handler.h"

class CGateService :
	public core::CServiceBase
{
	DECLARE_OBJECT(CGateService)

public:
	CGateService();
	virtual ~CGateService();

	CClientSessionMgr*			getClientSessionMgr() const;
	CClientMessageDispatcher*	getClientMessageDispatcher() const;

private:
	virtual bool				onInit();
	virtual void				onFrame();
	virtual void				onQuit();

private:
	CClientConnectionFactory*		m_pClientConnectionFactory;
	CClientSessionMgr*				m_pClientSessionMgr;
	CClientMessageDispatcher*		m_pClientMessageDispatcher;
	CClientMessageHandler*			m_pClientMessageHandler;
	core::CDefaultProtobufFactory*	m_pDefaultProtobufFactory;
};