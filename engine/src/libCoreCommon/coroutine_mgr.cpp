#include "coroutine_mgr.h"
#include "core_app.h"

#ifndef _WIN32
#include <unistd.h>
#include <sys/mman.h>
#define __USE_VALGRIND__
#ifdef __USE_VALGRIND__
#include "valgrind/valgrind.h"
#endif
#endif

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/process_util.h"

#define _MAX_CO_RECYCLE_COUNT	100

namespace core
{
	CCoroutineImpl* CCoroutineMgr::createCoroutine(uint32_t nStackSize, const std::function<void(uint64_t)>& callback)
	{
		DebugAstEx(callback != nullptr, nullptr);

#ifdef _WIN32
		if (nStackSize == 0)
			nStackSize = CCoreApp::Inst()->getCoroutineStackSize();
#endif

		uint32_t nPageSize = base::process_util::getPageSize();
		nStackSize = (nStackSize + nPageSize - 1) / nPageSize * nPageSize;
		
		std::unique_lock<std::mutex> guard(this->m_lock);

		for (auto iter = this->m_listRecycleCoroutine.begin(); iter != this->m_listRecycleCoroutine.end(); ++iter)
		{
			CCoroutineImpl* pCoroutineImpl = *iter;
			// 栈相差不大的，就重复使用
			if (std::abs((int32_t)pCoroutineImpl->getStackSize() - (int32_t)nStackSize) <= (int32_t)(10 * nPageSize))
			{
				pCoroutineImpl->setCallback(callback);
				pCoroutineImpl->setState(eCS_SUSPEND);
				this->m_listRecycleCoroutine.erase(iter);
				this->m_mapCoroutine[pCoroutineImpl->getCoroutineID()] = pCoroutineImpl;
				this->m_nTotalStackSize += nStackSize;
				return pCoroutineImpl;
			}
		}

		CCoroutineImpl* pCoroutineImpl = new CCoroutineImpl();
		if (!pCoroutineImpl->init(this->m_nNextCoroutineID++, nStackSize, callback))
		{
			SAFE_DELETE(pCoroutineImpl);
			return nullptr;
		}

		this->m_mapCoroutine[pCoroutineImpl->getCoroutineID()] = pCoroutineImpl;
		this->m_nTotalStackSize += nStackSize;

		return pCoroutineImpl;
	}

	CCoroutineImpl* CCoroutineMgr::getCoroutine(uint64_t nID)
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		auto iter = this->m_mapCoroutine.find(nID);
		if (iter == this->m_mapCoroutine.end())
			return nullptr;

		return iter->second;
	}

	uint32_t CCoroutineMgr::getCoroutineCount()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		return (uint32_t)this->m_mapCoroutine.size();
	}

	uint64_t CCoroutineMgr::getTotalStackSize()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		return (uint64_t)this->m_nTotalStackSize;
	}

	CCoroutineMgr::CCoroutineMgr()
		: m_nNextCoroutineID(1)
		, m_nTotalStackSize(0)
	{

	}

	CCoroutineMgr::~CCoroutineMgr()
	{
		for (auto iter = this->m_listRecycleCoroutine.begin(); iter != this->m_listRecycleCoroutine.end(); ++iter)
		{
			CCoroutineImpl* pCoroutineImpl = *iter;
			SAFE_DELETE(pCoroutineImpl);
		}
	}

	bool CCoroutineMgr::init()
	{
		return true;
	}

	bool CCoroutineMgr::addRecycleCoroutine(CCoroutineImpl* pCoroutineImpl)
	{
		DebugAstEx(pCoroutineImpl != nullptr, false);

		std::unique_lock<std::mutex> guard(this->m_lock);

		this->m_mapCoroutine.erase(pCoroutineImpl->getCoroutineID());
		this->m_nTotalStackSize -= pCoroutineImpl->getStackSize();

		if (this->m_listRecycleCoroutine.size() > _MAX_CO_RECYCLE_COUNT)
			return false;

		this->m_listRecycleCoroutine.push_back(pCoroutineImpl);

		return true;
	}

#ifndef _WIN32
	char* CCoroutineMgr::allocStack(uint32_t& nStackSize, uint32_t& nValgrindID)
	{
		uint32_t nPageSize = base::process_util::getPageSize();
		nStackSize = (nStackSize + nPageSize - 1) / nPageSize * nPageSize;

		char* pStack = reinterpret_cast<char*>(mmap(nullptr, nStackSize + 2 * nPageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0));
		if (pStack == (void*)MAP_FAILED)
			return nullptr;

#ifdef __USE_VALGRIND__
		nValgrindID = VALGRIND_STACK_REGISTER(pStack + nPageSize, pStack + nPageSize + nStackSize);
#endif

		mprotect(pStack, nPageSize, PROT_NONE);
		mprotect(pStack + nPageSize + nStackSize, nPageSize, PROT_NONE);

		return pStack + nPageSize;
	}

	void CCoroutineMgr::freeStack(char* pStack, uint32_t nStackSize, uint32_t nValgrindID)
	{
		uint32_t nPageSize = base::process_util::getPageSize();

		munmap(pStack - nPageSize, nStackSize + 2 * nPageSize);

#ifdef __USE_VALGRIND__
		VALGRIND_STACK_DEREGISTER(nValgrindID);
#endif
	}
#endif
}