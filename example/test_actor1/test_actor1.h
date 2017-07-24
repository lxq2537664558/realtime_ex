#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\actor_base.h"
#include "libCoreCommon\ticker.h"

using namespace std;
using namespace core;
using namespace base;

class CTestActor1 :
	public CActorBase
{
	DECLARE_OBJECT(CTestActor1)

public:
	CTestActor1();
	virtual ~CTestActor1();

private:
	virtual void	onInit(const std::string& szContext);
	virtual void	onDestroy();

	void			onTicker1(uint64_t nContext);
	void			onTicker2(uint64_t nContext);
	void			onTicker3(uint64_t nContext);

private:
	CTicker			m_ticker1;
	CTicker			m_ticker2;
	CTicker			m_ticker3;
	CServiceBase*	m_pServiceBase;
};