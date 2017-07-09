#pragma once

#include "actor_base.h"

namespace core
{
	class CActorFactory :
		public base::noncopyable
	{
	public:
		CActorFactory() { }
		virtual ~CActorFactory() { }

		virtual CActorBase*	createActor() = 0;
	};
}