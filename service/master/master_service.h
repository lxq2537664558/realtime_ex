#pragma once

#include "libCoreCommon/service_base.h"

#include "service_registry.h"
#include "node_connection_factory.h"

class CMasterService :
	public core::CServiceBase
{
public:
	CMasterService();
	virtual ~CMasterService();

	CServiceRegistry*				getServiceRegistry() const;
	uint32_t						getMasterID() const;
	base::CWriteBuf&				getWriteBuf();

	virtual core::CProtobufFactory*	getProtobufFactory() const;

	virtual void					release();

private:
	virtual bool					onInit();
	virtual void					onFrame();
	virtual void					onQuit();

private:
	uint32_t				m_nMasterID;
	CServiceRegistry*		m_pServiceRegistry;
	CNodeConnectionFactory*	m_pNodeConnectionFactory;
	base::CWriteBuf			m_writeBuf;
};