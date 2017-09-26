#include "hero_patch_static_config.h"

#include "libBaseCommon/csv_parser.h"
#include "libBaseCommon/string_util.h"
#include "libBaseCommon/debug_helper.h"

CHeroPatchStaticConfig::CHeroPatchStaticConfig()
{

}

CHeroPatchStaticConfig::~CHeroPatchStaticConfig()
{

}

bool CHeroPatchStaticConfig::init()
{
	base::CCSVParser sCSVParser;
	if (!sCSVParser.load(this->getConfigName()))
		return false;

	for (uint32_t i = 0; i < sCSVParser.getRowCount(); ++i)
	{
		SHeroPatchConfigInfo sHeroPatchConfigInfo;
		sCSVParser.getValue(i, "id", sHeroPatchConfigInfo.nID);
		sCSVParser.getValue(i, "name", sHeroPatchConfigInfo.szName);
		sCSVParser.getValue(i, "req_item_id", sHeroPatchConfigInfo.nActiveReqItemID);
		sCSVParser.getValue(i, "req_lv", sHeroPatchConfigInfo.nActiveReqLv);
		sCSVParser.getValue(i, "req_patch_id", sHeroPatchConfigInfo.nActiveReqPatchID);
		sCSVParser.getValue(i, "req_hero_id", sHeroPatchConfigInfo.nActiveReqHeroID);

		std::string szCost;
		sCSVParser.getValue(i, "cost", szCost);
		std::vector<std::string> vecCost;
		base::string_util::split(szCost, vecCost, ";");
		for (size_t j = 0; j < vecCost.size(); ++j)
		{
			std::vector<std::string> vecAttr;
			base::string_util::split(vecCost[j], vecAttr, ":");
			DebugAstEx(vecAttr.size() == 2, false);

			SAttributeValue sAttributeValue;
			DebugAstEx(base::string_util::convert_to_value(vecAttr[0], sAttributeValue.nType), false);
			DebugAstEx(base::string_util::convert_to_value(vecAttr[1], sAttributeValue.nValue), false);

			sHeroPatchConfigInfo.vecActiveCost.push_back(sAttributeValue);
		}

		this->m_mapHeroPatchConfigInfo[sHeroPatchConfigInfo.nID] = sHeroPatchConfigInfo;
	}

	return true;
}

uint32_t CHeroPatchStaticConfig::getConfigType() const
{
	return eSCT_HeroPatch;
}

std::string CHeroPatchStaticConfig::getConfigName()
{
	return "csv/hero_patch.csv";
}

const SHeroPatchConfigInfo* CHeroPatchStaticConfig::getHeroPatchConfigInfo(uint32_t nID) const
{
	auto iter = this->m_mapHeroPatchConfigInfo.find(nID);
	if (iter == this->m_mapHeroPatchConfigInfo.end())
		return nullptr;

	return &iter->second;
}

void CHeroPatchStaticConfig::release()
{
	base::CSingleton<CHeroPatchStaticConfig>::release();
}
