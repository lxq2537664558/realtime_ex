#pragma once
#include "libBaseCommon\noncopyable.h"

#include <string>

namespace core
{
	class CServiceIDConverter :
		public base::noncopyable
	{
	public:
		CServiceIDConverter() { }
		virtual ~CServiceIDConverter() { }

		virtual uint16_t convert(const std::string& szServiceType, uint16_t nServiceID) = 0;
	};
}