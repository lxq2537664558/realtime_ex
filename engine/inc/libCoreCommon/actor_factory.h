#pragma once

#include <string>

namespace core
{
	class CActorBase;
	class CActorFactory
	{
	public:
		virtual ~CActorFactory() { }

		virtual CActorBase*	createActor(const std::string& szType) = 0;
	};
}