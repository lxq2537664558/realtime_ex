#include "gm_command_mgr.h"

#include "libBaseCommon/debug_helper.h"
#include "player.h"
#include "../common/error_code.h"

CGMCommandMgr::CGMCommandMgr(CGameService* pGameService)
	: m_pGameService(pGameService)
{
	this->m_mapFunction["add_item"] = std::bind(&CGMCommandMgr::add_item, this, std::placeholders::_1, std::placeholders::_2);
	this->m_mapFunction["add_attr"] = std::bind(&CGMCommandMgr::add_attr, this, std::placeholders::_1, std::placeholders::_2);
}

CGMCommandMgr::~CGMCommandMgr()
{

}

uint32_t CGMCommandMgr::dispatchCommand(CPlayer* pPlayer, const std::string& szName, const std::string& szArg)
{
	DebugAstEx(pPlayer != nullptr, eEC_GM_NameError);

	auto iter = this->m_mapFunction.find(szName);
	if (iter == this->m_mapFunction.end())
		return eEC_GM_NameError;

	auto& callback = iter->second;
	DebugAstEx(callback != nullptr, eEC_GM_NameError);

	std::vector<std::string> vecArg;
	base::string_util::split(szArg, vecArg);

	if (!callback(pPlayer, vecArg))
		return eEC_GM_ArgError;

	return eEC_Sucess;
}

bool CGMCommandMgr::add_item(CPlayer* pPlayer, const std::vector<std::string>& vecArg)
{
	if (vecArg.size() != 2)
		return false;

	uint32_t nID = 0;
	uint32_t nCount = 0;
	
	if (!base::string_util::convert_to_value(vecArg[0], nID))
		return false;
	if (!base::string_util::convert_to_value(vecArg[1], nCount))
		return false;

	pPlayer->getItemModule()->addItem(nID, nCount);
	
	return true;
}

bool CGMCommandMgr::add_attr(CPlayer* pPlayer, const std::vector<std::string>& vecArg)
{
	if (vecArg.size() != 2)
		return false;

	uint32_t nID = 0;
	uint32_t nCount = 0;

	if (!base::string_util::convert_to_value(vecArg[0], nID))
		return false;
	if (!base::string_util::convert_to_value(vecArg[1], nCount))
		return false;

	pPlayer->getAttributeModule()->addAttribute(nID, nCount);

	return true;
}
