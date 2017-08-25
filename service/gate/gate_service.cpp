#include "gate_service.h"
#include "gate_connection_from_client.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"

#include "tinyxml2/tinyxml2.h"

#include "msg_proto_src/g2d_addr_notify.pb.h"
#include "msg_proto_src/g2d_online_count_notify.pb.h"

//#define _WEB_SOCKET_

using namespace core;

CGateService::CGateService()
	: m_pGateClientConnectionFactory(nullptr)
	, m_pGateClientSessionMgr(nullptr)
	, m_pGateClientMessageDispatcher(nullptr)
	, m_pGateClientMessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
	, m_pJsonProtobufFactory(nullptr)
{
}

CGateService::~CGateService()
{
	SAFE_DELETE(this->m_pGateClientConnectionFactory);
	SAFE_DELETE(this->m_pGateClientSessionMgr);
	SAFE_DELETE(this->m_pGateClientMessageDispatcher);
	SAFE_DELETE(this->m_pGateClientMessageHandler);
	SAFE_DELETE(this->m_pNormalProtobufFactory);
	SAFE_DELETE(this->m_pJsonProtobufFactory);
}

bool CGateService::onInit()
{
	this->m_pGateClientMessageDispatcher = new CGateClientMessageDispatcher(this);
	this->m_pGateClientMessageHandler = new CGateClientMessageHandler(this);
	this->m_pGateServiceMessageHandler = new CGateServiceMessageHandler(this);

	this->m_pGateClientSessionMgr = new CGateClientSessionMgr(this);

	this->m_pGateClientConnectionFactory = new CGateClientConnectionFactory();
	CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory("CGateConnectionFromClient", this->m_pGateClientConnectionFactory);

	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	this->m_pJsonProtobufFactory = new CJsonProtobufFactory();
	
	this->setServiceConnectCallback(std::bind(&CGateService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyOnlineCount.setCallback(std::bind(&CGateService::onNotifyOnlineCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyOnlineCount, 5000, 5000, 0);

	tinyxml2::XMLDocument* pConfigXML = new tinyxml2::XMLDocument();
	if (pConfigXML->LoadFile(this->getConfigFileName().c_str()) != tinyxml2::XML_SUCCESS)
	{
		PrintWarning("load {} config error", this->getConfigFileName());
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

	const std::string szReportHost = pListenClientAddrXML->Attribute("report_host");

	char szAddr[256] = { 0 };
	base::function_util::snprintf(szAddr, _countof(szAddr), "%s:%d", szReportHost.c_str(), nPort);
	this->m_szAddr = szAddr;

	char szBuf[256] = { 0 };
	base::function_util::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	// 启动客户端连接
#ifdef _WEB_SOCKET_
	CBaseApp::Inst()->getBaseConnectionMgr()->listen(szHost, nPort, true, "CGateConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser, eCCT_Websocket);
#else
	CBaseApp::Inst()->getBaseConnectionMgr()->listen(szHost, nPort, true, "CGateConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser, eCCT_Normal);
#endif

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

CGateClientSessionMgr* CGateService::getGateClientSessionMgr() const
{
	return this->m_pGateClientSessionMgr;
}

CGateClientMessageDispatcher* CGateService::getGateClientMessageDispatcher() const
{
	return this->m_pGateClientMessageDispatcher;
}

void CGateService::release()
{
	delete this;
}

CProtobufFactory* CGateService::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

CGateClientMessageHandler* CGateService::getGateClientMessageHandler() const
{
	return this->m_pGateClientMessageHandler;
}

void CGateService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		g2d_addr_notify msg1;
		msg1.set_addr(this->m_szAddr);
		this->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg1);

		g2d_online_count_notify msg2;
		msg2.set_count(this->m_pGateClientSessionMgr->getSessionCount());
		this->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg2);
	}
}

void CGateService::onNotifyOnlineCount(uint64_t nContext)
{
	g2d_online_count_notify msg;
	msg.set_count(this->m_pGateClientSessionMgr->getSessionCount());
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

core::CProtobufFactory* CGateService::getForwardProtobufFactory() const
{
#ifdef _WEB_SOCKET_
	return this->m_pJsonProtobufFactory;
#else
	return this->m_pNormalProtobufFactory;
#endif
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CGateService();
}