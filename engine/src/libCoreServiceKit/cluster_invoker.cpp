#include "stdafx.h"
#include "cluster_invoker.h"
#include "core_service_app.h"
#include "core_service_app_impl.h"
#include "core_service_kit_define.h"
#include "service_base.h"

#include "libBaseCommon/debug_helper.h"
#include "libCoreCommon/coroutine.h"
#include "libCoreCommon/promise.h"

namespace core
{
	namespace cluster_invoker
	{
		bool invoke(uint16_t nServiceID, const message_header* pData)
		{
			DebugAstEx(pData != nullptr, false);

			SRequestMessageInfo sRequestMessageInfo;
			sRequestMessageInfo.pData = const_cast<message_header*>(pData);
			sRequestMessageInfo.nSessionID = 0;
			sRequestMessageInfo.nCoroutineID = 0;
			sRequestMessageInfo.nFromActorID = 0;
			sRequestMessageInfo.nToActorID = 0;

			return CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo);
		}

		bool invoke_r(uint16_t nServiceID, const message_header* pData, CResponseFuture& sResponseFuture)
		{
			DebugAstEx(pData != nullptr, false);

			SRequestMessageInfo sRequestMessageInfo;
			sRequestMessageInfo.pData = const_cast<message_header*>(pData);
			sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
			sRequestMessageInfo.nCoroutineID = 0;
			sRequestMessageInfo.nFromActorID = 0;
			sRequestMessageInfo.nToActorID = 0;

			if (!CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo))
				return false;

			CCoreServiceAppImpl::Inst()->getTransporter()->addResponseWaitInfo(sRequestMessageInfo.nSessionID, 0);

			SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(sRequestMessageInfo.nSessionID, false);
			DebugAstEx(nullptr != pResponseWaitInfo, false);

			std::shared_ptr<CPromise<CMessage>> pPromise = std::make_shared<CPromise<CMessage>>();

			pResponseWaitInfo->callback = [pPromise](SResponseWaitInfo* pResponseWaitInfo, CMessage pMessage, uint32_t nErrorCode)->void
			{
				pPromise->setValue(pMessage, nErrorCode);
			};

			sResponseFuture = pPromise->getFuture();

