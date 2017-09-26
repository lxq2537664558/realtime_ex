#pragma once
#include <stdint.h>

class IEvent
{
public:
	IEvent() { }
	virtual ~IEvent() { }

	virtual uint32_t getEventType() const = 0;
};