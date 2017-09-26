#pragma once

#include "libCoreCommon/actor_factory.h"

class CPlayerFactory :
	public core::CActorFactory
{
public:
	CPlayerFactory();
	virtual ~CPlayerFactory();

	virtual core::CActorBase*	createActor(const std::string& szType);
};