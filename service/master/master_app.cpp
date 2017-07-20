#include "stdafx.h"

#include "libCoreCommon/base_connection_mgr.h"

#include "tinyxml2/tinyxml2.h"

#include "master_app.h"
#include "connection_from_node.h"
#include "node_connection_factory.h"

CMasterApp::CMasterApp()
	: m_pServiceRegistry(nullptr)
	, m_pNodeConnectionFactory(nullptr)
	, m_nMasterID(0)
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
	this->m_pServiceRegistry = new CServiceRegistry();

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

	tinyxml2::XMLElement* pMasterXML = pRootXML->FirstChildElement("master");
	if (pMasterXML == nullptr)
	{
		PrintWarning("pMasterXML == nullptr");
		return false;
	}

	this->m_nMasterID = (uint16_t)pMasterXML->UnsignedAttribute("id");

	this->m_pNodeConnectionFactory = new CNodeConnectionFactory();
	this->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, this->m_pNodeConnectionFactory);

	const std::string szHost = pMasterXML->Attribute("host");
	uint16_t nPort = (uint16_t)pMasterXML->UnsignedAttribute("port");
	
	this->getBaseConnectionMgr()->listen(szHost, nPort, eBCT_ConnectionFromService, "", 10 * 1024, 10 * 1024, nullptr);
	
	SAFE_DELETE(pConfigXML);

	return true;
}

void CMasterApp::onDestroy()
{
	this->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, nullptr);

	SAFE_DELETE(this->m_pServiceRegistry);
	SAFE_DELETE(this->m_pNodeConnectionFactory);
}

bool CMasterApp::onProcess()
{
	return true;
}

CServiceRegistry* CMasterApp::getServiceRegistry() const
{
	return this->m_pServiceRegistry;
}

uint16_t CMasterApp::getMasterID() const
{
	return this->m_nMasterID;
}

int32_t main(int32_t argc, char* argv[])
{
	CMasterApp* pMasterApp = new CMasterApp();
	pMasterApp->run(argv[0], "master_config.xml");
	SAFE_DELETE(pMasterApp);

	return 0;
}