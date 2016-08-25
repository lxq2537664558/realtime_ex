#pragma once
#include "libCoreCommon/base_app.h"

#include "node_mgr.h"

class CNodeConnectionFactory;
class CMasterApp
	: public core::CBaseApp
{
public:
	CMasterApp();
	virtual ~CMasterApp();

	static CMasterApp* Inst();

	virtual bool	onInit();
	virtual void	onDestroy();

	virtual void	onQuit();

	CNodeMgr*		getNodeMgr() const;

private:
	CNodeMgr*				m_pNodeMgr;
	CNodeConnectionFactory*	m_pNodeConnectionFactory;
};