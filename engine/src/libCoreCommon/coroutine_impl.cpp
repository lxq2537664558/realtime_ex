#include "stdafx.h"
#include "coroutine_impl.h"
#include "coroutine_mgr.h"
#include "base_app_impl.h"

#include "libBaseCommon/debug_helper.h"

#include <algorithm>

namespace core
{

	void CCoroutineImpl::onCallback(void* pParm)
	{
		CCoroutineImpl* pCoroutineImpl = reinterpret_cast<CCoroutineImpl*>(pParm);
		pCoroutineImpl->m_fn(pCoroutineImpl->m_nContext);
		pCoroutineImpl->yield(eCYT_Dead);
	};

	void CCoroutineImpl::saveStack()
	{
#ifndef _WIN32
		char* pStackTop = CBaseAppImpl::Inst()->getCoroutineMgr()->getMainStack() + CBaseAppImpl::Inst()->getCoroutineMgr()->getMainStackSize();
		char nDummy = 0;
		DebugAst(pStackTop - &nDummy <= CBaseAppImpl::Inst()->getCoroutineMgr()->getMainStackSize());

		if (this->m_nStackCap < pStackTop - &nDummy)
		{
			delete [] this->m_pStack;
			this->m_nStackCap = pStackTop - &nDummy;
			this->m_pStack= new char[this->m_nStackCap];
		}
		this->m_nStackSize = pStackTop - &nDummy;
		memcpy(this->m_pStack, &nDummy, this->m_nStackSize);
#endif
	}

	CCoroutineImpl::CCoroutineImpl()
		: m_nID(0)
		, m_pParentCoroutine(nullptr)
		, m_nContext(0)
		, m_eState(eCS_DEAD)
#ifdef _WIN32
		, m_hHandle(nullptr)
#else
		, m_nStackCap(0)
		, m_nStackSize(0)
		, m_pStack(nullptr)
#endif
	{

	}

	CCoroutineImpl::~CCoroutineImpl()
	{
#ifdef _WIN32
		if (this->m_hHandle != nullptr)
		{
			if (this->getCoroutineID() != 1)
				DeleteFiber(this->m_hHandle);
			else
				ConvertFiberToThread();

			this->m_hHandle = nullptr;
		}
#else
		SAFE_DELETE_ARRAY(this->m_pStack);
#endif
	}

	bool CCoroutineImpl::init(uint64_t nID, std::function<void(uint64_t)> fn)
	{
		DebugAstEx(this->m_eState == eCS_DEAD, false);

#ifdef _WIN32

		if (this->m_hHandle != nullptr)
		{
			DeleteFiber(this->m_hHandle);
			this->m_hHandle = nullptr;
		}
#define _WIN_CO_STACK_SIZE 64*1024
		if (nID != 1)
		{
			this->m_hHandle = CreateFiberEx(_WIN_CO_STACK_SIZE,
				_WIN_CO_STACK_SIZE, FIBER_FLAG_FLOAT_SWITCH,
				(LPFIBER_START_ROUTINE)CCoroutineImpl::onCallback, this);
			if (this->m_hHandle == nullptr)
				return false;
		}
		else
		{
			this->m_hHandle = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
			if (this->m_hHandle == nullptr)
				return false;
		}
#else
		if (nID == 1)
		{
			this->m_eState = eCS_READY;
			this->m_nID = nID;
			return true;
		}

		DebugAstEx(fn != nullptr, false);

		if (-1 == getcontext(&this->m_ctx))
			return false;

		this->m_ctx.uc_stack.ss_sp = CBaseAppImpl::Inst()->getCoroutineMgr()->getMainStack();
		this->m_ctx.uc_stack.ss_size = CBaseAppImpl::Inst()->getCoroutineMgr()->getMainStackSize();
		this->m_ctx.uc_link = nullptr;

		makecontext(&this->m_ctx, (void(*)(void))&CCoroutineImpl::onCallback, 1, this);

		this->m_nStackSize = 0;
#endif

		this->m_fn = fn;
		this->m_eState = eCS_READY;
		this->m_nID = nID;
		this->m_nContext = 0;

		return true;
	}

	uint64_t CCoroutineImpl::yield(ECoroutineYieldType eType)
	{
		DebugAstEx(this->m_eState == eCS_RUNNING, 0);

		if (this->m_pParentCoroutine == nullptr)
		{
			PrintWarning("current corotine is root corotine don't yield");
			return 0;
		}

		CBaseAppImpl::Inst()->getCoroutineMgr()->setCurrentCoroutine(this->m_pParentCoroutine);

		this->m_pParentCoroutine->m_eState = eCS_RUNNING;
		if (eType == eCYT_Dead)
			this->setState(eCS_DEAD);
		else if (eType == eCYT_Normal)
			this->setState(eCS_SUSPEND);

		this->m_pParentCoroutine = nullptr;

		if (this->m_eState == eCS_DEAD)
			CBaseAppImpl::Inst()->getCoroutineMgr()->recycleCoroutine(this);
		
		CCoroutineImpl* pCurrentCoroutine = CBaseAppImpl::Inst()->getCoroutineMgr()->getCurrentCoroutine();

#ifdef _WIN32
		SwitchToFiber(pCurrentCoroutine->m_hHandle);
#else
		this->saveStack();
		swapcontext(&this->m_ctx, &pCurrentCoroutine->m_ctx);
#endif

		return this->m_nContext;
	}
	
	void CCoroutineImpl::resume(uint64_t nContext)
	{
		DebugAst(this->getState() == eCS_READY || this->getState() == eCS_SUSPEND);

		CCoroutineImpl* pCurrentCoroutine = CBaseAppImpl::Inst()->getCoroutineMgr()->getCurrentCoroutine();
		DebugAst(pCurrentCoroutine != nullptr);

		bool bRestoreStack = (this->getState() == eCS_SUSPEND);
		this->setState(eCS_RUNNING);
		pCurrentCoroutine->setState(eCS_SUSPEND);
		CBaseAppImpl::Inst()->getCoroutineMgr()->setCurrentCoroutine(this);
		this->m_pParentCoroutine = pCurrentCoroutine;
		this->m_nContext = nContext;

#ifdef _WIN32
		SwitchToFiber(this->m_hHandle);
#else
		if (bRestoreStack)
			memcpy(CBaseAppImpl::Inst()->getCoroutineMgr()->getMainStack() + CBaseAppImpl::Inst()->getCoroutineMgr()->getMainStackSize() - this->m_nStackSize, this->m_pStack, this->m_nStackSize);
		
		swapcontext(&this->m_pParentCoroutine->m_ctx, &this->m_ctx);
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

	void CCoroutineImpl::sendMessage(void* pData)
	{
		this->m_listMessage.push_back(pData);
	}

	void* CCoroutineImpl::recvMessage()
	{
		if (this->m_listMessage.empty())
			return nullptr;

		void* pData = this->m_listMessage.front();
		this->m_listMessage.pop_front();

		return pData;
	}
}