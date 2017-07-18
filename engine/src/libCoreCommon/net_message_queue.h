#pragma once
#include <condition_variable>
#include <mutex>
#include <vector>

#include "libBaseCommon/circle_queue.h"
#include "core_common.h"

namespace core
{

	class CNetMessageQueue :
		public base::noncopyable
	{
	public:
		CNetMessageQueue();
		virtual ~CNetMessageQueue();

		void	send(const SMessagePacket& sMessagePacket);
		void	recv(std::vector<SMessagePacket>& vecMessagePacket);

	private:
		base::CCircleQueue<SMessagePacket, false>	
								m_queue;
		std::mutex				m_lock;
	};
}