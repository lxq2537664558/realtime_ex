#pragma once

#include "actor_base.h"

namespace core
{
	class CActorBaseFactory :
		public base::noncopyable
	{
	public:
		CActorBaseFactory() { }
		virtual ~CActorBaseFactory() { }

		virtual CActorBase*	createActorBase() = 0;
	};
}