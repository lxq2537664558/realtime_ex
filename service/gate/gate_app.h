#pragma once
#include "libCoreCommon/core_app.h"

#include "service_mgr.h"

class CGateApp
	: public core::CCoreApp
{
public:
	CGateApp();
	virtual ~CGateApp();

	static CGateApp* Inst();

	virtual bool	onInit();
	virtual void	onDestroy();

	virtual void	onQuit();

	CServiceMgr*	getServiceMgr() const;

private:
	void			onCheckConnect(uint64_t nContext);

private:
	std::string		m_szMasterHost;
	uint16_t		m_nMasterPort;
	CServiceMgr*	m_pServiceMgr;
	core::CTicker	m_tickCheckConnect;
};