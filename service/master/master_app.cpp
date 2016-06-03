#include "stdafx.h"
#include "master_app.h"
#include "connection_from_service.h"
#include "service_connection_factory.h"

#include "libCoreCommon/base_connection_mgr.h"

#include "tinyxml2/tinyxml2.h"

CMasterApp::CMasterApp()
	: m_pServiceMgr(nullptr)
	, m_pServiceConnectionFactory(nullptr)
{

}

CMasterApp::~CMasterApp()
{

}

CMasterApp* CMasterApp::Inst()
{
	return static_cast<CMasterApp*>(CBaseApp::Inst());
}

bool CMasterApp::onInit()
{
	this->m_pServiceMgr = new CServiceMgr();
	if (!this->m_pServiceMgr->init())
		return false;

	tinyxml2::XMLDocument* pConfigXML = new tinyxml2::XMLDocument();
	if (pConfigXML->LoadFile(CBaseApp::Inst()->getConfigFileName().c_str()) != tinyxml2::XML_SUCCESS)
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

	tinyxml2::XMLElement* pHostInfoXML = pRootXML->FirstChildElement("host_info");
	if (pHostInfoXML == nullptr)
	{
		PrintWarning("pHostInfoXML == nullptr");
		return false;
	}

	this->m_pServiceConnectionFactory = new CServiceConnectionFactory();
	this->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, this->m_pServiceConnectionFactory);

	const std::string szHost = pHostInfoXML->Attribute("host");
	uint16_t nPort = (uint16_t)pHostInfoXML->UnsignedAttribute("port");
	uint32_t nRecvBufSize = pHostInfoXML->UnsignedAttribute("recv_buf_size");
	uint32_t nSendBufSize = pHostInfoXML->UnsignedAttribute("send_buf_size");
	
	this->getBaseConnectionMgr()->listen(szHost, nPort, eBCT_ConnectionFromService, "", nSendBufSize, nRecvBufSize);

	return true;
}

void CMasterApp::onDestroy()
{
	this->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, nullptr);

	SAFE_DELETE(this->m_pServiceMgr);
	SAFE_DELETE(this->m_pServiceConnectionFactory);
}

void CMasterApp::onQuit()
{
	this->doQuit();
}

CServiceMgr* CMasterApp::getServiceMgr() const
{
	return this->m_pServiceMgr;
}

int32_t main(int32_t argc, char* argv[])
{
	CMasterApp* pMasterApp = new CMasterApp();
	pMasterApp->run(argc, argv, "master_config.xml");

	return 0;
}