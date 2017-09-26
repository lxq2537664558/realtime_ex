#include "hero_static_config.h"

#include "libBaseCommon/csv_parser.h"
#include "libBaseCommon/string_util.h"
#include "libBaseCommon/debug_helper.h"

CHeroStaticConfig::CHeroStaticConfig()
{

}

CHeroStaticConfig::~CHeroStaticConfig()
{

}

bool CHeroStaticConfig::init()
{
	base::CCSVParser sCSVParser;
	if (!sCSVParser.load(this->getConfigName()))
		return false;

	for (uint32_t i = 0; i < sCSVParser.getRowCount(); ++i)
	{
		SHeroConfigInfo sHeroConfigInfo;
		sCSVParser.getValue(i, "id", sHeroConfigInfo.nID);
		sCSVParser.getValue(i, "name", sHeroConfigInfo.szName);
		sCSVParser.getValue(i, "quality", sHeroConfigInfo.nQuality);
		sCSVParser.getValue(i, "occupation", sHeroConfigInfo.nOccupation);
		sCSVParser.getValue(i, "sex", sHeroConfigInfo.nSex);
		sCSVParser.getValue(i, "req_lv", sHeroConfigInfo.nActiveReqLv);
		sCSVParser.getValue(i, "req_patch_id", sHeroConfigInfo.nActiveReqPatchID);

		sCSVParser.getValue(i, "base_hp", sHeroConfigInfo.nBaseHP);
		sCSVParser.getValue(i, "grow_up_hp", sHeroConfigInfo.nGrowupHP);
		sCSVParser.getValue(i, "base_def", sHeroConfigInfo.nBaseDef);
		sCSVParser.getValue(i, "grow_up_def", sHeroConfigInfo.nGrowupDef);
		sCSVParser.getValue(i, "base_atk", sHeroConfigInfo.nBaseAtk);
		sCSVParser.getValue(i, "grow_up_atk", sHeroConfigInfo.nGrowupAtk);
		sCSVParser.getValue(i, "atk_cd", sHeroConfigInfo.nAtkCD);
		sCSVParser.getValue(i, "crit", sHeroConfigInfo.nCrit);
		sCSVParser.getValue(i, "dodge", sHeroConfigInfo.nDodge);
		sCSVParser.getValue(i, "avoid_injury", sHeroConfigInfo.nAvoidInjury);

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

			sHeroConfigInfo.vecActiveCost.push_back(sAttributeValue);
		}

		std::string szSkill;
		sCSVParser.getValue(i, "skill", szSkill);
		std::vector<std::string> vecSkill;
		base::string_util::split(szSkill, vecSkill, ";");
		for (size_t j = 0; j < vecSkill.size(); ++j)
		{
			uint32_t nSkillID = 0;
			DebugAstEx(base::string_util::convert_to_value(vecSkill[j], nSkillID), false);
			
			sHeroConfigInfo.vecSkillID.push_back(nSkillID);
		}

		this->m_mapHeroConfigInfo[sHeroConfigInfo.nID] = sHeroConfigInfo;
	}

	return true;
}

uint32_t CHeroStaticConfig::getConfigType() const
{
	return eSCT_Hero;
}

std::string CHeroStaticConfig::getConfigName()
{
	return "csv/hero.csv";
}

const SHeroConfigInfo* CHeroStaticConfig::getHeroConfigInfo(uint32_t nID) const
{
	auto iter = this->m_mapHeroConfigInfo.find(nID);
	if (iter == this->m_mapHeroConfigInfo.end())
		return nullptr;

	return &iter->second;
}

void CHeroStaticConfig::release()
{
	base::CSingleton<CHeroStaticConfig>::release();
}
