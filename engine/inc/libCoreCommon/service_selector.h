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

		virtual uint32_t	getType() const = 0;
		virtual uint32_t	select(const std::string& szServiceType, uint64_t nContext) = 0;
	};
}