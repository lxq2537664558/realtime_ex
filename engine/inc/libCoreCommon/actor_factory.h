#pragma once

#include "actor_base.h"

namespace core
{
	class CActorFactory
	{
	public:
		virtual ~CActorFactory() { }

		virtual CActorBase*	createActor(const std::string& szType) = 0;
	};
}