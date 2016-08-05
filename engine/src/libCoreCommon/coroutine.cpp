#include "stdafx.h"
#include "coroutine.h"
#include "coroutine_impl.h"
#include "core_app.h"
#include "coroutine_mgr.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{

	namespace coroutine
	{
		void resume(uint64_t nID, uint64_t nContext)
		{
			CCoroutineImpl* pCoroutineImpl = CCoreApp::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
			{
				PrintWarning("invalid coroutine id");
				return;
			}

			pCoroutineImpl->resume(nContext);
		}

		uint64_t yield()
		{
			return CCoreApp::Inst()->getCoroutineMgr()->getCurrentCoroutine()->yield(true);
		}

		uint32_t getState(uint64_t nID)
		{
			CCoroutineImpl* pCoroutineImpl = CCoreApp::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return eCS_DEAD;

			return pCoroutineImpl->getState();
		}

		uint64_t getCurrentID()
		{
			return CCoreApp::Inst()->getCoroutineMgr()->getCurrentCoroutine()->getCoroutineID();
		}

		uint64_t create(std::function<void(uint64_t)> fn)
		{
			CCoroutineImpl* pCoroutineImpl = CCoreApp::Inst()->getCoroutineMgr()->createCoroutine(fn);
			if (nullptr == pCoroutineImpl)
				return 0;

			return pCoroutineImpl->getCoroutineID();
		}

		void close(uint64_t nID)
		{
			CCoroutineImpl* pCoroutineImpl = CCoreApp::Inst()->getCoroutineMgr()->getCurrentCoroutine();
			if (nullptr == pCoroutineImpl)
				return;

			if (pCoroutineImpl->getCoroutineID() == nID)
			{
				PrintWarning("can't close current corotine");
				return;
			}

			pCoroutineImpl = CCoreApp::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return;
		}

		void sendMessage(uint64_t nID, void* pData)
		{
			CCoroutineImpl* pCoroutineImpl = CCoreApp::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return;

			pCoroutineImpl->sendMessage(pData);
		}

		void* recvMessage(uint64_t nID)
		{
			CCoroutineImpl* pCoroutineImpl = CCoreApp::Inst()->getCoroutineMgr()->getCoroutine(nID);
			if (nullptr == pCoroutineImpl)
				return nullptr;

			return pCoroutineImpl->recvMessage();
		}

	}
}