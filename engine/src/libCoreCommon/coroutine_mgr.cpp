#include "stdafx.h"
#include "coroutine_mgr.h"
#include "coroutine_impl.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"

#define _MAX_CO_RECYCLE_COUNT	100

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

	CCoroutineImpl* CCoroutineMgr::createCoroutine(std::function<void(uint64_t)> callback)
	{
		DebugAstEx(callback != nullptr, nullptr);

		CCoroutineImpl* pCoroutineImpl = nullptr;
		if (!this->m_listRecycleCoroutineImpl.empty())
		{
			pCoroutineImpl = this->m_listRecycleCoroutineImpl.front();
			this->m_listRecycleCoroutineImpl.pop_front();

			pCoroutineImpl->setCallback(callback);
			pCoroutineImpl->setState(eCS_SUSPEND);
		}
		else
		{
			pCoroutineImpl = new CCoroutineImpl();
			if (!pCoroutineImpl->init(this->m_nNextCoroutineID++, callback))
			{
				SAFE_DELETE(pCoroutineImpl);
				return nullptr;
			}
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

	void CCoroutineMgr::addRecycleCoroutine(CCoroutineImpl* pCoroutineImpl)
	{
		DebugAst(pCoroutineImpl != nullptr);

		this->m_mapCoroutineImpl.erase(pCoroutineImpl->getCoroutineID());

		this->m_listRecycleCoroutineImpl.push_back(pCoroutineImpl);
	}

	void CCoroutineMgr::update()
	{
		while (this->m_listRecycleCoroutineImpl.size() > _MAX_CO_RECYCLE_COUNT)
		{
			CCoroutineImpl* pCoroutineImpl = *this->m_listRecycleCoroutineImpl.begin();
			SAFE_DELETE(pCoroutineImpl);

			this->m_listRecycleCoroutineImpl.pop_front();
		}
	}

}