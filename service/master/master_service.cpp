#include "stdafx.h"
#include "master_service.h"
#include "connection_from_node.h"

#include "libCoreCommon\base_app.h"
#include "libCoreCommon\base_connection_mgr.h"

#include "tinyxml2\tinyxml2.h"

using namespace core;

DEFINE_OBJECT(CMasterService, 1);

CMasterService::CMasterService()
	: m_pServiceRegistry(nullptr)
	, m_writeBuf(UINT16_MAX)
	, m_pNodeConnectionFactory(nullptr)
	, m_nMasterID(0)
{

}

CMasterService::~CMasterService()
{
	SAFE_DELETE(this->m_pServiceRegistry);
	SAFE_DELETE(this->m_pNodeConnectionFactory);
}

bool CMasterService::onInit()
{
	this->m_pServiceRegistry = new CServiceRegistry(this);
	this->m_nMasterID = this->getServiceID();

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

	tinyxml2::XMLElement* pMasterXML = pRootXML->FirstChildElement("master");
	if (pMasterXML == nullptr)
	{
		PrintWarning("pMasterXML == nullptr");
		return false;
	}

	this->m_pNodeConnectionFactory = new CNodeConnectionFactory();
	CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, this->m_pNodeConnectionFactory);

	const std::string szHost = pMasterXML->Attribute("host") != nullptr ? pMasterXML->Attribute("host") : "0.0.0.0";
	uint16_t nPort = (uint16_t)pMasterXML->UnsignedAttribute("port");

	char szBuf[256] = { 0 };
	base::crt::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	CBaseApp::Inst()->getBaseConnectionMgr()->listen(szHost, nPort, eBCT_ConnectionFromService, szBuf, 10 * 1024, 10 * 1024, nullptr);

	SAFE_DELETE(pConfigXML);

	PrintInfo("CMasterService::onInit");
	return true;
}

void CMasterService::onFrame()
{

}

void CMasterService::onQuit()
{

}

CServiceRegistry* CMasterService::getServiceRegistry() const
{
	return this->m_pServiceRegistry;
}

uint32_t CMasterService::getMasterID() const
{
	return this->m_nMasterID;
}

base::CWriteBuf& CMasterService::getWriteBuf()
{
	this->m_writeBuf.clear();
	return this->m_writeBuf;
}

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CMasterService::registerClassInfo();
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