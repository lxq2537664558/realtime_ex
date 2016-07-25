#include "stdafx.h"
#include "response_promise.h"
#include "core_service_kit_impl.h"

namespace core
{
	CResponsePromise::CResponsePromise()
		: m_nSessionID(0)
	{

	}

	CResponsePromise::~CResponsePromise()
	{

	}

	void CResponsePromise::then(InvokeCallback callback)
	{
		this->then(callback, nullptr);
	}

	void CResponsePromise::then(InvokeCallback callback, InvokeErrCallback err)
	{
		DebugAst(callback != nullptr);

		if (this->m_nSessionID == 0)
		{
			PrintWarning("CResponsePromise::then error zero session id");
			return;
		}
		SResponseWaitInfo* pResponseWaitInfo = CCoreServiceKitImpl::Inst()->getTransporter()->getResponseWaitInfo(this->m_nSessionID, false);
		if (nullptr == pResponseWaitInfo)
		{
			PrintWarning("CResponsePromise::then error invalid session id: "UINT64FMT, this->m_nSessionID);
			return;
		}

		auto callback_ex = [callback](SResponseWaitInfo* pResponseWaitInfo, uint8_t nMessageType, message_header_ptr pMessage)->void
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

	CResponsePromise CResponsePromise::then_r(InvokeCallbackEx callback)
	{
		return this->then_r(callback, nullptr);
	}

	CResponsePromise CResponsePromise::then_r(InvokeCallbackEx callback, InvokeErrCallback err)
	{
		DebugAstEx(callback != nullptr, CResponsePromise());

		if (this->m_nSessionID == 0)
		{
			PrintWarning("CResponsePromise::then_r error zero session id");
			return CResponsePromise();
		}
		SResponseWaitInfo* pResponseWaitInfo = CCoreServiceKitImpl::Inst()->getTransporter()->getResponseWaitInfo(this->m_nSessionID, false);
		if (nullptr == pResponseWaitInfo)
		{
			PrintWarning("CResponsePromise::then_r error invalid session id: "UINT64FMT, this->m_nSessionID);
			return CResponsePromise();
		}

		auto callback_ex = [callback](SResponseWaitInfo* pResponseWaitInfo, uint8_t nMessageType, message_header_ptr pMessage)->void
		{
			CResponsePromise sResponsePromise = callback(nMessageType, pMessage);
			SResponseWaitInfo* pNextResponseWaitInfo = CCoreServiceKitImpl::Inst()->getTransporter()->getResponseWaitInfo(sResponsePromise.m_nSessionID, false);
			if (nullptr == pNextResponseWaitInfo)
			{
				PrintWarning("CResponsePromise::then_r error invalid session id: "UINT64FMT, sResponsePromise.m_nSessionID);
				return;
			}

			if (pNextResponseWaitInfo->callback != nullptr)
			{
				PrintWarning("CResponsePromise::then_r error callback is init session id: "UINT64FMT, sResponsePromise.m_nSessionID);
				return;
			}

			pNextResponseWaitInfo->listPromise = pResponseWaitInfo->listPromise;

			pResponseWaitInfo = pNextResponseWaitInfo;

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

		CResponsePromise sResponsePromise;
		sResponsePromise.m_nSessionID = this->m_nSessionID;

		this->m_nSessionID = 0;
		return sResponsePromise;
	}
}