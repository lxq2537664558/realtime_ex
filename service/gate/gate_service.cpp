#include "stdafx.h"
#include "gate_service.h"
#include "gate_connection_from_client.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"

#include "tinyxml2/tinyxml2.h"

#include "proto_src/gate_addr_notify.pb.h"
#include "proto_src/gate_online_count_notify.pb.h"

using namespace core;

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
	this->m_pGateServiceMessageHandler = new CGateServiceMessageHandler(this);

	this->m_pClientSessionMgr = new CClientSessionMgr(this);

	this->m_pClientConnectionFactory = new CClientConnectionFactory();
	CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory("CGateConnectionFromClient", this->m_pClientConnectionFactory);

	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	
	this->setServiceConnectCallback(std::bind(&CGateService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyGateOnlineCount.setCallback(std::bind(&CGateService::onNotifyGateOnlineCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyGateOnlineCount, 5000, 5000, 0);

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

	char szAddr[256] = { 0 };
	base::crt::snprintf(szAddr, _countof(szAddr), "%s:%d", szHost.c_str(), nPort);
	this->m_szAddr = szAddr;

	char szBuf[256] = { 0 };
	base::crt::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	// 启动客户端连接
	CBaseApp::Inst()->getBaseConnectionMgr()->listen(szHost, nPort, "CGateConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser);

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

void CGateService::release()
{
	delete this;
}

CProtobufFactory* CGateService::getProtobufFactory() const
{
	return this->m_pDefaultProtobufFactory;
}

CClientMessageHandler* CGateService::getClientMessageHandler() const
{
	return this->m_pClientMessageHandler;
}

void CGateService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		gate_addr_notify msg1;
		msg1.set_addr(this->m_szAddr);
		this->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg1);

		gate_online_count_notify msg2;
		msg2.set_count(this->m_pClientSessionMgr->getSessionCount());
		this->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg2);
	}
}

void CGateService::onNotifyGateOnlineCount(uint64_t nContext)
{
	gate_online_count_notify msg;
	msg.set_count(this->m_pClientSessionMgr->getSessionCount());
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CGateService();
}