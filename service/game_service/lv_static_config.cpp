#include "lv_static_config.h"

#include "libBaseCommon/csv_parser.h"
#include "libBaseCommon/string_util.h"
#include "libBaseCommon/debug_helper.h"

CLvStaticConfig::CLvStaticConfig()
{

}

CLvStaticConfig::~CLvStaticConfig()
{

}

bool CLvStaticConfig::init()
{
	base::CCSVParser sCSVParser;
	if (!sCSVParser.load(this->getConfigName()))
		return false;

	for (uint32_t i = 0; i < sCSVParser.getRowCount(); ++i)
	{
		SLvConfigInfo sLvConfigInfo;
		sCSVParser.getValue(i, "lv", sLvConfigInfo.nID);
		sCSVParser.getValue(i, "base_work_gold", sLvConfigInfo.nBaseWorkGold);
		sCSVParser.getValue(i, "base_adventure_gold", sLvConfigInfo.nBaseAdventureGold);
		
		this->m_mapLvConfigInfo[sLvConfigInfo.nID] = sLvConfigInfo;
	}

	return true;
}

uint32_t CLvStaticConfig::getConfigType() const
{
	return eSCT_Lv;
}

std::string CLvStaticConfig::getConfigName()
{
	return "csv/lv.csv";
}


const SLvConfigInfo* CLvStaticConfig::getLvConfigInfo(uint32_t nID) const
{
	auto iter = this->m_mapLvConfigInfo.find(nID);
	if (iter == this->m_mapLvConfigInfo.end())
		return nullptr;

	return &iter->second;
}

void CLvStaticConfig::release()
{
	base::CSingleton<CLvStaticConfig>::release();
}
