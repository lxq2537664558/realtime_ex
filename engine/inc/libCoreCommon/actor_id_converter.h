#pragma once
#include "libBaseCommon\noncopyable.h"

namespace core
{
	class CActorIDConverter :
		public base::noncopyable
	{
	public:
		CActorIDConverter() { }
		virtual ~CActorIDConverter() { }

		virtual uint16_t convertToServiceID(uint64_t nActorID) = 0;
	};
}