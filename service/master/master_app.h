#pragma once
#include "libCoreCommon/base_app.h"

#include "service_registry.h"

class CNodeConnectionFactory;
class CMasterApp
	: public core::CBaseApp
{
public:
	CMasterApp();
	virtual ~CMasterApp();

	static CMasterApp* Inst();

	virtual bool	onInit();
	virtual bool	onProcess();
	virtual void	onDestroy();

	CServiceRegistry*	getServiceRegistry() const;
	uint32_t			getMasterID() const;
	base::CWriteBuf&	getWriteBuf();

private:
	uint32_t				m_nMasterID;
	CServiceRegistry*		m_pServiceRegistry;
	CNodeConnectionFactory*	m_pNodeConnectionFactory;
	base::CWriteBuf			m_writeBuf;
};