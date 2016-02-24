#include "stdafx.h"
#include "message_port.h"
#include "service_mgr.h"

#include "libBaseCommon/debug_helper.h"
#include "libCoreCommon/core_app.h"


namespace core
{
	CMessagePort::CMessagePort()
	{
	}

	CMessagePort::~CMessagePort()
	{

	}

	bool CMessagePort::init()
	{
		return true;
	}

	bool CMessagePort::call(const std::string& szServiceName, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		if (!pMessage->SerializeToString(&sRequestMessageInfo.szMessageData))
			return false;

		sRequestMessageInfo.szMessageName = pMessage->GetTypeName();
		sRequestMessageInfo.callback = nullptr;

		return CServiceMgr::Inst()->call(szServiceName, sRequestMessageInfo);
	}

	bool CMessagePort::call_r(const std::string& szServiceName, const google::protobuf::Message* pMessage, std::function<void(const google::protobuf::Message*, EResponseResultType)>& callback)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		if (!pMessage->SerializeToString(&sRequestMessageInfo.szMessageData))
			return false;

		sRequestMessageInfo.szMessageName = pMessage->GetTypeName();
		sRequestMessageInfo.callback = callback;

		return CServiceMgr::Inst()->call(szServiceName, sRequestMessageInfo);
	}

	bool CMessagePort::broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		if (!pMessage->SerializeToString(&sRequestMessageInfo.szMessageData))
			return false;

		sRequestMessageInfo.szMessageName = pMessage->GetTypeName();
		sRequestMessageInfo.callback = nullptr;

		std::vector<std::string> vecServiceName;
		CServiceMgr::Inst()->getServiceName(szServiceType, vecServiceName);
		for (size_t i = 0; i < vecServiceName.size(); ++i)
		{
			if (!CServiceMgr::Inst()->call(vecServiceName[i], sRequestMessageInfo))
				return false;
		}

		return true;
	}

	void CMessagePort::response(const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		if (!pMessage->SerializeToString(&sResponseMessageInfo.szMessageData))
			return;

		sResponseMessageInfo.szMessageName = pMessage->GetTypeName();
		sResponseMessageInfo.nResult = eRRT_OK;

		bool bRet = CServiceMgr::Inst()->response(sResponseMessageInfo);
		DebugAst(bRet);
	}

	void CMessagePort::registMessageHandler(const std::string& szMessageName, std::function<void(const google::protobuf::Message*)>& callback)
	{
		DebugAst(callback != nullptr);

		CServiceMgr::Inst()->registMessageHandler(szMessageName, callback);
	}
}