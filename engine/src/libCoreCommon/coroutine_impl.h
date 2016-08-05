#pragma once

#include "libBaseCommon/noncopyable.h"

#include "coroutine.h"

#include <functional>
#include <list>

#ifndef _WIN32
#include <ucontext.h>
#endif

namespace core
{
	class CCoroutineMgr;
	class CCoroutineImpl :
		public base::noncopyable
	{
	public:
		CCoroutineImpl();
		~CCoroutineImpl();

		bool		init(uint64_t nID, std::function<void(uint64_t)> callback);
		uint64_t	yield(bool bNormal);
		void		resume(uint64_t nContext);
		uint32_t	getState() const;
		void		setState(uint32_t nState);
		uint64_t	getCoroutineID() const;
		void		sendMessage(void* pData);
		void*		recvMessage();

		void		setCallback(std::function<void(uint64_t)> callback);
		
	private:
		void		saveStack();

#ifdef _WIN32
		static void	onCallback(void* pParm);
#else
		static void	onCallback(uint32_t nParm1, uint32_t nParm2);
#endif
	private:
		uint64_t						m_nID;
		std::function<void(uint64_t)>	m_callback;
		uint64_t						m_nContext;
#ifdef _WIN32
		void*							m_hHandle;
#else
		ucontext_t						m_ctx;
		char*							m_pStack;
		uint32_t						m_nStackSize;
		uint32_t						m_nStackCap;
#endif
		ECoroutineState					m_eState;
		CCoroutineImpl*					m_pParentCoroutine;
		std::list<void*>				m_listMessage;
	};
}