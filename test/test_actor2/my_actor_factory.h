#pragma once

#include "libCoreCommon/actor_factory.h"

class CMyActorFactory :
	public core::CActorFactory
{
public:
	CMyActorFactory();
	virtual ~CMyActorFactory();

	virtual core::CActorBase*	createActor(const std::string& szType);
};