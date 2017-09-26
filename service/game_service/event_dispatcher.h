#pragma once

#include <stdint.h>

#include <map>
#include <vector>

class IEventHandler;
class IEvent;
class CEventDispatcher
{
public:
	CEventDispatcher();
	~CEventDispatcher();

	void	registerHandler(uint32_t nType, IEventHandler* pHander);
	void	unregisterHandler(IEventHandler* pHander);
	void	raiseEvent(IEvent* pEvent);

private:
	std::map<uint32_t, std::vector<IEventHandler*>>	m_mapEventHandler;
	bool											m_bDispatch;
};
