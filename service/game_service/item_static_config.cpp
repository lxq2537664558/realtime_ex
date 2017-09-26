#include "item_static_config.h"

#include "libBaseCommon/csv_parser.h"
#include "libBaseCommon/string_util.h"
#include "libBaseCommon/debug_helper.h"

CItemStaticConfig::CItemStaticConfig()
{

}

CItemStaticConfig::~CItemStaticConfig()
{

}

bool CItemStaticConfig::init()
{
	base::CCSVParser sCSVParser;
	if (!sCSVParser.load(this->getConfigName()))
		return false;

	for (uint32_t i = 0; i < sCSVParser.getRowCount(); ++i)
	{
		SItemConfigInfo sItemConfigInfo;
		sCSVParser.getValue(i, "id", sItemConfigInfo.nID);
		sCSVParser.getValue(i, "name", sItemConfigInfo.szName);
		sCSVParser.getValue(i, "type", sItemConfigInfo.nType);
		sCSVParser.getValue(i, "sub_type", sItemConfigInfo.nSubType);

		this->m_mapItemConfigInfo[sItemConfigInfo.nID] = sItemConfigInfo;
	}

	return true;
}

uint32_t CItemStaticConfig::getConfigType() const
{
	return eSCT_Item;
}

std::string CItemStaticConfig::getConfigName()
{
	return "csv/item.csv";
}

const SItemConfigInfo* CItemStaticConfig::getItemConfigInfo(uint32_t nID) const
{
	auto iter = this->m_mapItemConfigInfo.find(nID);
	if (iter == this->m_mapItemConfigInfo.end())
		return nullptr;

	return &iter->second;
}

void CItemStaticConfig::release()
{
	base::CSingleton<CItemStaticConfig>::release();
}
