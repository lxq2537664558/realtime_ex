#pragma once

#include "libCoreCommon\service_base.h"

#include "service_registry.h"
#include "node_connection_factory.h"

class CMasterService :
	public core::CServiceBase
{
	DECLARE_OBJECT(CMasterService)

public:
	CMasterService();
	virtual ~CMasterService();

	CServiceRegistry*	getServiceRegistry() const;
	uint32_t			getMasterID() const;
	base::CWriteBuf&	getWriteBuf();

private:
	virtual bool		onInit();
	virtual void		onFrame();
	virtual void		onQuit();

private:
	uint32_t				m_nMasterID;
	CServiceRegistry*		m_pServiceRegistry;
	CNodeConnectionFactory*	m_pNodeConnectionFactory;
	base::CWriteBuf			m_writeBuf;
};