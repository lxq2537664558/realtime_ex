#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/actor_base.h"
#include "libCoreCommon/ticker.h"

using namespace std;
using namespace core;
using namespace base;

class CTestActor0 :
	public CActorBase
{
public:
	CTestActor0();
	virtual ~CTestActor0();

private:
	virtual void	onInit(const std::string& szContext);
	virtual void	onDestroy();
	virtual void	release();
};