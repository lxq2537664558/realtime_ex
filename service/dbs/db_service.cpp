#include "db_service.h"

#include "tinyxml2/tinyxml2.h"

#include "libBaseDatabase/database.h"

using namespace core;

CDbService::CDbService()
	: m_pDbServiceMessageHandler(nullptr)
	, m_nDbID(0)
{
}

CDbService::~CDbService()
{
	SAFE_DELETE(this->m_pDbServiceMessageHandler);
}

bool CDbService::onInit()
{
	this->m_pDbServiceMessageHandler = new CDbServiceMessageHandler(this);
	
	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	
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

	tinyxml2::XMLElement* pCacheInfoXML = pRootXML->FirstChildElement("cache_info");
	DebugAstEx(pCacheInfoXML != nullptr, false);

	const std::string szProtoDir = pCacheInfoXML->Attribute("proto_dir");
	uint32_t nThreadCount = pCacheInfoXML->UnsignedAttribute("thread_count");
	uint32_t nCacheSize = pCacheInfoXML->UnsignedAttribute("cache_size");
	uint32_t nWriteBackTime = pCacheInfoXML->UnsignedAttribute("write_back_time");

	tinyxml2::XMLElement* pMysqlXML = pRootXML->FirstChildElement("mysql");
	DebugAstEx(pMysqlXML != nullptr, false);

	const std::string szHost = pMysqlXML->Attribute("host");
	uint16_t nPort = (uint16_t)pMysqlXML->UnsignedAttribute("port");
	const std::string szUser = pMysqlXML->Attribute("user");
	const std::string szPassword = pMysqlXML->Attribute("password");
	const std::string szName = pMysqlXML->Attribute("name");
	const std::string szCharset = pMysqlXML->Attribute("charset");

	this->m_nDbID = base::db::create(szHost, nPort, szName, szUser, szPassword, szCharset, szProtoDir, nThreadCount, nCacheSize, nWriteBackTime);
	if (0 == this->m_nDbID)
	{
		PrintWarning("0 == this->m_nDbID");
		return false;
	}

	SAFE_DELETE(pConfigXML);

	PrintInfo("CDbService::onInit");

	return true;
}

void CDbService::onFrame()
{
	base::db::update(this->m_nDbID);
}

void CDbService::onQuit()
{

}

void CDbService::release()
{
	delete this;
}

CProtobufFactory* CDbService::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

uint32_t CDbService::getDbID() const
{
	return this->m_nDbID;
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CDbService();
}