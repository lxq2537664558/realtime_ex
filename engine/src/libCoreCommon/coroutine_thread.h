#pragma once

#include "coroutine_impl.h"

#include "libBaseCommon/singleton.h"

namespace core
{
	class CCoroutineThread
	{
	public:
		CCoroutineThread();
		~CCoroutineThread();

		bool	init();

		void	setCurrentCoroutine(CCoroutineImpl* pCoroutineImpl);
		CCoroutineImpl*	
				getCurrentCoroutine() const;

		void*	getMainContext() const;

	private:
		CCoroutineImpl*	m_pCurrentCoroutine;
		void*			m_pMainContext;
#ifndef _WIN32
		char*			m_pMainStack;
		uint32_t		m_nMainStackSize;
		int32_t			m_nMainValgrindID;
#endif
	};

#ifdef _WIN32
	__declspec(noinline) CCoroutineThread* getCoroutineThread();
#else
	CCoroutineThread* getCoroutineThread() __attribute__((noinline));
#endif
}