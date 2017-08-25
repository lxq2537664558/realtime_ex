#pragma once
#include "libCoreCommon/service_base.h"

#include "gate_client_connection_factory.h"
#include "gate_client_session_mgr.h"
#include "gate_client_message_dispatcher.h"
#include "gate_client_message_handler.h"
#include "gate_service_message_handler.h"

#include "libCoreCommon/json_protobuf_factory.h"

class CGateService :
	public core::CServiceBase
{
public:
	CGateService();
	virtual ~CGateService();

	CGateClientSessionMgr*			getGateClientSessionMgr() const;
	CGateClientMessageDispatcher*	getGateClientMessageDispatcher() const;
	CGateClientMessageHandler*		getGateClientMessageHandler() const;

	virtual core::CProtobufFactory*	getServiceProtobufFactory() const;
	virtual core::CProtobufFactory*	getForwardProtobufFactory() const;

	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

	void							onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void							onNotifyOnlineCount(uint64_t nContext);

private:
	CGateClientConnectionFactory*	m_pGateClientConnectionFactory;
	CGateClientSessionMgr*			m_pGateClientSessionMgr;
	CGateClientMessageDispatcher*	m_pGateClientMessageDispatcher;
	CGateClientMessageHandler*		m_pGateClientMessageHandler;
	CGateServiceMessageHandler*		m_pGateServiceMessageHandler;

	core::CNormalProtobufFactory*	m_pNormalProtobufFactory;
	core::CJsonProtobufFactory*		m_pJsonProtobufFactory;

	core::CTicker					m_tickerNotifyOnlineCount;
	std::string						m_szAddr;
};