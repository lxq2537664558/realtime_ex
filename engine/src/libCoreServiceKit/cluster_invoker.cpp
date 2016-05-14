#include "stdafx.h"
#include "cluster_invoker.h"
#include "core_service_kit.h"
#include "core_service_kit_impl.h"
#include "load_balance.h"
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

	bool CClusterInvoker::invok(const std::string& szServiceName, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
		sRequestMessageInfo.callback = nullptr;
		
		return CCoreServiceKitImpl::Inst()->getTransporter()->call(szServiceName, sRequestMessageInfo);
	}

	bool CClusterInvoker::invok_r(const std::string& szServiceName, const google::protobuf::Message* pMessage, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
		sRequestMessageInfo.callback = callback;
		
		return CCoreServiceKitImpl::Inst()->getTransporter()->call(szServiceName, sRequestMessageInfo);
	}

	bool CClusterInvoker::invok(const google::protobuf::Message* pMessage, uint64_t nSessionID, const std::string& szServiceGroup)
	{
		DebugAstEx(pMessage != nullptr, false);

		const std::string& szMessageName = pMessage->GetTypeName();
		const SMetaMessageProxyInfo* pMetaMessageProxyInfo = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getMetaMessageProxyInfo(_GET_MESSAGE_ID(szMessageName));
		if (nullptr == pMetaMessageProxyInfo)
		{
			PrintWarning("CClusterInvoker::invok error invalid message name session_id: "UINT64FMT" service_group: %s message_name: %s", nSessionID, szServiceGroup.c_str(), szMessageName.c_str());
			return false;
		}
		ILoadBalance* pLoadBalance = CCoreServiceKitImpl::Inst()->getLoadBalanceMgr()->getLoadBalance(pMetaMessageProxyInfo->szLoadbalanceName);
		if (nullptr == pLoadBalance)
		{
			PrintWarning("CClusterInvoker::invok error invalid load_balance session_id: "UINT64FMT" service_group: %s message_name: %s load_balance_name: %s", nSessionID, szServiceGroup.c_str(), szMessageName.c_str(), pMetaMessageProxyInfo->szLoadbalanceName.c_str());
			return false;
		}

		const std::string szServiceName = pLoadBalance->select(szMessageName, nSessionID, szServiceGroup);
		if (szServiceName.empty())
			return false;

		return this->invok(szServiceName, pMessage);
	}

	bool CClusterInvoker::invok_r(const google::protobuf::Message* pMessage, uint64_t nSessionID, const std::string& szServiceGroup, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr, false);

		const std::string& szMessageName = pMessage->GetTypeName();
		const SMetaMessageProxyInfo* pMetaMessageProxyInfo = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getMetaMessageProxyInfo(szMessageName);
		if (nullptr == pMetaMessageProxyInfo)
		{
			PrintWarning("CClusterInvoker::invok_r error invalid message name session_id: "UINT64FMT" service_group: %s message_name: %s", nSessionID, szServiceGroup.c_str(), szMessageName.c_str());
			return false;
		}
		ILoadBalance* pLoadBalance = CCoreServiceKitImpl::Inst()->getLoadBalanceMgr()->getLoadBalance(pMetaMessageProxyInfo->szLoadbalanceName);
		if (nullptr == pLoadBalance)
		{
			PrintWarning("CClusterInvoker::invok error invalid load_balance session_id: "UINT64FMT" service_group: %s message_name: %s load_balance_name: %s", nSessionID, szServiceGroup.c_str(), szMessageName.c_str(), pMetaMessageProxyInfo->szLoadbalanceName.c_str());
			return false;
		}

		const std::string szServiceName = pLoadBalance->select(szMessageName, nSessionID, szServiceGroup);
		if (szServiceName.empty())
			return false;

		return this->invok_r(szServiceName, pMessage, callback, nContext);
	}

	void CClusterInvoker::response(const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = CCoreServiceKitImpl::Inst()->getTransporter()->getServiceSessionInfo().nSessionID;
		sResponseMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreServiceKitImpl::Inst()->getTransporter()->response(CCoreServiceKitImpl::Inst()->getTransporter()->getServiceSessionInfo().szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	void CClusterInvoker::response(const SServiceSessionInfo& sServiceSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sServiceSessionInfo.nSessionID;
		sResponseMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreServiceKitImpl::Inst()->getTransporter()->response(sServiceSessionInfo.szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	SServiceSessionInfo CClusterInvoker::getServiceSessionInfo() const
	{
		return CCoreServiceKitImpl::Inst()->getTransporter()->getServiceSessionInfo();
	}

	bool CClusterInvoker::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = sClientSessionInfo.nSessionID;
		sGateMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);

		return CCoreServiceKitImpl::Inst()->getTransporter()->send(sClientSessionInfo.szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::forward(const message_header* pHeader, uint64_t nSessionID, const std::string& szServiceGroup)
	{
		DebugAstEx(pHeader != nullptr, false);

		const SMetaMessageProxyInfo* pMetaMessageProxyInfo = CCoreServiceKitImpl::Inst()->getCoreServiceProxy()->getMetaMessageProxyInfo(pHeader->nMessageID);
		if (nullptr == pMetaMessageProxyInfo)
		{
			PrintWarning("CClusterInvoker::forward error invalid message name session_id: "UINT64FMT" service_group: %s message_id: %d", nSessionID, szServiceGroup.c_str(), pHeader->nMessageID);
			return false;
		}
		ILoadBalance* pLoadBalance = CCoreServiceKitImpl::Inst()->getLoadBalanceMgr()->getLoadBalance(pMetaMessageProxyInfo->szLoadbalanceName);
		if (nullptr == pLoadBalance)
		{
			PrintWarning("CClusterInvoker::invok error invalid load_balance session_id: "UINT64FMT" service_group: %s message_name: %s load_balance_name: %s", nSessionID, szServiceGroup.c_str(), pMetaMessageProxyInfo->szMessageName.c_str(), pMetaMessageProxyInfo->szLoadbalanceName.c_str());
			return false;
		}

		const std::string szServiceName = pLoadBalance->select(pMetaMessageProxyInfo->szMessageName, nSessionID, szServiceGroup);
		if (szServiceName.empty())
			return false;

		SGateForwardMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pHeader = const_cast<message_header*>(pHeader);

		return CCoreServiceKitImpl::Inst()->getTransporter()->forward(szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

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
			sGateBroadcastMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
			if (!CCoreServiceKitImpl::Inst()->getTransporter()->broadcast(iter->first, sGateBroadcastMessageInfo))
				bRet = false;
		}

		return bRet;
	}
}