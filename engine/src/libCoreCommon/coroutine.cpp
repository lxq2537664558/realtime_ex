#include "stdafx.h"
#include "coroutine.h"
#include "coroutine_impl.h"
#include "coroutine_mgr.h"
#include "coroutine_thread.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{
	namespace coroutine
	{
		void resume(uint64_t nID, uint64_t nContext)
		{
			CCoroutineImpl* pCoroutineImpl = CCoroutineMgr::Inst()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
			{
				PrintWarning("resume error invalid coroutine id: {}", nID);
				return;
			}

			pCoroutineImpl->resume(nContext);
		}

		uint64_t yield()
		{
			CCoroutineImpl* pCoroutineImpl = getCoroutineThread()->getCurrentCoroutine();
			if (pCoroutineImpl == nullptr)
			{
				PrintWarning("yield error invalid current coroutine");
				return 0;
			}

			return pCoroutineImpl->yield();
		}

		void update()
		{
			CCoroutineThread* pCoroutineThread = getCoroutineThread();

			pCoroutineThread->update();
		}

		uint32_t getState(uint64_t nID)
		{
			CCoroutineImpl* pCoroutineImpl = CCoroutineMgr::Inst()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return eCS_DEAD;

			return pCoroutineImpl->getState();
		}

		uint64_t getCurrentID()
		{
			CCoroutineImpl* pCoroutineImpl = getCoroutineThread()->getCurrentCoroutine();
			if (nullptr == pCoroutineImpl)
				return 0;

			return pCoroutineImpl->getCoroutineID();
		}

		uint64_t create(uint32_t nStackSize, const std::function<void(uint64_t)>& fn)
		{
			CCoroutineImpl* pCoroutineImpl = CCoroutineMgr::Inst()->createCoroutine(nStackSize, fn);
			if (nullptr == pCoroutineImpl)
				return 0;

			return pCoroutineImpl->getCoroutineID();
		}

		void close(uint64_t nID)
		{
			CCoroutineImpl* pCoroutineImpl = CCoroutineMgr::Inst()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return;

			DebugAst(pCoroutineImpl->getState() == eCS_DEAD);
			// ипн╢й╣ож
		}

		void setLocalData(uint64_t nID, const char* szName, uint64_t nData)
		{
			CCoroutineImpl* pCoroutineImpl = CCoroutineMgr::Inst()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return;

			pCoroutineImpl->setLocalData(szName, nData);
		}

		bool getLocalData(uint64_t nID, const char* szName, uint64_t& nData)
		{
			CCoroutineImpl* pCoroutineImpl = CCoroutineMgr::Inst()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return false;

			return pCoroutineImpl->getLocalData(szName, nData);
		}

		void delLocalData(uint64_t nID, const char* szName)
		{
			CCoroutineImpl* pCoroutineImpl = CCoroutineMgr::Inst()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return;

			pCoroutineImpl->delLocalData(szName);
		}

		void init()
		{
			getCoroutineThread()->init();
		}

		uint32_t getCoroutineCount()
		{
			return CCoroutineMgr::Inst()->getCoroutineCount();
		}

		uint64_t getTotalStackSize()
		{
			return CCoroutineMgr::Inst()->getTotalStackSize();
		}
	}
}