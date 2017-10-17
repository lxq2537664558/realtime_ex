#include "gate_service.h"
#include "gate_connection_from_client.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"

#include "tinyxml2/tinyxml2.h"

#include "server_proto_src/g2d_addr_notify.pb.h"
#include "server_proto_src/g2d_online_count_notify.pb.h"

#define _WEB_SOCKET_

using namespace core;

CGateService::CGateService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
{
}

CGateService::~CGateService()
{
}

bool CGateService::onInit()
{
	this->m_pGateClientMessageDispatcher = std::make_unique<CGateClientMessageDispatcher>(this);
	this->m_pGateClientMessageHandler = std::make_unique<CGateClientMessageHandler>(this);
	this->m_pGateServiceMessageHandler = std::make_unique<CGateServiceMessageHandler>(this);

	this->m_pGateClientSessionMgr = std::make_unique<CGateClientSessionMgr>(this);

	this->m_pGateClientConnectionFactory = std::make_unique<CGateClientConnectionFactory>();
	this->getBaseConnectionMgr()->setBaseConnectionFactory("CGateConnectionFromClient", this->m_pGateClientConnectionFactory.get());

	this->m_pNormalProtobufSerializer = std::make_unique<CNormalProtobufSerializer>();
	this->m_pJsonProtobufSerializer = std::make_unique<CJsonProtobufSerializer>();
	
	this->setForwardMessageSerializer(this->m_pJsonProtobufSerializer.get());
	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer.get());

	this->setServiceMessageSerializer(0, eMST_Protobuf);

	this->setServiceConnectCallback(std::bind(&CGateService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));
	this->setServiceDisconnectCallback(std::bind(&CGateService::onServiceDisconnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyOnlineCount.setCallback(std::bind(&CGateService::onNotifyOnlineCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyOnlineCount, 5000, 5000, 0);

	tinyxml2::XMLDocument sConfigXML;
	if (sConfigXML.LoadFile(this->getConfigFileName().c_str()) != tinyxml2::XML_SUCCESS)
	{
		PrintWarning("load {} config error", this->getConfigFileName());
		return false;
	}
	tinyxml2::XMLElement* pRootXML = sConfigXML.RootElement();
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
	if (!this->getBaseConnectionMgr()->listen(szHost, nPort, true, "CGateConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser, eCCT_Websocket))
#else
	if (!this->getBaseConnectionMgr()->listen(szHost, nPort, true, "CGateConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser, eCCT_Normal))
#endif
	{
		PrintWarning("gate listen error");
		return false;
	}

	PrintInfo("CGateService::onInit");

	return true;
}

void CGateService::onFrame()
{

}

void CGateService::onQuit()
{
	PrintInfo("CGateService::onQuit");
	this->doQuit();
}

CGateClientSessionMgr* CGateService::getGateClientSessionMgr() const
{
	return this->m_pGateClientSessionMgr.get();
}

CGateClientMessageDispatcher* CGateService::getGateClientMessageDispatcher() const
{
	return this->m_pGateClientMessageDispatcher.get();
}

void CGateService::release()
{
	delete this;

	google::protobuf::ShutdownProtobufLibrary();
}

CGateClientMessageHandler* CGateService::getGateClientMessageHandler() const
{
	return this->m_pGateClientMessageHandler.get();
}

void CGateService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		g2d_addr_notify msg1;
		msg1.set_addr(this->m_szAddr);
		this->getServiceInvoker()->send(nServiceID, &msg1);

		g2d_online_count_notify msg2;
		msg2.set_count(this->m_pGateClientSessionMgr->getSessionCount());
		this->getServiceInvoker()->send(nServiceID, &msg2);
	}
}

void CGateService::onServiceDisconnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "gas")
	{
		this->m_pGateClientSessionMgr->onGasDisconnect(nServiceID);
	}
}

void CGateService::onNotifyOnlineCount(uint64_t nContext)
{
	g2d_online_count_notify msg;
	msg.set_count(this->m_pGateClientSessionMgr->getSessionCount());
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CGateService(sServiceBaseInfo, szConfigFileName);
}