#pragma once

#include "libCoreCommon/base_app.h"

class CTestServiceApp1
	: public core::CBaseApp
{
public:
	CTestServiceApp1();
	virtual ~CTestServiceApp1();

	static CTestServiceApp1* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();
};