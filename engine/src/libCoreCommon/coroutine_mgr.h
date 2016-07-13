#pragma once
#include "libBaseCommon/noncopyable.h"

#include "coroutine_impl.h"
#include "ticker.h"

#include <map>
#include <list>

namespace core
{
	class CCoroutineMgr :
		public base::noncopyable
	{
	public:
		CCoroutineMgr();
		~CCoroutineMgr();

		bool			init(uint32_t nMainStackSize);

		char*			getMainStack() const;
		uint32_t		getMainStackSize() const;

		CCoroutineImpl*	startCoroutine(std::function<void(uint64_t)> fn);
		CCoroutineImpl*	getCoroutine(uint64_t nID) const;
		void			recycleCoroutine(CCoroutineImpl* pCoroutineImpl);

		void			setCurrentCoroutine(CCoroutineImpl* pCoroutineImpl);
		CCoroutineImpl*	getCurrentCoroutine() const;
		
	private:
		uint64_t								m_nNextCoroutineID;
		std::map<uint64_t, CCoroutineImpl*>		m_mapCoroutineImpl;
		std::list<CCoroutineImpl*>				m_listRecycleCoroutineImpl;
		CCoroutineImpl*							m_pCurrentCoroutine;
		CCoroutineImpl*							m_pRootCoroutine;
		char*									m_pMainStack;
		uint32_t								m_nMainStackSize;
	};
}