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

		virtual uint32_t convert(const std::string& szServiceType, uint32_t nServiceID) = 0;
	};
}