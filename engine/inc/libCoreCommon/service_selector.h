#pragma once
#include <stdint.h>
#include <string>

#include "core_common.h"

namespace core
{
	class CServiceSelector
	{
	public:
		virtual ~CServiceSelector() { }

		virtual uint32_t	select(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nContext) = 0;
	};
}