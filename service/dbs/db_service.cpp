#include "db_service.h"

#include "tinyxml2/tinyxml2.h"

#include "libBaseDatabase/database.h"

using namespace core;

CDbService::CDbService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
	, m_nDbID(0)
{
}

CDbService::~CDbService()
{
}

bool CDbService::onInit()
{
	this->m_pDbServiceMessageHandler = std::make_unique<CDbServiceMessageHandler>(this);
	
	this->m_pNormalProtobufSerializer = std::make_unique<CNormalProtobufSerializer>();
	
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

	base::db::SCacheConfigInfo sCacheConfigInfo;
	sCacheConfigInfo.nMaxCacheSize = nCacheSize;
	sCacheConfigInfo.nWritebackTime = nWriteBackTime;

	tinyxml2::XMLElement* pTableCacheXML = pRootXML->FirstChildElement("cache_table");
	if (pTableCacheXML != nullptr)
	{
		for (tinyxml2::XMLElement* pTableInfoXML = pTableCacheXML->FirstChildElement("table_info"); pTableInfoXML != nullptr; pTableInfoXML = pTableInfoXML->NextSiblingElement("table_info"))
		{
			std::string szName = pTableInfoXML->Attribute("name");
			sCacheConfigInfo.vecTable.push_back(szName);
		}
	}

	this->m_nDbID = base::db::create(szHost, nPort, szName, szUser, szPassword, szCharset, szProtoDir, nThreadCount, sCacheConfigInfo);
	if (0 == this->m_nDbID)
	{
		PrintWarning("0 == this->m_nDbID");
		return false;
	}

	PrintInfo("connect db successful addr: {}:{} db_name: {} charset: {}", szHost, nPort, szName, szCharset);
	PrintInfo("CDbService::onInit");

	return true;
}

void CDbService::onFrame()
{
	base::db::update(this->m_nDbID);
}

void CDbService::onQuit()
{
	PrintInfo("CDbService::onQuit");
	if (this->m_nDbID != 0)
	{
		base::db::release(this->m_nDbID);
		this->m_nDbID = 0;
	}
	
	this->doQuit();
}

void CDbService::release()
{
	delete this;
	google::protobuf::ShutdownProtobufLibrary();
}

uint32_t CDbService::getDbID() const
{
	return this->m_nDbID;
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CDbService(sServiceBaseInfo, szConfigFileName);
}