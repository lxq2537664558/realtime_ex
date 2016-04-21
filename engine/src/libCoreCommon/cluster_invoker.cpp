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

	bool CClusterInvoker::invok(const google::protobuf::Message* pMessage, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam)
	{
		DebugAstEx(pMessage != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pMessage->GetTypeName(), false, nLoadBalanceParam);
		if (szServiceName.empty())
			return false;

		return this->invok(szServiceName, pMessage);
	}

	bool CClusterInvoker::invok_r(const google::protobuf::Message* pMessage, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pMessage->GetTypeName(), false, nLoadBalanceParam);
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

	bool CClusterInvoker::send(const SClientSessionInfo& sGateSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = sGateSessionInfo.nSessionID;
		sGateMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);

		return CCoreApp::Inst()->getTransport()->send(sGateSessionInfo.szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::foward(uint64_t nSessionID, const google::protobuf::Message* pMessage, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam)
	{
		DebugAstEx(pMessage != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pMessage->GetTypeName(), false, nLoadBalanceParam);
		if (szServiceName.empty())
			return false;

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pMessage = const_cast<google::protobuf::Message*>(pMessage);

		return CCoreApp::Inst()->getTransport()->send(szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::broadcast(const std::vector<SClientSessionInfo>& vecGateSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::map<std::string, std::vector<uint64_t>> mapGateSessionInfo;
		for (size_t i = 0; i < vecGateSessionInfo.size(); ++i)
		{
			mapGateSessionInfo[vecGateSessionInfo[i].szServiceName].push_back(vecGateSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapGateSessionInfo.begin(); iter != mapGateSessionInfo.end(); ++iter)
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