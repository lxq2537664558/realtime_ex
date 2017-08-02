#pragma once
#include <stdint.h>

namespace core
{
	class CActorIDConverter
	{
	public:
		virtual ~CActorIDConverter() { }

		virtual uint32_t convertToServiceID(uint64_t nActorID) = 0;
	};
}