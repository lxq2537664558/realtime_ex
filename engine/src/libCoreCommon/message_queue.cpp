#include "message_queue.h"

namespace core
{
	CMessageQueue::CMessageQueue()
	{

	}

	CMessageQueue::~CMessageQueue()
	{

	}

	size_t CMessageQueue::getSize()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		return this->m_listMessagePacket.size();
	}

	bool CMessageQueue::isEmpty()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		return this->m_listMessagePacket.empty();
	}
}