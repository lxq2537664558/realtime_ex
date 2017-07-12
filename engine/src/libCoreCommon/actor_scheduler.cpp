#include "stdafx.h"
#include "actor_scheduler.h"
#include "message_dispatcher.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_common_define.h"
#include "actor_base_impl.h"

#include "libCoreCommon/base_app.h"
#include "libBaseCommon/base_time.h"


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
		int64_t nCurTime = base::getGmtTime();
		for (auto iter = this->m_mapPendingActorBase.begin(); iter != this->m_mapPendingActorBase.end();)
		{
			CActorBaseImpl* pActorBaseImpl = iter->second;
			if (pActorBaseImpl == nullptr)
			{
				++iter;
				continue;
			}

			if (!pActorBaseImpl->onPendingTimer(nCurTime))
			{
				++iter;
				continue;
			}

			this->m_mapPendingActorBase.erase(iter++);
		}

		std::map<uint64_t, CActorBaseImpl*> mapWorkActorBase;
		mapWorkActorBase.swap(this->m_mapWorkActorBase);
		for (auto iter = mapWorkActorBase.begin(); iter != mapWorkActorBase.end(); ++iter)
		{
			iter->second->process();
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
		DebugAst(pBaseActorImpl != nullptr);

		if (pBaseActorImpl->getState() == CActorBaseImpl::eABS_Normal || pBaseActorImpl->getState() == CActorBaseImpl::eABS_RecvPending)
			this->m_mapWorkActorBase[pBaseActorImpl->getID()] = pBaseActorImpl;
	}

	void CActorScheduler::addPendingActorBase(CActorBaseImpl* pActorBase)
	{
		DebugAst(pActorBase != nullptr);

		DebugAst(pActorBase->getState() == CActorBaseImpl::eABS_Normal);

		this->m_mapPendingActorBase[pActorBase->getID()] = pActorBase;

		pActorBase->setState(CActorBaseImpl::eABS_Pending);
	}
}