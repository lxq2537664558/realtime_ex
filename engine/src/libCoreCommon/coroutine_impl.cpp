#include "coroutine_impl.h"
#include "coroutine_mgr.h"

#include <algorithm>

#include <string.h>
#include <assert.h>
#ifdef _WIN32
#include <Windows.h>
#endif

#include "libBaseCommon/debug_helper.h"

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
			CCoroutineMgr* pCoroutineMgr = getCoroutineMgr();
			CCoroutineImpl* pCoroutineImpl = pCoroutineMgr->getCurrentCoroutine();
			pCoroutineImpl->m_callback(pCoroutineImpl->m_nContext);
			pCoroutineMgr->addRecycleCoroutine(pCoroutineImpl);

			pCoroutineMgr->setCurrentCoroutine(nullptr);

			pCoroutineImpl->m_eState = eCS_DEAD;
#ifdef _WIN32
			SwitchToFiber(pCoroutineMgr->getMainContext());
#else
			if (!pCoroutineImpl->m_bOwnerStack)
				pCoroutineImpl->m_nStackSize = 0;
			if (save_context(pCoroutineImpl->m_pContext) == 0)
				restore_context(pCoroutineMgr->getMainContext(), 1);
#endif
		}
	}
	
#ifndef _WIN32
	void CCoroutineImpl::saveStack()
	{
		char* pStack = getCoroutineMgr()->getMainStack();
		char nDummy = 0;

		if (this->m_nStackCap < (uintptr_t)(pStack - &nDummy))
		{
			SAFE_DELETE_ARRAY(this->m_pStack);
			this->m_nStackCap = pStack - &nDummy;
			this->m_pStack= new char[this->m_nStackCap];
		}
		this->m_nStackSize = pStack - &nDummy;
		memcpy(this->m_pStack, &nDummy, this->m_nStackSize);
	}
#endif

	CCoroutineImpl::CCoroutineImpl()
		: m_nID(0)
		, m_nContext(0)
		, m_eState(eCS_NONE)
		, m_pContext(nullptr)
		, m_nStackSize(0)
#ifndef _WIN32
		, m_nStackCap(0)
		, m_pStack(nullptr)
		, m_bOwnerStack(false)
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
		if (this->m_bOwnerStack)
		{
			CCoroutineMgr::freeStack(this->m_pStack, (uint32_t)this->m_nStackSize, nValgrindID);
		}
		else
		{
			SAFE_DELETE_ARRAY(this->m_pStack);
		}
#endif
	}

	bool CCoroutineImpl::init(uint64_t nID, uint32_t nStackSize, const std::function<void(uint64_t)>& callback)
	{
		DebugAstEx(this->m_eState == eCS_NONE, false);
		
		DebugAstEx(callback != nullptr, false);
		
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
		if (nStackSize != 0)
			this->m_bOwnerStack = true;
		
		CCoroutineMgr* pCoroutineMgr = getCoroutineMgr();
		if (nStackSize != 0)
		{
			this->m_bOwnerStack = true;
			uint32_t nValgrindID = 0;
			char* pStack = CCoroutineMgr::allocStack(nStackSize, nValgrindID);
			if (nullptr == pStack)
				return false;
			this->m_pStack = pStack + nStackSize;
			this->m_nStackSize = nStackSize;

			reinterpret_cast<context*>(this->m_pContext)->rsp = (uintptr_t)this->m_pStack;
			this->m_nValgrindID = nValgrindID;
		}
		else
		{
			this->m_bOwnerStack = false;
			reinterpret_cast<context*>(this->m_pContext)->rsp = (uintptr_t)pCoroutineMgr->getMainStack();
		}
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
		
		CCoroutineMgr* pCoroutineMgr = getCoroutineMgr();
		pCoroutineMgr->setCurrentCoroutine(nullptr);

		this->m_eState = eCS_SUSPEND;

#ifdef _WIN32
		SwitchToFiber(pCoroutineMgr->getMainContext());
#else
		if (!this->m_bOwnerStack)
			this->saveStack();

		if (save_context(this->m_pContext) == 0)
			restore_context(pCoroutineMgr->getMainContext(), 1);
#endif	
		return this->m_nContext;
	}
	
	void CCoroutineImpl::resume(uint64_t nContext)
	{
		DebugAst(this->getState() == eCS_READY || this->getState() == eCS_SUSPEND);

		this->m_eState = eCS_RUNNING;

		CCoroutineMgr* pCoroutineMgr = getCoroutineMgr();
		pCoroutineMgr->setCurrentCoroutine(this);
		this->m_nContext = nContext;

#ifdef _WIN32
		SwitchToFiber(this->m_pContext);
#else
		if (!this->m_bOwnerStack)
			memcpy(pCoroutineMgr->getMainStack() - this->m_nStackSize, this->m_pStack, this->m_nStackSize);

		if (save_context(pCoroutineMgr->getMainContext()) == 0)
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
#ifndef _WIN32
		if (!this->m_bOwnerStack)
			return 0;
#endif

		return (uint32_t)this->m_nStackSize;
	}

}