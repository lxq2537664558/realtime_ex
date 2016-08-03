#include "stdafx.h"
#include "coroutine.h"
#include "coroutine_impl.h"
#include "base_app_impl.h"
#include "coroutine_mgr.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{

	namespace coroutine
	{
		void resume(uint64_t nID, uint64_t nContext)
		{
			CCoroutineImpl* pCoroutineImpl = CBaseAppImpl::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
			{
				PrintWarning("invalid coroutine id");
				return;
			}

			pCoroutineImpl->resume(nContext);
		}

		uint64_t yield()
		{
			return CBaseAppImpl::Inst()->getCoroutineMgr()->getCurrentCoroutine()->yield(eCYT_Normal);
		}

		uint32_t getState(uint64_t nID)
		{
			CCoroutineImpl* pCoroutineImpl = CBaseAppImpl::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return eCS_DEAD;

			return pCoroutineImpl->getState();
		}

		uint64_t getCurrentID()
		{
			return CBaseAppImpl::Inst()->getCoroutineMgr()->getCurrentCoroutine()->getCoroutineID();
		}

		uint64_t start(std::function<void(uint64_t)> fn)
		{
			CCoroutineImpl* pCoroutineImpl = CBaseAppImpl::Inst()->getCoroutineMgr()->startCoroutine(fn);
			if (nullptr == pCoroutineImpl)
				return 0;

			return pCoroutineImpl->getCoroutineID();
		}

		void close(uint64_t nID)
		{

		}

		void sendMessage(uint64_t nID, void* pData)
		{
			CCoroutineImpl* pCoroutineImpl = CBaseAppImpl::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return;

			pCoroutineImpl->sendMessage(pData);
		}

		void* recvMessage(uint64_t nID)
		{
			CCoroutineImpl* pCoroutineImpl = CBaseAppImpl::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return nullptr;

			return pCoroutineImpl->recvMessage();
		}

	}
}