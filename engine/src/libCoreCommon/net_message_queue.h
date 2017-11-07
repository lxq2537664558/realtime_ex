#pragma once

#include "message_queue.h"

namespace core
{
	class CNetMessageQueue :
		public CMessageQueue
	{
	public:
		CNetMessageQueue();
		virtual ~CNetMessageQueue();

		virtual void	send(const SMessagePacket& sMessagePacket);
		virtual void	recv(std::vector<SMessagePacket>& vecMessagePacket);
	};
}