#pragma once

#include "base_actor.h"

namespace core
{
	class CBaseActorFactory :
		public base::noncopyable
	{
	public:
		CBaseActorFactory() { }
		virtual ~CBaseActorFactory() { }

		virtual CBaseActor*	createBaseActor() = 0;
	};
}