#pragma once

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/core_service_app.h"

class CTestActorApp2
	: public core::CCoreServiceApp
{
public:
	CTestActorApp2();
	virtual ~CTestActorApp2();

	static CTestActorApp2* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();
};