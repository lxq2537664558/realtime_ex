#include "event_dispatcher.h"
#include "event_handler.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/logger.h"

CEventDispatcher::CEventDispatcher()
	: m_bDispatch(false)
{

}

CEventDispatcher::~CEventDispatcher()
{

}

void CEventDispatcher::registerHandler(uint32_t nType, IEventHandler* pHander)
{
	DebugAst(nullptr != pHander);

	if (this->m_bDispatch)
	{
		PrintWarning("dispatching can't register handler");
		return;
	}

	if( pHander->m_pEventDispatcher != nullptr && pHander->m_pEventDispatcher != this)
	{
		PrintWarning("regist event handler to two event dispatcher");
		return;
	}

	pHander->m_pEventDispatcher = this;
	auto& vecHandler = this->m_mapEventHandler[nType];
	vecHandler.push_back(pHander);
}

void CEventDispatcher::unregisterHandler(IEventHandler* pHander)
{
	DebugAst(pHander != nullptr);

	if (this->m_bDispatch)
	{
		PrintWarning("dispatching can't register handler");
		return;
	}

	for( auto iter = this->m_mapEventHandler.begin(); iter != this->m_mapEventHandler.end(); ++iter )
	{
		auto& vecHandler = iter->second;
		for( int32_t i = (int32_t)(vecHandler.size()-1); i >= 0; --i )
		{
			if( vecHandler[i] == pHander )
				vecHandler.erase(vecHandler.begin()+i );
		}
	}
}

void CEventDispatcher::raiseEvent(IEvent* pEvent)
{
	DebugAst(pEvent != nullptr);

	auto iter = this->m_mapEventHandler.find(pEvent->getEventType());
	if (iter == this->m_mapEventHandler.end())
		return;

	this->m_bDispatch = true;
	auto& vecHandler = iter->second;
	for( size_t i = 0; i < vecHandler.size(); ++i )
	{
		if( vecHandler[i] == nullptr )
			continue;

		vecHandler[i]->onEvent(pEvent);
	}
	this->m_bDispatch = false;
}

