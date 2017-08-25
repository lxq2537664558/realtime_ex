#pragma once
#include <stdint.h>

class IPlayerModuleEvent
{
public:
	virtual ~IPlayerModuleEvent() {};

public:
	virtual uint32_t getEventType() = 0;
};