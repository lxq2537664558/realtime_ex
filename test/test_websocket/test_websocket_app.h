#pragma once

#include "client_connection_factory.h"

#include "libCoreCommon/base_app.h"

class CTestWebsocketApp :
	public core::CBaseApp
{
public:
	CTestWebsocketApp();
	virtual ~CTestWebsocketApp();

	static CTestWebsocketApp*
					Inst();

private:
	virtual bool	onInit();
	virtual void	onDestroy();

private:
	CClientConnectionFactory*	m_pClientConnectionFactory;
};