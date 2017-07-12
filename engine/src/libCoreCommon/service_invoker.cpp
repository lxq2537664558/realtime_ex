#include "stdafx.h"
#include "service_invoker.h"
#include "core_common_define.h"
#include "actor_base.h"
#include "coroutine.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"

namespace core
{
	CServiceInvoker::CServiceInvoker(uint16_t nServiceID)
		: m_nServiceID(nServiceID)
	{

	}

	CServiceInvoker::~CServiceInvoker()
	{

	}

	bool CServiceInvoker::init()
	{
		return true;
	}

	bool CServiceInvoker::send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->invoke(eType, 0, this->m_nServiceID, nID, pMessage);
	}

	bool CServiceInvoker::forward(EMessageTargetType eType, uint64_t nID, uint64_t nSessionID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->forward(eType, nSessionID, this->m_nServiceID, nID, pMessage);
	}

	void CServiceInvoker::response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		bool bRet = CCoreApp::Inst()->getTransporter()->response(sSessionInfo.eTargetType, sSessionInfo.nSessionID, eRRT_OK, sSessionInfo.nFromID, pMessage);
		DebugAst(bRet);
	}

	bool CServiceInvoker::send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		return CCoreApp::Inst()->getTransporter()->send(sClientSessionInfo.nSessionID, sClientSessionInfo.nGateServiceID, pMessage);
	}

	bool CServiceInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::map<uint16_t, std::vector<uint64_t>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].nGateServiceID].push_back(vecClientSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			if (!CCoreApp::Inst()->getTransporter()->broadcast(iter->second, iter->first, pMessage))
				bRet = false;
		}

		return bRet;
	}

	bool CServiceInvoker::invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		uint64_t nSessionID = CCoreApp::Inst()->getTransporter()->genSessionID();
		if (!CCoreApp::Inst()->getTransporter()->invoke(eType, nSessionID, this->m_nServiceID, nID, pMessage))
			return false;

		SPendingResponseInfo* pPendingResponseInfo = CCoreApp::Inst()->getTransporter()->addPendingResponseInfo(nSessionID, nID, pMessage->GetTypeName(), callback);
		DebugAstEx(pPendingResponseInfo != nullptr, false);
		
		return true;
	}
}