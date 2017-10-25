#pragma once

#include "coroutine_impl.h"

#include "libBaseCommon/singleton.h"

#include <map>
#include <list>
#include <mutex>

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

		void	addDeadCoroutine(CCoroutineImpl* pCoroutineImpl);

		void	update();

		void*	getMainContext() const;

	private:
		std::list<CCoroutineImpl*>	m_listDeadCoroutine;
		CCoroutineImpl*				m_pCurrentCoroutine;
		void*						m_pMainContext;
#ifndef _WIN32
		char*						m_pMainStack;
		uint32_t					m_nMainStackSize;
		int32_t						m_nMainValgrindID;
#endif
	};

	CCoroutineThread* getCoroutineThread();
}