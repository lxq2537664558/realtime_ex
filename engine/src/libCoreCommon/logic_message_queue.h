#pragma once
#include <condition_variable>
#include <mutex>
#include <vector>

#include "libBaseCommon/circle_queue.h"
#include "core_common.h"

namespace core
{

	class CLogicMessageQueue
	{
	public:
		CLogicMessageQueue();
		virtual ~CLogicMessageQueue();

		bool	init();
		void	send(const SMessagePacket& sMessagePacket);
		void	recv(std::vector<SMessagePacket>& vecMessagePacket);

		bool	empty();

	private:
		base::CCircleQueue<SMessagePacket, false>	
								m_queue;
		std::mutex				m_lock;
	};
}