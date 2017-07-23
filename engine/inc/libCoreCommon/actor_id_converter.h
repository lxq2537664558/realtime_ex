#pragma once
#include "libBaseCommon\noncopyable.h"

namespace core
{
	class CActorIDConverter :
		public base::noncopyable
	{
	public:
		virtual ~CActorIDConverter() { }

		virtual uint32_t convertToServiceID(uint64_t nActorID) = 0;
	};
}