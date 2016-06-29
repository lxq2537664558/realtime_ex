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
	enum ECoroutineYieldType
	{
		eCYT_Normal,
		eCYT_Dead,
		eCYT_Sleep,
	};

	class CCoroutineMgr;
	class CCoroutineImpl :
		public base::noncopyable
	{
	public:
		CCoroutineImpl();
		~CCoroutineImpl();

		bool		init(uint64_t nID, std::function<void(uint64_t)> fn);
		uint64_t	yield(ECoroutineYieldType eType);
		void		resume(uint64_t nContext);
		uint32_t	getState() const;
		void		setState(uint32_t nState);
		uint64_t	getCoroutineID() const;
		void		sendMessage(void* pData);
		void*		recvMessage();
		
	private:
		void		saveStack();

		static void	onCallback(void* pParm);

	private:
		uint64_t						m_nID;
		std::function<void(uint64_t)>	m_fn;
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