#include "stdafx.h"
#include "login_service.h"
#include "login_connection_from_client.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#include "tinyxml2/tinyxml2.h"

using namespace core;

CLoginService::CLoginService()
	: m_pClientConnectionFactory(nullptr)
	, m_pClientMessageDispatcher(nullptr)
	, m_pClientMessageHandler(nullptr)
	, m_pDefaultProtobufFactory(nullptr)
{
}

CLoginService::~CLoginService()
{
	SAFE_DELETE(this->m_pClientConnectionFactory);
	SAFE_DELETE(this->m_pClientMessageDispatcher);
	SAFE_DELETE(this->m_pClientMessageHandler);
	SAFE_DELETE(this->m_pDefaultProtobufFactory);
}

bool CLoginService::onInit()
{
	this->m_pClientMessageDispatcher = new CClientMessageDispatcher(this);
	this->m_pClientMessageHandler = new CClientMessageHandler(this);

	this->m_pClientConnectionFactory = new CClientConnectionFactory();
	CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory("CLoginConnectionFromClient", this->m_pClientConnectionFactory);
	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	
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
	CBaseApp::Inst()->getBaseConnectionMgr()->listen(szHost, nPort, "CLoginConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser);

	SAFE_DELETE(pConfigXML);

	PrintInfo("CGateService::onInit");

	return true;
}

void CLoginService::onFrame()
{

}

void CLoginService::onQuit()
{

}

CClientMessageDispatcher* CLoginService::getClientMessageDispatcher() const
{
	return this->m_pClientMessageDispatcher;
}

void CLoginService::release()
{
	delete this;
}

CProtobufFactory* CLoginService::getProtobufFactory() const
{
	return this->m_pDefaultProtobufFactory;
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CLoginService();
}