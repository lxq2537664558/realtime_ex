#include "stdafx.h"
#include "net_message_queue.h"
#include "net_runnable.h"
#include "core_connection_mgr.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/profiling.h"


namespace core
{
	CNetMessageQueue::CNetMessageQueue()
	{
	}

	CNetMessageQueue::~CNetMessageQueue()
	{
	}

	void CNetMessageQueue::send(const SMessagePacket& sMessagePacket)
	{
		PROFILING_GUARD(CNetMessageQueue::send)
		std::unique_lock<std::mutex> guard(this->m_lock);

		this->m_vecMessagePacket.push_back(sMessagePacket);
		CNetRunnable::Inst()->getCoreConnectionMgr()->wakeup();
	}

	void CNetMessageQueue::recv(std::vector<SMessagePacket>& vecMessagePacket)
	{
		PROFILING_GUARD(CNetMessageQueue::recv)
		vecMessagePacket.clear();

		std::unique_lock<std::mutex> guard(this->m_lock);
		
		std::swap(vecMessagePacket, this->m_vecMessagePacket);
	}
}