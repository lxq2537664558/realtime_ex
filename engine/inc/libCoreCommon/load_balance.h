#pragma once

namespace core
{
	class CLoadBalance
	{
	public:
		virtual ~CLoadBalance() { }
		
		virtual std::string select(uint32_t nMessageID, uint64_t nContext) = 0;
	};
}