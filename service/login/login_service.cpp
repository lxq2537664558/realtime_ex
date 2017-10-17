#include "login_service.h"
#include "login_connection_from_client.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#include "tinyxml2/tinyxml2.h"

#define _WEB_SOCKET_

using namespace core;

CLoginService::CLoginService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
{
}

CLoginService::~CLoginService()
{
}

bool CLoginService::onInit()
{
	this->m_pLoginClientMessageDispatcher = std::make_unique<CLoginClientMessageDispatcher>(this);
	this->m_pLoginClientMessageHandler = std::make_unique<CLoginClientMessageHandler>(this);

	this->m_pLoginClientConnectionFactory = std::make_unique<CLoginClientConnectionFactory>();
	this->getBaseConnectionMgr()->setBaseConnectionFactory("CLoginConnectionFromClient", this->m_pLoginClientConnectionFactory.get());
	
	this->m_pNormalProtobufSerializer = std::make_unique<CNormalProtobufSerializer>();
	this->m_pJsonProtobufSerializer = std::make_unique<CJsonProtobufSerializer>();
	
	this->setForwardMessageSerializer(this->m_pJsonProtobufSerializer.get());
	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer.get());

	this->setServiceMessageSerializer(0, eMST_Protobuf);

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

	char szBuf[256] = { 0 };
	base::function_util::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	// 启动客户端连接
#ifdef _WEB_SOCKET_
	if (!this->getBaseConnectionMgr()->listen(szHost, nPort, true, "CLoginConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser, eCCT_Websocket))
#else
	if (!this->getBaseConnectionMgr()->listen(szHost, nPort, true, "CLoginConnectionFromClient", szBuf, nSendBufSize, nRecvBufSize, default_client_message_parser, eCCT_Normal))
#endif
	{
		PrintWarning("gate listen error");
		return false;
	}
	
	PrintInfo("CGateService::onInit");

	return true;
}

void CLoginService::onFrame()
{

}

void CLoginService::onQuit()
{
	PrintInfo("CLoginService::onQuit");
	this->doQuit();
}

CLoginClientMessageDispatcher* CLoginService::getLoginClientMessageDispatcher() const
{
	return this->m_pLoginClientMessageDispatcher.get();
}

void CLoginService::release()
{
	delete this;

	google::protobuf::ShutdownProtobufLibrary();
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CLoginService(sServiceBaseInfo, szConfigFileName);
}