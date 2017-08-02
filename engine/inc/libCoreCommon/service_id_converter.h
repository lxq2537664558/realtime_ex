#pragma once

#include <stdint.h>
#include <string>

namespace core
{
	class CServiceIDConverter
	{
	public:
		virtual ~CServiceIDConverter() { }

		virtual uint32_t convert(uint32_t nServiceID) = 0;
	};
}