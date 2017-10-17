#include "dispatch_service.h"
#include "dispatch_service_message_handler.h"

#include "tinyxml2/tinyxml2.h"

CDispatchService::CDispatchService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
	, m_nGlobalDbServiceID(0)
	, m_nAccountTblCount(1)
{

}

CDispatchService::~CDispatchService()
{

}

void CDispatchService::release()
{
	delete this;

	google::protobuf::ShutdownProtobufLibrary();
}

COnlineCountMgr* CDispatchService::getOnlineCountMgr() const
{
	return this->m_pOnlineCountMgr.get();
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

	tinyxml2::XMLElement* pGlobalDbsXML = pRootXML->FirstChildElement("global_dbs_info");
	DebugAstEx(pGlobalDbsXML != nullptr, false);

	this->m_nAccountTblCount = pGlobalDbsXML->UnsignedAttribute("account_tbl_count");
	this->m_nGlobalDbServiceID = pGlobalDbsXML->UnsignedAttribute("global_dbs_id");
	
	DebugAstEx(this->m_nAccountTblCount != 0, false);
	DebugAstEx(this->m_nGlobalDbServiceID != 0, false);
	
	this->m_pNormalProtobufSerializer = std::make_unique<CNormalProtobufSerializer>();
	
	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer.get());

	this->setServiceMessageSerializer(0, eMST_Protobuf);
	
	this->m_pDispatchServiceMessageHandler = std::make_unique<CDispatchServiceMessageHandler>(this);
	this->m_pOnlineCountMgr = std::make_unique<COnlineCountMgr>();

	PrintInfo("CDispatchService::onInit");

	return true;
}

void CDispatchService::onFrame()
{
}

void CDispatchService::onQuit()
{
	PrintInfo("CDispatchService::onQuit");
	this->doQuit();
}

extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CDispatchService(sServiceBaseInfo, szConfigFileName);
}