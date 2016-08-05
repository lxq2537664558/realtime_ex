#include "stdafx.h"
#include "response_future.h"
#include "core_service_app_impl.h"

namespace core
{
	CResponseFuture::CResponseFuture()
		: m_nSessionID(0)
		, m_nActorID(0)
	{

	}

	CResponseFuture::~CResponseFuture()
	{

	}

	void CResponseFuture::then(InvokeCallback callback)
	{
		this->then(callback, nullptr);
	}

	void CResponseFuture::then(InvokeCallback callback, InvokeErrCallback err)
	{
		DebugAst(callback != nullptr);

		if (this->m_nSessionID == 0)
		{
			PrintWarning("CResponseFuture::then error zero session id");
			return;
		}
		SResponseWaitInfo* pResponseWaitInfo = nullptr;
		if (this->m_nActorID != 0)
		{
			CBaseActorImpl* pActorBase = CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(this->m_nActorID);
			if (nullptr == pActorBase)
			{
				PrintWarning("CResponseFuture::then error invalid actor id: "UINT64FMT" session id: "UINT64FMT, this->m_nActorID, this->m_nSessionID);
				return;
			}
			pResponseWaitInfo = pActorBase->getResponseWaitInfo(this->m_nSessionID, false);
		}
		else
		{
			pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(this->m_nSessionID, false);
		}
		if (nullptr == pResponseWaitInfo)
		{
			PrintWarning("CResponseFuture::then error invalid actor id: "UINT64FMT" session id: "UINT64FMT, this->m_nActorID, this->m_nSessionID);
			return;
		}

		auto callback_ex = [callback](SResponseWaitInfo* pResponseWaitInfo, uint8_t nMessageType, CMessage pMessage)->void
		{
			callback(nMessageType, pMessage);
		};

		if (pResponseWaitInfo->callback == nullptr)
		{
			pResponseWaitInfo->callback = callback_ex;
			pResponseWaitInfo->err = err;
		}
		else
		{
			pResponseWaitInfo->listPromise.push_back(std::make_pair(callback_ex, err));
		}

		this->m_nSessionID = 0;
	}

	CResponseFuture CResponseFuture::then_r(InvokeCallbackEx callback)
	{
		return this->then_r(callback, nullptr);
	}

	CResponseFuture CResponseFuture::then_r(InvokeCallbackEx callback, InvokeErrCallback err)
	{
		DebugAstEx(callback != nullptr, CResponseFuture());

		if (this->m_nSessionID == 0)
		{
			PrintWarning("CResponseFuture::then_r error zero session id");
			return CResponseFuture();
		}
		SResponseWaitInfo* pResponseWaitInfo = nullptr;
		if (this->m_nActorID != 0)
		{
			CBaseActorImpl* pActorBase = CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(this->m_nActorID);
			if (nullptr == pActorBase)
			{
				PrintWarning("CResponseFuture::then_r error invalid actor id: "UINT64FMT" session id: "UINT64FMT, this->m_nActorID, this->m_nSessionID);
				return CResponseFuture();;
			}
			pResponseWaitInfo = pActorBase->getResponseWaitInfo(this->m_nSessionID, false);
		}
		else
		{
			pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(this->m_nSessionID, false);
		}
		if (nullptr == pResponseWaitInfo)
		{
			PrintWarning("CResponseFuture::then then_r invalid actor id: "UINT64FMT" session id: "UINT64FMT, this->m_nActorID, this->m_nSessionID);
			return CResponseFuture();;
		}

		auto callback_ex = [callback](SResponseWaitInfo* pResponseWaitInfo, uint8_t nMessageType, CMessage pMessage)->void
		{
			CResponseFuture sResponsePromise = callback(nMessageType, pMessage);

			SResponseWaitInfo* pNextResponseWaitInfo = nullptr;
			if (sResponsePromise.m_nActorID != 0)
			{
				CBaseActorImpl* pActorBase = CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(sResponsePromise.m_nActorID);
				if (nullptr == pActorBase)
				{
					PrintWarning("CResponseFuture::then_r error invalid actor id: "UINT64FMT" session id: "UINT64FMT, sResponsePromise.m_nActorID, sResponsePromise.m_nSessionID);
					return;
				}
				pNextResponseWaitInfo = pActorBase->getResponseWaitInfo(sResponsePromise.m_nSessionID, false);
			}
			else
			{
				pNextResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(sResponsePromise.m_nSessionID, false);
			}
			if (nullptr == pNextResponseWaitInfo)
			{
				PrintWarning("CResponseFuture::then then_r invalid actor id: "UINT64FMT" session id: "UINT64FMT, sResponsePromise.m_nActorID, sResponsePromise.m_nSessionID);
				return;;
			}

			pNextResponseWaitInfo->listPromise = pResponseWaitInfo->listPromise;

			if (!pNextResponseWaitInfo->listPromise.empty())
			{
				pNextResponseWaitInfo->callback = pNextResponseWaitInfo->listPromise.front().first;
				pNextResponseWaitInfo->err = pNextResponseWaitInfo->listPromise.front().second;
				pNextResponseWaitInfo->listPromise.pop_front();
			}
		};

		if (pResponseWaitInfo->callback == nullptr)
		{
			pResponseWaitInfo->callback = callback_ex;
			pResponseWaitInfo->err = err;
		}
		else
		{
			pResponseWaitInfo->listPromise.push_back(std::make_pair(callback_ex, err));
		}

		CResponseFuture sResponsePromise;
		sResponsePromise.m_nSessionID = this->m_nSessionID;

		this->m_nSessionID = 0;
		return sResponsePromise;
	}

	void CResponseFuture::setSessionID(uint64_t nSessionID)
	{
		this->m_nSessionID = nSessionID;
	}

	void CResponseFuture::setActorID(uint64_t nActorID)
	{
		this->m_nActorID = nActorID;
	}
}