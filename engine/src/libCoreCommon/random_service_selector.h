#pragma once

#include "service_selector.h"

namespace core
{
	class CRandomServiceSelector :
		public CServiceSelector
	{
	public:
		CRandomServiceSelector() { }
		virtual ~CRandomServiceSelector() { }

		virtual uint32_t	getType() const { return eSST_Random; }
		virtual uint32_t	select(const std::string& szServiceType, uint64_t nContext);
	};
}