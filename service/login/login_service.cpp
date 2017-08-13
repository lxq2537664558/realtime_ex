#include "stdafx.h"
#include "login_service.h"
#include "login_connection_from_client.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#include "tinyxml2/tinyxml2.h"
#include "proto_src/player_login_request.pb.h"

using namespace core;

CLoginService::CLoginService()
	: m_pLoginClientConnectionFactory(nullptr)
	, m_pLoginClientMessageDispatcher(nullptr)
	, m_pLoginClientMessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
	, m_pJsonProtobufFactory(nullptr)
{
}

CLoginService::~CLoginService()
{
	SAFE_DELETE(this->m_pLoginClientConnectionFactory);
	SAFE_DELETE(this->m_pLoginClientMessageDispatcher);
	SAFE_DELETE(this->m_pLoginClientMessageHandler);
	SAFE_DELETE(this->m_pNormalProtobufFactory);
	SAFE_DELETE(this->m_pJsonProtobufFactory);
}

bool CLoginService::onInit()
{
	this->m_pLoginClientMessageDispatcher = new CLoginClientMessageDispatcher(this);
	this->m_pLoginClientMessageHandler = new CLoginClientMessageHandler(this);

	this->m_pLoginClientConnectionFactory = new CLoginClientConnectionFactory();
	CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory("CLoginConnectionFromClient", this->m_pLoginClientConnectionFactory);
	
	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	this->m_pJsonProtobufFactory = new CJsonProtobufFactory();
	
	tinyxml2::XMLDocument* pConfigXML = new tinyxml2::XMLDocument();
	if (pConfigXML->LoadFile(this->getConfigFileName().c_str()) != tinyxml2::XML_SUCCESS)
	{
		PrintWarning("load etc config error");
		return false;
	}
	tinyxml2::XMLElement* pRootXML = pConfigXML->RootElement();
	if (pRootXML == nullptr)
	{
		PrintWarning("pRootXML == nullptr");
		return false;
	}

	tinyxml2::XMLElement* pListenClientAddrXML = pRootXML->FirstChildElement("client");
	DebugAstEx(pListenClientAddrXML != nullptr, false);

	const std::string szHost = pListenClientAddrXML->Attribute("host") != nullptr ? pListenClientAddrXML->Attribute("host") : "0.0.0.0";
	uint16_t nPort = (uint16_t)pListenClientAddrXML->UnsignedAttribute("port");
	uint32_t nSendBufSize = pListenClientAddrXML->IntAttribute("send_buf_size");
	uint32_t nRecvBufSize = pListenClientAddrXML->IntAttribute("recv_buf_size");

	char szBuf[256] = { 0 };
	base::crt::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	// 启动客户端连接
	CBaseApp::Inst()->getBaseConnectionMgr()->listen(szHost, nPort, true, "CLoginConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser, eCCT_Websocket);

	SAFE_DELETE(pConfigXML);

	PrintInfo("CGateService::onInit");

	player_login_request request_msg;
	request_msg.set_account_id(1);
	request_msg.set_server_id(1);
	char szBuf1[256] = { 0 };
	int32_t nRet = this->m_pJsonProtobufFactory->serialize_protobuf_message_to_buf(&request_msg, szBuf1, _countof(szBuf1));
	auto pMsg = this->m_pJsonProtobufFactory->unserialize_protobuf_message_from_buf("player_login_request", szBuf1, nRet);

	return true;
}

void CLoginService::onFrame()
{

}

void CLoginService::onQuit()
{

}

CLoginClientMessageDispatcher* CLoginService::getLoginClientMessageDispatcher() const
{
	return this->m_pLoginClientMessageDispatcher;
}

void CLoginService::release()
{
	delete this;
}

CProtobufFactory* CLoginService::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

core::CProtobufFactory* CLoginService::getForwardProtobufFactory() const
{
	return this->m_pJsonProtobufFactory;
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CLoginService();
}