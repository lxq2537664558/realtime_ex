#include "stdafx.h"
#include "gate_app.h"
#include "connection_from_client.h"
#include "connection_from_service.h"
#include "connection_to_master.h"

#include "libCoreCommon/base_connection_mgr.h"

#define _CHECK_CONNECT_TIME 5*1000

CGateApp::CGateApp()
	: m_pServiceMgr(nullptr)
	, m_nMasterPort(0)
{
	this->m_tickCheckConnect.setCallback(&CGateApp::onCheckConnect, this);
}

CGateApp::~CGateApp()
{

}

CGateApp* CGateApp::Inst()
{
	return static_cast<CGateApp*>(CCoreApp::Inst());
}

bool CGateApp::onInit()
{
	if (!core::CCoreApp::onInit())
		return false;
	
	CConnectionFromClient::registClassInfo();
	CConnectionFromService::registClassInfo();
	CConnectionToMaster::registClassInfo();

	this->m_pServiceMgr = new CServiceMgr();
	if (!this->m_pServiceMgr->init())
		return false;

	tinyxml2::XMLElement* pMasterAddrXML = this->m_pRootXML->FirstChildElement("connect_master_addr");
	DebugAstEx(pMasterAddrXML != nullptr, false);
	const char* szMasterHost = pMasterAddrXML->Attribute("host");
	this->m_szMasterHost = szMasterHost;
	this->m_nMasterPort = (uint16_t)pMasterAddrXML->IntAttribute("port");
	
	this->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, "master", _GET_CLASS_NAME(CConnectionToMaster), 10 * 1024, 10 * 1024, nullptr);
	this->registTicker(&this->m_tickCheckConnect, _CHECK_CONNECT_TIME, _CHECK_CONNECT_TIME, 0);

	tinyxml2::XMLElement* pListenClientAddrXML = this->m_pRootXML->FirstChildElement("listen_client_addr");
	DebugAstEx(pListenClientAddrXML != nullptr, false);
	this->getBaseConnectionMgr()->listen(pListenClientAddrXML->Attribute("host"), (uint16_t)pListenClientAddrXML->IntAttribute("port"), "", _GET_CLASS_NAME(CConnectionFromClient), pListenClientAddrXML->IntAttribute("send_buf_size"), pListenClientAddrXML->IntAttribute("recv_buf_size"), core::default_parser_raw_data);

	this->getBaseConnectionMgr()->listen(this->getServiceBaseInfo().szHost, this->getServiceBaseInfo().nPort, "", _GET_CLASS_NAME(CConnectionFromService), this->getServiceBaseInfo().nSendBufSize, this->getServiceBaseInfo().nRecvBufSize, nullptr);
	
	return true;
}

void CGateApp::onDestroy()
{
	SAFE_DELETE(this->m_pServiceMgr);
}

void CGateApp::onCheckConnect(uint64_t nContext)
{
	// 断线重连，这种逻辑一般情况下是没有问题的
	uint32_t nCount = this->getBaseConnectionMgr()->getBaseConnectionCount(_GET_CLASS_NAME(CConnectionToMaster));
	if (nCount == 0)
		this->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, "master", _GET_CLASS_NAME(CConnectionToMaster), 10 * 1024, 10 * 1024, nullptr);
}

void CGateApp::onQuit()
{
	this->doQuit();
}

CServiceMgr* CGateApp::getServiceMgr() const
{
	return this->m_pServiceMgr;
}

int32_t main(int32_t argc, char* argv[])
{
	CGateApp* pGateApp = new CGateApp();
	pGateApp->run(argc, argv, "gate_config.xml");

	return 0;
}