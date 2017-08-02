#pragma once

#include "service_selector.h"

namespace core
{
	class CHashServiceSelector :
		public CServiceSelector
	{
	public:
		CHashServiceSelector() { }
		virtual ~CHashServiceSelector() { }

		virtual uint32_t	getType() const { return eSST_Hash; }
		virtual uint32_t	select(const std::string& szServiceType, uint64_t nContext);
	};
}