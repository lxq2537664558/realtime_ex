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

	bool CClusterInvoker::invok(const std::string& szServiceName, uint16_t nMessageFormat, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.nMessageFormat = nMessageFormat;
		sRequestMessageInfo.pData = pData;
		sRequestMessageInfo.callback = nullptr;
		
		return CCoreApp::Inst()->getTransport()->call(szServiceName, sRequestMessageInfo);
	}

	bool CClusterInvoker::invok_r(const std::string& szServiceName, uint16_t nMessageFormat, const message_header* pData, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.nMessageFormat = nMessageFormat;
		sRequestMessageInfo.pData = pData;
		sRequestMessageInfo.callback = callback;
		
		return CCoreApp::Inst()->getTransport()->call(szServiceName, sRequestMessageInfo);
	}

	bool CClusterInvoker::invok(uint16_t nMessageFormat, const message_header* pData, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam)
	{
		DebugAstEx(pData != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pData->nMessageID, false, nLoadBalanceParam);
		if (szServiceName.empty())
			return false;

		return this->invok(szServiceName, nMessageFormat, pData);
	}

	bool CClusterInvoker::invok_r(uint16_t nMessageFormat, const message_header* pData, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam, InvokeCallback callback, uint64_t nContext /* = 0 */)
	{
		DebugAstEx(pData != nullptr && callback != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pData->nMessageID, false, nLoadBalanceParam);
		if (szServiceName.empty())
			return false;

		return this->invok_r(szServiceName, nMessageFormat, pData, callback, nContext);
	}

	void CClusterInvoker::response(uint16_t nMessageFormat, const message_header* pData)
	{
		DebugAst(pData != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nMessageFormat = nMessageFormat;
		sResponseMessageInfo.nSessionID = CCoreApp::Inst()->getTransport()->getServiceSessionInfo().nSessionID;
		sResponseMessageInfo.pData = pData;
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreApp::Inst()->getTransport()->response(CCoreApp::Inst()->getTransport()->getServiceSessionInfo().szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	void CClusterInvoker::response(const SServiceSessionInfo& sServiceSessionInfo, uint16_t nMessageFormat, const message_header* pData)
	{
		DebugAst(pData != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nMessageFormat = nMessageFormat;
		sResponseMessageInfo.nSessionID = sServiceSessionInfo.nSessionID;
		sResponseMessageInfo.pData = pData;
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CCoreApp::Inst()->getTransport()->response(sServiceSessionInfo.szServiceName, sResponseMessageInfo);
		DebugAst(bRet);
	}

	SServiceSessionInfo CClusterInvoker::getServiceSessionInfo() const
	{
		return CCoreApp::Inst()->getTransport()->getServiceSessionInfo();
	}

	bool CClusterInvoker::send(const SClientSessionInfo& sGateSessionInfo, uint16_t nMessageFormat, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = sGateSessionInfo.nSessionID;
		sGateMessageInfo.nMessageFormat = nMessageFormat | eMT_TO_GATE;
		sGateMessageInfo.pData = pData;

		return CCoreApp::Inst()->getTransport()->send(sGateSessionInfo.szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::foward(uint64_t nSessionID, uint16_t nMessageFormat, const message_header* pData, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam)
	{
		DebugAstEx(pData != nullptr && pLoadBalancePolicy != nullptr, false);

		const std::string szServiceName = pLoadBalancePolicy->select(pData->nMessageID, false, nLoadBalanceParam);
		if (szServiceName.empty())
			return false;

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.nMessageFormat = nMessageFormat | eMT_FROM_GATE;
		sGateMessageInfo.pData = pData;

		return CCoreApp::Inst()->getTransport()->send(szServiceName, sGateMessageInfo);
	}

	bool CClusterInvoker::broadcast(const std::vector<SClientSessionInfo>& vecGateSessionInfo, uint16_t nMessageFormat, const message_header* pData)
	{
		DebugAstEx(pData != nullptr, false);

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
			sGateBroadcastMessageInfo.nMessageFormat = nMessageFormat;
			sGateBroadcastMessageInfo.pData = pData;
			if (!CCoreApp::Inst()->getTransport()->broadcast(iter->first, sGateBroadcastMessageInfo))
				bRet = false;
		}

		return bRet;
	}
}