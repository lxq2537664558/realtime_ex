#include "event_handler.h"
#include "event_dispatcher.h"

IEventHandler::IEventHandler()
{
	this->m_pEventDispatcher = nullptr;
}

IEventHandler::~IEventHandler()
{
	if( this->m_pEventDispatcher != nullptr)
		this->m_pEventDispatcher->unregisterHandler(this);
}