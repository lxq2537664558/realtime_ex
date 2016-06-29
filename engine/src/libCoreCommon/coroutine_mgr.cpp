#include "stdafx.h"
#include "coroutine_mgr.h"
#include "coroutine_impl.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"

namespace core
{
	CCoroutineMgr::CCoroutineMgr()
		: m_pCurrentCoroutine(nullptr)
		, m_pRootCoroutine(nullptr)
		, m_nNextCoroutineID(1)
	{

	}

	CCoroutineMgr::~CCoroutineMgr()
	{
		SAFE_DELETE(this->m_pRootCoroutine);
		SAFE_DELETE_ARRAY(this->m_pMainStack);
	}

	bool CCoroutineMgr::init(uint32_t nMainStackSize)
	{
		CCoroutineImpl* pCoroutine = new CCoroutineImpl();
		if (!pCoroutine->init(this->m_nNextCoroutineID++, nullptr))
		{
			SAFE_DELETE(pCoroutine);
			return false;
		}

		this->m_pRootCoroutine = pCoroutine;
		this->m_pCurrentCoroutine = pCoroutine;
		this->m_pMainStack = new char[nMainStackSize];
		this->m_nMainStackSize = nMainStackSize;
		return true;
	}

	char* CCoroutineMgr::getMainStack() const
	{
		return this->m_pMainStack;
	}

	uint32_t CCoroutineMgr::getMainStackSize() const
	{
		return this->m_nMainStackSize;
	}

	CCoroutineImpl* CCoroutineMgr::getCurrentCoroutine() const
	{
		return this->m_pCurrentCoroutine;
	}

	void CCoroutineMgr::setCurrentCoroutine(CCoroutineImpl* pCoroutineImpl)
	{
		DebugAst(pCoroutineImpl != nullptr);

		this->m_pCurrentCoroutine = pCoroutineImpl;
	}

	CCoroutineImpl* CCoroutineMgr::startCoroutine(std::function<void(uint64_t)> fn)
	{
		CCoroutineImpl* pCoroutineImpl = nullptr;
		if (!this->m_listRecycleCoroutineImpl.empty())
		{
			pCoroutineImpl = this->m_listRecycleCoroutineImpl.front();
			this->m_listRecycleCoroutineImpl.pop_front();
		}
		else
		{
			pCoroutineImpl = new CCoroutineImpl();
		}
		
		if (!pCoroutineImpl->init(this->m_nNextCoroutineID++, fn))
		{
			SAFE_DELETE(pCoroutineImpl);
			return pCoroutineImpl;
		}

		this->m_mapCoroutineImpl[pCoroutineImpl->getCoroutineID()] = pCoroutineImpl;

		return pCoroutineImpl;
	}

	CCoroutineImpl* CCoroutineMgr::getCoroutine(uint64_t nID) const
	{
		auto iter = this->m_mapCoroutineImpl.find(nID);
		if (iter == this->m_mapCoroutineImpl.end())
			return nullptr;

		return iter->second;
	}

	void CCoroutineMgr::recycleCoroutine(CCoroutineImpl* pCoroutineImpl)
	{
		DebugAst(pCoroutineImpl != nullptr && pCoroutineImpl->getState() == eCS_DEAD);

		this->m_mapCoroutineImpl.erase(pCoroutineImpl->getCoroutineID());

		this->m_listRecycleCoroutineImpl.push_back(pCoroutineImpl);
	}

	void CCoroutineMgr::sleep(uint64_t ms)
	{
		DebugAst(this->m_mapWaitCoroutineInfo.find(this->m_pCurrentCoroutine->getCoroutineID()) == this->m_mapWaitCoroutineInfo.end());

		SWaitCoroutineInfo sWaitCoroutineInfo;
		sWaitCoroutineInfo.pCoroutineImpl = this->m_pCurrentCoroutine;
		sWaitCoroutineInfo.pTicker = new CTicker();
		sWaitCoroutineInfo.pTicker->setCallback(std::bind(&CCoroutineMgr::onWaitTicker, this, std::placeholders::_1));
		CCoreApp::Inst()->registerTicker(sWaitCoroutineInfo.pTicker, ms, 0, this->m_pCurrentCoroutine->getCoroutineID());

		this->m_mapWaitCoroutineInfo[this->m_pCurrentCoroutine->getCoroutineID()] = sWaitCoroutineInfo;

		this->m_pCurrentCoroutine->yield(eCYT_Sleep);
	}

	void CCoroutineMgr::onWaitTicker(uint64_t nContext)
	{
		auto iter = this->m_mapWaitCoroutineInfo.find(nContext);
		if (iter == this->m_mapWaitCoroutineInfo.end())
		{
			PrintWarning("unknown wait coroutine id: "UINT64FMT, nContext);
			return;
		}

		SAFE_DELETE(iter->second.pTicker);
		CCoroutineImpl* pCoroutineImpl = iter->second.pCoroutineImpl;
		this->m_mapWaitCoroutineInfo.erase(iter);

		DebugAst(pCoroutineImpl != nullptr);

		pCoroutineImpl->setState(eCS_SUSPEND);
		pCoroutineImpl->resume(0);
	}
}