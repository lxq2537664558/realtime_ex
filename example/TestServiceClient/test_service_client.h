#pragma once

#include "libCoreCommon/base_app.h"

class CTestServiceApp2
	: public core::CBaseApp
{
public:
	CTestServiceApp2();
	virtual ~CTestServiceApp2();

	static CTestServiceApp2* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();
};