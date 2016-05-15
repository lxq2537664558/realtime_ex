#pragma once

#include "libCoreCommon/base_app.h"

class CTestServiceClientApp
	: public core::CBaseApp
{
public:
	CTestServiceClientApp();
	virtual ~CTestServiceClientApp();

	static CTestServiceClientApp* Inst();

	virtual bool		onInit();
	virtual void		onDestroy();

	virtual void		onQuit();
};