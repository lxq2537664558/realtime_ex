#pragma once
#include <stdint.h>

#include "libBaseCommon/noncopyable.h"

#include "event.h"

class CEventDispatcher;
class IEventHandler :
	public base::noncopyable
{
	friend class CEventDispatcher;

public:
	IEventHandler();
	virtual ~IEventHandler();

	virtual void onEvent(const IEvent* pEvent) { }

private:
	CEventDispatcher* m_pEventDispatcher;
};