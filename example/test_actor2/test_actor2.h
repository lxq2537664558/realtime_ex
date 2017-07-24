#pragma once
#include "libCoreCommon\service_base.h"
#include "libCoreCommon\actor_base.h"
#include "libCoreCommon\ticker.h"

using namespace std;
using namespace core;
using namespace base;

class CTestActor2 :
	public CActorBase
{
	DECLARE_OBJECT(CTestActor2)

public:
	CTestActor2();
	virtual ~CTestActor2();

private:
	virtual void	onInit(const std::string& szContext);
	virtual void	onDestroy();

private:
	CServiceBase*	m_pServiceBase;
};