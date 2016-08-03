#pragma once

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/core_service_app.h"

class CTestActorApp1
	: public core::CCoreServiceApp
{
public:
	CTestActorApp1();
	virtual ~CTestActorApp1();

	static CTestActorApp1* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();
};