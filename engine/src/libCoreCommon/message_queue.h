#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/circle_queue.h"

#include "core_common_define.h"

#include <mutex>

namespace core
{
	class CMessageQueue :
		public base::noncopyable
	{
	public:
		CMessageQueue();
		virtual ~CMessageQueue();

		virtual void	send(const SMessagePacket& sMessagePacket) = 0;
		virtual void	recv(std::vector<SMessagePacket>& vecMessagePacket) = 0;
		
		size_t			getSize();
		bool			isEmpty();

	protected:
		base::CCircleQueue<SMessagePacket, false>	m_queue;
		std::mutex									m_lock;
	};
}