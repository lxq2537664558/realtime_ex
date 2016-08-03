#pragma once

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/core_service_app.h"

class CTestServiceApp2
	: public core::CCoreServiceApp
{
public:
	CTestServiceApp2();
	virtual ~CTestServiceApp2();

	static CTestServiceApp2* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();
};