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
	uint16_t			getMasterID() const;

private:
	uint16_t				m_nMasterID;
	CServiceRegistry*		m_pServiceRegistry;
	CNodeConnectionFactory*	m_pNodeConnectionFactory;
};