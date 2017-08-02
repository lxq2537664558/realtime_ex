#include "stdafx.h"
#include "gate_service.h"
#include "connection_from_client.h"

#include "libCoreCommon\base_app.h"
#include "libCoreCommon\base_connection_mgr.h"

#include "tinyxml2\tinyxml2.h"

using namespace core;

DEFINE_OBJECT(CGateService, 1);

CGateService::CGateService()
	: m_pClientConnectionFactory(nullptr)
	, m_pClientSessionMgr(nullptr)
	, m_pClientMessageDispatcher(nullptr)
	, m_pClientMessageHandler(nullptr)
	, m_pDefaultProtobufFactory(nullptr)
{
}

CGateService::~CGateService()
{
	SAFE_DELETE(this->m_pClientConnectionFactory);
	SAFE_DELETE(this->m_pClientSessionMgr);
	SAFE_DELETE(this->m_pClientMessageDispatcher);
	SAFE_DELETE(this->m_pClientMessageHandler);
	SAFE_DELETE(this->m_pDefaultProtobufFactory);
}

bool CGateService::onInit()
{
	this->m_pClientMessageDispatcher = new CClientMessageDispatcher(this);
	this->m_pClientMessageHandler = new CClientMessageHandler(this);

	this->m_pClientSessionMgr = new CClientSessionMgr(this);

	this->m_pClientConnectionFactory = new CClientConnectionFactory();
	CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromClient, this->m_pClientConnectionFactory);
	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	this->setProtobufFactory(this->m_pDefaultProtobufFactory);

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
	CBaseApp::Inst()->getBaseConnectionMgr()->listen(szHost, nPort, eBCT_ConnectionFromClient, szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser);

	SAFE_DELETE(pConfigXML);

	PrintInfo("CGateService::onInit");

	return true;
}

void CGateService::onFrame()
{

}

void CGateService::onQuit()
{

}

CClientSessionMgr* CGateService::getClientSessionMgr() const
{
	return this->m_pClientSessionMgr;
}

CClientMessageDispatcher* CGateService::getClientMessageDispatcher() const
{
	return this->m_pClientMessageDispatcher;
}

#ifdef _WIN32

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CGateService::registerClassInfo();
		break;

	case DLL_THREAD_ATTACH:

		break;

	case DLL_THREAD_DETACH:

		break;

	case DLL_PROCESS_DETACH:

		break;
	}
	return TRUE;
}
#endif