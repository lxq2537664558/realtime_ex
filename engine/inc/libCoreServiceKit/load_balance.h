#pragma once
#include "libCoreCommon/core_common.h"

namespace core
{
	class ILoadBalance
	{
	public:
		virtual ~ILoadBalance() { }
		
		virtual const std::string&	getName() const = 0;
		virtual std::string			select(const std::string& szMessageName, uint64_t nSessionID, const std::string& szServiceGroup) = 0;
	};

	class CRandomLoadBalance :
		public ILoadBalance
	{
	public:
		CRandomLoadBalance();
		virtual ~CRandomLoadBalance();

		virtual const std::string&	getName() const;
		virtual std::string			select(const std::string& szMessageName, uint64_t nSessionID, const std::string& szServiceGroup);
	};
}