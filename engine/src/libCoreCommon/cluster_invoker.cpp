#include "stdafx.h"
#include "cluster_invoker.h"
#include "core_app.h"
#include "load_balance_policy.h"

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
		
		return CCoreApp::Inst()->getTransport()->call(szServiceName, sRequestMessageInfo);
	}

	bool CClusterInvoker::invok_r(const std::string& szServiceName, const google::protobuf::Message* pMessage, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
		sRequestMessageInfo.callback = callback;
		
		return CCoreApp::Inst()->getTransport()->call(szServiceName, sRequestMessageInfo);
	}

	bool CClusterInvoker::invok(const google::protobuf::Message* pMessage, const std::string& szServiceGroup, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceContext)
	{
		DebugAstEx(pMessage != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pMessage->GetTypeName(), szServiceGroup, nLoadBalanceContext);
		if (szServiceName.empty())
			return false;

		return this->invok(szServiceName, pMessage);
	}

	bool CClusterInvoker::invok_r(const google::protobuf::Message* pMessage, const std::string& szServiceGroup, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceContext, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pMessage->GetTypeName(), szServiceGroup, nLoadBalanceContext);
		if (szServiceName.empty())
			return false;

		return this->invok_r(szServiceName, pMessage, callback, nContext);
	}

	void CClusterInvoker::response(const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = CCoreApp::Inst()->getTransport()->getServiceSessionInfo().nSessionID;
		sResponseMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreApp::Inst()->getTransport()->response(CCoreApp::Inst()->getTransport()->getServiceSessionInfo().szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	void CClusterInvoker::response(const SServiceSessionInfo& sServiceSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sServiceSessionInfo.nSessionID;
		sResponseMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreApp::Inst()->getTransport()->response(sServiceSessionInfo.szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	SServiceSessionInfo CClusterInvoker::getServiceSessionInfo() const
	{
		return CCoreApp::Inst()->getTransport()->getServiceSessionInfo();
	}

	bool CClusterInvoker::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = sClientSessionInfo.nSessionID;
		sGateMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);

		return CCoreApp::Inst()->getTransport()->send(sClientSessionInfo.szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::forward(uint64_t nSessionID, const message_header* pHeader, const std::string& szServiceGroup, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceContext)
	{
		DebugAstEx(pHeader != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string& szMessageName = CCoreApp::Inst()->getMessageDirectory()->getOtherMessageName(pHeader->nMessageID);
		const std::string szServiceName = pLoadBalancePolicy->select(szMessageName, szServiceGroup, nLoadBalanceContext);
		if (szServiceName.empty())
			return false;

		SGateForwardMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pHeader = const_cast<message_header*>(pHeader);

		return CCoreApp::Inst()->getTransport()->forward(szServiceName, sGateMessageInfo);
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
			if (!CCoreApp::Inst()->getTransport()->broadcast(iter->first, sGateBroadcastMessageInfo))
				bRet = false;
		}

		return bRet;
	}
}