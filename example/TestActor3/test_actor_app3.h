#pragma once

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/core_service_app.h"

class CTestActorApp3
	: public core::CCoreServiceApp
{
public:
	CTestActorApp3();
	virtual ~CTestActorApp3();

	static CTestActorApp3* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();
};