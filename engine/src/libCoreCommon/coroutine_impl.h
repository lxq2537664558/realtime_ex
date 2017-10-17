#pragma once

#include "coroutine.h"

#include <map>

using namespace core::coroutine;

namespace core
{
#ifdef _WIN32

	struct context
	{
		int64_t rbx;
		int64_t rsp;
		int64_t rbp;
		int64_t r12;
		int64_t r13;
		int64_t r14;
		int64_t r15;
		int64_t rip;
		int64_t gs[4];
	};

#else

	struct context
	{
		int64_t rbx;
		int64_t rsp;
		int64_t rbp;
		int64_t r12;
		int64_t r13;
		int64_t r14;
		int64_t r15;
		int64_t rip;
		int64_t fp;
	};

#endif

	// 为了支持多线程，不支持共享栈，不然保存栈的时候需要加锁，但是此时正在执行的协程没法锁
	class CCoroutineThread;
	class CCoroutineImpl
	{
	public:
		CCoroutineImpl();
		~CCoroutineImpl();

		bool		init(uint64_t nID, uint32_t nStackSize, const std::function<void(uint64_t)>& callback);
		uint64_t	yield();
		void		resume(uint64_t nContext);
		uint32_t	getState() const;
		void		setState(uint32_t nState);
		uint64_t	getCoroutineID() const;
		void		setLocalData(const char* szName, uint64_t nData);
		bool		getLocalData(const char* szName, uint64_t& nData) const;
		void		delLocalData(const char* szName);

		uint32_t	getStackSize() const;

		void		setCallback(const std::function<void(uint64_t)>& callback);

	private:
#ifndef _WIN32
		static void	onCallback();
#endif
		static void	onCallback(void* pParm);

	private:
		uint64_t						m_nID;
		std::function<void(uint64_t)>	m_callback;
		uint64_t						m_nContext;
		ECoroutineState					m_eState;
		std::map<std::string, uint64_t>	m_mapLocalData;
		void*							m_pContext;
		uintptr_t						m_nStackSize;

#ifndef _WIN32
		char*							m_pStack;
		uintptr_t						m_nStackCap;
		int32_t							m_nValgrindID;
#endif
	};
}