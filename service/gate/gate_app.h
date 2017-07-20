#pragma once
#include "libCoreServiceKit/core_service_app.h"

#include "gate_session_mgr.h"

class CServiceConnectionFactory;
class CGateApp
	: public core::CCoreServiceApp
{
public:
	CGateApp();
	virtual ~CGateApp();

	static CGateApp* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();

	CGateSessionMgr*	getGateSessionMgr() const;

private:
	CGateSessionMgr*			m_pGateSessionMgr;
	CServiceConnectionFactory*	m_pServiceConnectionFactory;
};