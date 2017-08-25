#pragma once
#include <stdint.h>
#include <string>

class IPlayerAttributeModule
{
public:
	virtual ~IPlayerAttributeModule() {}

	virtual const std::string&	getName() const = 0;
	virtual uint32_t			getLv() const = 0;
	virtual uint64_t			getGold() const = 0;
	virtual uint64_t			getMoney() const = 0;
	virtual uint64_t			getVitality() const = 0;
};