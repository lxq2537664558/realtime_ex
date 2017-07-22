#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\ticker.h"

#include "test_actor1.h"

#include <map>

using namespace std;
using namespace core;
using namespace base;

class CTestActorService1 :
	public CServiceBase
{
	DECLARE_OBJECT(CTestService1)

public:
	CTestActorService1();
	virtual ~CTestActorService1();

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

	void			onServiceConnect(uint32_t nServiceID);
	void			onServiceDisconnect(uint32_t nServiceID);

private:
	CTicker						m_ticker1;
	CTicker						m_ticker2;
	std::map<uint32_t, bool>	m_mapConnectFlag;

	CTestActor1*				m_pTestActor1;
};