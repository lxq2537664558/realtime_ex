#include "stdafx.h"
#include "cluster_invoker.h"
#include "core_service_kit.h"
#include "core_service_kit_impl.h"
#include "core_service_kit_define.h"
#include "service_base.h"

#include "libBaseCommon/debug_helper.h"


namespace core
{
	CClusterInvoker::CClusterInvoker()
	{
	}

	CClusterInvoker::~CClusterInvoker()
	{

	}

	bool CClusterInvoker::init()
	{
		return true;
	}

	bool CClusterInvoker::invok(const std::string& szServiceName, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.callback = nullptr;
		
		return CCoreServiceKitImpl::Inst()->getTransporter()->call(szServiceName, sRequestMessageInfo);
	}

	bool CClusterInvoker::invok_r(const std::string& szServiceName, const message_header* pData, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = const_cast<message_header*>(pData);
		sRequestMessageInfo.callback = callback;
		
		return CCoreServiceKitImpl::Inst()->getTransporter()->call(szServiceName, sRequestMessageInfo);
	}

	void CClusterInvoker::response(const message_header* pData)
	{
		DebugAst(pData != nullptr);

		CCoreServiceKitImpl::Inst()->getInvokerTrace()->send(pData->nMessageID);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = CCoreServiceKitImpl::Inst()->getTransporter()->getServiceSessionInfo().nSessionID;
		sResponseMessageInfo.pData = const_cast<message_header*>(pData);
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreServiceKitImpl::Inst()->getTransporter()->response(CCoreServiceKitImpl::Inst()->getTransporter()->getServiceSessionInfo().szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	void CClusterInvoker::response(const SServiceSessionInfo& sServiceSessionInfo, const message_header* pData)
	{
		DebugAst(pData != nullptr);

		CCoreServiceKitImpl::Inst()->getInvokerTrace()->send(pData->nMessageID);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sServiceSessionInfo.nSessionID;
		sResponseMessageInfo.pData = const_cast<message_header*>(pData);
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreServiceKitImpl::Inst()->getTransporter()->response(sServiceSessionInfo.szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	SServiceSessionInfo CClusterInvoker::getServiceSessionInfo() const
	{
		return CCoreServiceKitImpl::Inst()->getTransporter()->getServiceSessionInfo();
	}

	bool CClusterInvoker::send(const SClientSessionInfo& sClientSessionInfo, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);

		CCoreServiceKitImpl::Inst()->getInvokerTrace()->send(pData->nMessageID);

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = sClientSessionInfo.nSessionID;
		sGateMessageInfo.pData = const_cast<message_header*>(pData);

		return CCoreServiceKitImpl::Inst()->getTransporter()->send(sClientSessionInfo.szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::forward(const std::string& szServiceName, uint64_t nSessionID, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);
		
		CCoreServiceKitImpl::Inst()->getInvokerTrace()->send(pData->nMessageID);

		SGateForwardMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pData = const_cast<message_header*>(pData);

		return CCoreServiceKitImpl::Inst()->getTransporter()->forward(szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);

		std::map<std::string, std::vector<uint64_t>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].szServiceName].push_back(vecClientSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			SGateBroadcastMessageInfo sGateBroadcastMessageInfo;
			sGateBroadcastMessageInfo.vecSessionID = iter->second;
			sGateBroadcastMessageInfo.pData = const_cast<message_header*>(pData);
			if (!CCoreServiceKitImpl::Inst()->getTransporter()->broadcast(iter->first, sGateBroadcastMessageInfo))
				bRet = false;
		}

		return bRet;
	}
}