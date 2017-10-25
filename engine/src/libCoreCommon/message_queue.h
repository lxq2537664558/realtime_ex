#pragma once
#include "libBaseCommon/noncopyable.h"

#include "core_common_define.h"

#include <mutex>
#include <list>

namespace core
{
	class CMessageQueue :
		public base::noncopyable
	{
	public:
		CMessageQueue();
		virtual ~CMessageQueue();

		virtual void	send(const SMessagePacket& sMessagePacket) = 0;
		virtual void	recv(std::list<SMessagePacket>& listMessagePacket) = 0;
		
		size_t			getSize();
		bool			isEmpty();

	protected:
		std::list<SMessagePacket>	m_listMessagePacket;
		std::mutex					m_lock;
	};
}