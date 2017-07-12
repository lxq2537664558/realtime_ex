#include "stdafx.h"
#include "actor_scheduler.h"
#include "message_dispatcher.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_common_define.h"
#include "actor_base_impl.h"

#include "libCoreCommon/base_app.h"


namespace core
{
	CActorScheduler::CActorScheduler()
		: m_nNextActorID(1)
	{
		this->m_vecBuf.resize(UINT16_MAX);
	}

	CActorScheduler::~CActorScheduler()
	{

	}

	bool CActorScheduler::init()
	{
		return true;
	}

	CActorBaseImpl* CActorScheduler::getActorBase(uint64_t nID) const
	{
		auto iter = this->m_mapActorBase.find(nID);
		if (iter == this->m_mapActorBase.end())
			return nullptr;

		return iter->second;
	}

	void CActorScheduler::run()
	{
		std::list<CActorBaseImpl*> listWorkActorBase;
		listWorkActorBase.swap(this->m_listWorkActorBase);
		for (auto iter = listWorkActorBase.begin(); iter != listWorkActorBase.end(); ++iter)
		{
			(*iter)->process();
		}
	}

	CActorBaseImpl* CActorScheduler::createActorBase(CActorBase* pActorBase)
	{
		DebugAstEx(pActorBase != nullptr, nullptr);

		CActorBaseImpl* pActorBaseImpl = new CActorBaseImpl(this->m_nNextActorID++, pActorBase);

		this->m_mapActorBase[pActorBaseImpl->getID()] = pActorBaseImpl;

		return pActorBaseImpl;
	}

	void CActorScheduler::destroyActorBase(CActorBaseImpl* pActorBaseImpl)
	{
		DebugAst(pActorBaseImpl != nullptr);

		this->m_mapActorBase.erase(pActorBaseImpl->getID());

		SAFE_DELETE(pActorBaseImpl);
	}

	void CActorScheduler::addWorkActorBase(CActorBaseImpl* pBaseActorImpl)
	{
		DebugAst(pBaseActorImpl != nullptr && pBaseActorImpl->getState() != CActorBaseImpl::eABS_Pending);

		if (pBaseActorImpl->getState() == CActorBaseImpl::eABS_Working)
			return;

		this->m_listWorkActorBase.push_back(pBaseActorImpl);
	}

}