#pragma once

#include "coroutine_impl.h"

#include "libBaseCommon/singleton.h"

#include <map>
#include <list>
#include <mutex>

namespace core
{
	class CCoroutineMgr :
		public base::CSingleton<CCoroutineMgr>
	{
	public:
		CCoroutineMgr();
		~CCoroutineMgr();

		bool			init();
		CCoroutineImpl*	createCoroutine(uint32_t nStackSize, const std::function<void(uint64_t)>& callback);
		CCoroutineImpl*	getCoroutine(uint64_t nID);
		uint32_t		getCoroutineCount();
		uint64_t		getTotalStackSize();
		bool			addRecycleCoroutine(CCoroutineImpl* pCoroutineImpl);

#ifndef _WIN32
		static char*	allocStack(uint32_t& nStackSize, uint32_t& nValgrindID);
		static void		freeStack(char* pStack, uint32_t nStackSize, uint32_t nValgrindID);
#endif

	private:
		int64_t									m_nTotalStackSize;
		uint64_t								m_nNextCoroutineID;
		std::map<uint64_t, CCoroutineImpl*>		m_mapCoroutine;
		std::list<CCoroutineImpl*>				m_listRecycleCoroutine;
		std::mutex								m_lock;
	};
}