			return true;
		}

		bool invoke_r(uint16_t nServiceID, const message_header* pData, InvokeCallback& callback)
		{
			DebugAstEx(pData != nullptr && callback != nullptr, false);

			SRequestMessageInfo sRequestMessageInfo;
			sRequestMessageInfo.pData = const_cast<message_header*>(pData);
			sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
			sRequestMessageInfo.nCoroutineID = 0;
			sRequestMessageInfo.nFromActorID = 0;
			sRequestMessageInfo.nToActorID = 0;

			if (!CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nServiceID, sRequestMessageInfo))
				return false;

			CCoreServiceAppImpl::Inst()->getTransporter()->addResponseWaitInfo(sRequestMessageInfo.nSessionID, 0);

			SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(sRequestMessageInfo.nSessionID, false);
			DebugAstEx(nullptr != pResponseWaitInfo, false);

			pResponseWaitInfo->callback = [callback](SResponseWaitInfo*, CMessage pMessage, uint32_t nErrorCode)->void
			{
				callback(pMessage, nErrorCode);
			};

			return true;
		}

		bool invoke(const std::string& szServiceName, const message_header* pData)
		{
			uint16_t nServiceID = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getServiceID(szServiceName);
			if (nServiceID == 0)
				return false;

			return invoke(nServiceID, pData);
		}

		bool invoke_r(const std::string& szServiceName, const message_header* pData, CResponseFuture& sResponseFuture)
		{
			uint16_t nServiceID = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getServiceID(szServiceName);
			if (nServiceID == 0)
				return false;

			return invoke_r(nServiceID, pData, sResponseFuture);
		}

		bool invoke_r(const std::string& szServiceName, const message_header* pData, InvokeCallback& callback)
		{
			uint16_t nServiceID = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getServiceID(szServiceName);
			if (nServiceID == 0)
				return false;

			return invoke_r(nServiceID, pData, callback);
		}

		void response(const message_header* pData)
		{
			DebugAst(pData != nullptr);

			response(CCoreServiceAppImpl::Inst()->getTransporter()->getServiceSessionInfo(), pData);
		}

		void response(const SServiceSessionInfo& sServiceSessionInfo, const message_header* pData)
		{
			DebugAst(pData != nullptr);

			CCoreServiceAppImpl::Inst()->getInvokerTrace()->send(pData->nMessageID);

			SResponseMessageInfo sResponseMessageInfo;
			sResponseMessageInfo.nSessionID = sServiceSessionInfo.nSessionID;
			sResponseMessageInfo.pData = const_cast<message_header*>(pData);
			sResponseMessageInfo.nResult = eRRT_OK;
			sResponseMessageInfo.nFromActorID = 0;
			sResponseMessageInfo.nToActorID = 0;

			bool bRet = CCoreServiceAppImpl::Inst()->getTransporter()->response(sServiceSessionInfo.nServiceID, sResponseMessageInfo);
			DebugAst(bRet);
		}

		SServiceSessionInfo getServiceSessionInfo()
		{
			return CCoreServiceAppImpl::Inst()->getTransporter()->getServiceSessionInfo();
		}

		bool send(const SClientSessionInfo& sClientSessionInfo, const message_header* pData)
		{
			DebugAstEx(pData != nullptr, false);

			CCoreServiceAppImpl::Inst()->getInvokerTrace()->send(pData->nMessageID);

			SGateMessageInfo sGateMessageInfo;
			sGateMessageInfo.nSessionID = sClientSessionInfo.nSessionID;
			sGateMessageInfo.pData = const_cast<message_header*>(pData);

			return CCoreServiceAppImpl::Inst()->getTransporter()->send(sClientSessionInfo.nServiceID, sGateMessageInfo);
		}

		bool forward(uint16_t nServiceID, uint64_t nSessionID, const message_header* pData)
		{
			return forward(nServiceID, 0, nSessionID, pData);
		}

		bool forward(uint16_t nServiceID, uint64_t nActorID, uint64_t nSessionID, const message_header* pData)
		{
			DebugAstEx(pData != nullptr, false);

			CCoreServiceAppImpl::Inst()->getInvokerTrace()->send(pData->nMessageID);

			SGateForwardMessageInfo sGateMessageInfo;
			sGateMessageInfo.nActorID = nActorID;
			sGateMessageInfo.nSessionID = nSessionID;
			sGateMessageInfo.pData = const_cast<message_header*>(pData);

			return CCoreServiceAppImpl::Inst()->getTransporter()->forward(nServiceID, sGateMessageInfo);
		}

		bool forward(const std::string& szServiceName, uint64_t nSessionID, const message_header* pData)
		{
			uint16_t nServiceID = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getServiceID(szServiceName);
			if (nServiceID == 0)
				return false;

			return forward(nServiceID, 0, nSessionID, pData);
		}

		bool forward(const std::string& szServiceName, uint64_t nActorID, uint64_t nSessionID, const message_header* pData)
		{
			uint16_t nServiceID = CCoreServiceAppImpl::Inst()->getCoreServiceProxy()->getServiceID(szServiceName);
			if (nServiceID == 0)
				return false;

			return forward(nServiceID, nActorID, nSessionID, pData);
		}

		bool broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const message_header* pData)
		{
			DebugAstEx(pData != nullptr, false);

			std::map<uint16_t, std::vector<uint64_t>> mapClientSessionInfo;
			for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
			{
				mapClientSessionInfo[vecClientSessionInfo[i].nServiceID].push_back(vecClientSessionInfo[i].nSessionID);
			}

			bool bRet = true;
			for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
			{
				SGateBroadcastMessageInfo sGateBroadcastMessageInfo;
				sGateBroadcastMessageInfo.vecSessionID = iter->second;
				sGateBroadcastMessageInfo.pData = const_cast<message_header*>(pData);
				if (!CCoreServiceAppImpl::Inst()->getTransporter()->broadcast(iter->first, sGateBroadcastMessageInfo))
					bRet = false;
			}

			return bRet;
		}
	}
}