#include "message_queue.h"

#define _DEFAULT_MESSAGE_QUEUE 1024

namespace core
{
	CMessageQueue::CMessageQueue()
		: m_queue(_DEFAULT_MESSAGE_QUEUE)
	{

	}

	CMessageQueue::~CMessageQueue()
	{

	}

	size_t CMessageQueue::getSize()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		return this->m_queue.size();
	}

	bool CMessageQueue::isEmpty()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		return this->m_queue.size() == 0;
	}
}