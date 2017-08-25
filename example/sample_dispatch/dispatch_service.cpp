#include "dispatch_service.h"
#include "dispatch_service_message_handler.h"

#include "tinyxml2/tinyxml2.h"

CDispatchService::CDispatchService()
	: m_pDispatchServiceMessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
	, m_pOnlineCountMgr(nullptr)
	, m_nGlobalDbServiceID(0)
	, m_nAccountTblCount(1)
{

}

CDispatchService::~CDispatchService()
{

}

core::CProtobufFactory* CDispatchService::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

void CDispatchService::release()
{
	delete this;
}

COnlineCountMgr* CDispatchService::getOnlineCountMgr() const
{
	return this->m_pOnlineCountMgr;
}

uint32_t CDispatchService::getGlobalDbServiceID() const
{
	return this->m_nGlobalDbServiceID;
}

uint32_t CDispatchService::getAccountTblCount() const
{
	return this->m_nAccountTblCount;
}

bool CDispatchService::onInit()
{
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

	tinyxml2::XMLElement* pGlobalDbsXML = pRootXML->FirstChildElement("global_dbs_info");
	DebugAstEx(pGlobalDbsXML != nullptr, false);

	this->m_nAccountTblCount = pGlobalDbsXML->UnsignedAttribute("account_tbl_count");
	this->m_nGlobalDbServiceID = pGlobalDbsXML->UnsignedAttribute("global_dbs_id");
	
	DebugAstEx(this->m_nAccountTblCount != 0, false);
	DebugAstEx(this->m_nGlobalDbServiceID != 0, false);

	SAFE_DELETE(pConfigXML);

	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	this->m_pDispatchServiceMessageHandler = new CDispatchServiceMessageHandler(this);
	this->m_pOnlineCountMgr = new COnlineCountMgr();

	PrintInfo("CDispatchService::onInit");

	return true;
}

void CDispatchService::onFrame()
{
}

void CDispatchService::onQuit()
{
	PrintInfo("CDispatchService::onQuit");
}

extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CDispatchService();
}