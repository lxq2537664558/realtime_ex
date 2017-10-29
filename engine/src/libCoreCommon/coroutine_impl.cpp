#include "coroutine_impl.h"
#include "coroutine_mgr.h"
#include "coroutine_thread.h"

#include "libBaseCommon/debug_helper.h"

#define _MIN_CO_STACK 64 * 1024

#ifndef _WIN32
extern "C" int32_t	save_context(void* reg);
extern "C" void		restore_context(void* reg, int32_t);
#endif

using namespace core::coroutine;

namespace core
{
#ifdef _WIN32
	void CCoroutineImpl::onCallback(void* pParm)
#else
	void CCoroutineImpl::onCallback()
#endif
	{
		while (true)
		{
			CCoroutineThread* pCoroutineThread = getCoroutineThread();
			CCoroutineImpl* pCoroutineImpl = pCoroutineThread->getCurrentCoroutine();
			pCoroutineImpl->m_callback(pCoroutineImpl->m_nContext);
			// 一定要把callback设置成null，不然绑在这个function上的对象就不会释放
			pCoroutineImpl->m_callback = nullptr;
			// 这里需要重新获取，因为有可能线程切换了
			pCoroutineThread = getCoroutineThread();
			pCoroutineThread->setCurrentCoroutine(nullptr);
			
			// 这里不能马上放入回收列表，因为这个时候还没有切换到该线程的主协程
			pCoroutineImpl->setState(eCS_DEAD);

#ifdef _WIN32
			SwitchToFiber(pCoroutineThread->getMainContext());
#else
			if (save_context(pCoroutineImpl->m_pContext) == 0)
				restore_context(pCoroutineThread->getMainContext(), 1);
#endif
		}
	}

	CCoroutineImpl::CCoroutineImpl()
		: m_nID(0)
		, m_nContext(0)
		, m_eState(eCS_NONE)
		, m_pContext(nullptr)
		, m_nStackSize(0)
#ifndef _WIN32
		, m_nStackCap(0)
		, m_pStack(nullptr)
		, m_nValgrindID(0)
#endif
	{
#ifndef _WIN32
		this->m_pContext = new context();
#endif
	}

	CCoroutineImpl::~CCoroutineImpl()
	{
		uint32_t nValgrindID = 0;
#ifndef _WIN32
		context* pContext = reinterpret_cast<context*>(this->m_pContext);
		SAFE_DELETE(pContext);
		nValgrindID = this->m_nValgrindID;
		
		// 释放时需要考虑栈是倒长的
		CCoroutineMgr::freeStack(this->m_pStack - this->m_nStackSize, (uint32_t)this->m_nStackSize, nValgrindID);
#endif
	}

	bool CCoroutineImpl::init(uint64_t nID, uint32_t nStackSize, const std::function<void(uint64_t)>& callback)
	{
		DebugAstEx(this->m_eState == eCS_NONE, false);
		
		DebugAstEx(callback != nullptr, false);
		DebugAstEx(nStackSize >= _MIN_CO_STACK, false);

#ifdef _WIN32
		DebugAstEx(nStackSize != 0, false);
		this->m_nStackSize = nStackSize;
		this->m_pContext = ::CreateFiberEx(nStackSize, nStackSize, FIBER_FLAG_FLOAT_SWITCH, (LPFIBER_START_ROUTINE)CCoroutineImpl::onCallback, this);
		if (this->m_pContext == nullptr)
			return false;
#else
		if (save_context(this->m_pContext) != 0)
		{
			CCoroutineImpl::onCallback();

			// 不可能执行到这里的
			DebugAstEx(!"", false);
		}

		uint32_t nValgrindID = 0;
		char* pStack = CCoroutineMgr::allocStack(nStackSize, nValgrindID);
		if (nullptr == pStack)
			return false;
		this->m_pStack = pStack + nStackSize;
		this->m_nStackSize = nStackSize;

		reinterpret_cast<context*>(this->m_pContext)->rsp = (uintptr_t)this->m_pStack;
		this->m_nValgrindID = nValgrindID;
#endif

		this->m_callback = callback;
		this->m_nContext = 0;
		this->m_eState = eCS_READY;
		this->m_nID = nID;
		
		return true;
	}

	uint64_t CCoroutineImpl::yield()
	{
		DebugAstEx(this->m_eState == eCS_RUNNING, false);
		
		CCoroutineThread* pCoroutineThread = getCoroutineThread();
		pCoroutineThread->setCurrentCoroutine(nullptr);

		this->m_eState = eCS_SUSPEND;

#ifdef _WIN32
		SwitchToFiber(pCoroutineThread->getMainContext());
#else
		if (save_context(this->m_pContext) == 0)
			restore_context(pCoroutineThread->getMainContext(), 1);
#endif	
		return this->m_nContext;
	}
	
	void CCoroutineImpl::resume(uint64_t nContext)
	{
		DebugAst(this->getState() == eCS_READY || this->getState() == eCS_SUSPEND);

		this->m_eState = eCS_RUNNING;

		CCoroutineThread* pCoroutineThread = getCoroutineThread();
		pCoroutineThread->setCurrentCoroutine(this);
		this->m_nContext = nContext;

#ifdef _WIN32
		SwitchToFiber(this->m_pContext);
#else
		if (save_context(pCoroutineThread->getMainContext()) == 0)
			restore_context(this->m_pContext, 1);
#endif
	}

	uint32_t CCoroutineImpl::getState() const
	{
		return this->m_eState;
	}

	void CCoroutineImpl::setState(uint32_t nState)
	{
		this->m_eState = (ECoroutineState)nState;
	}

	uint64_t CCoroutineImpl::getCoroutineID() const
	{
		return this->m_nID;
	}

	void CCoroutineImpl::setLocalData(const char* szName, uint64_t nData)
	{
		if (nullptr == szName)
			return;

		this->m_mapLocalData[szName] = nData;
	}

	bool CCoroutineImpl::getLocalData(const char* szName, uint64_t& nData) const
	{
		if (szName == nullptr)
			return false;

		auto iter = this->m_mapLocalData.find(szName);
		if (iter == this->m_mapLocalData.end())
			return false;

		nData = iter->second;

		return true;
	}

	void CCoroutineImpl::delLocalData(const char* szName)
	{
		if (szName == nullptr)
			return;

		this->m_mapLocalData.erase(szName);
	}

	void CCoroutineImpl::setCallback(const std::function<void(uint64_t)>& callback)
	{
		this->m_callback = callback;
	}

	uint32_t CCoroutineImpl::getStackSize() const
	{
		return (uint32_t)this->m_nStackSize;
	}

}