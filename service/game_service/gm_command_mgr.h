#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <functional>

class CGameService;
class CPlayer;
class CGMCommandMgr
{
public:
	CGMCommandMgr(CGameService* pGameService);
	~CGMCommandMgr();

	uint32_t	dispatchCommand(CPlayer* pPlayer, const std::string& szName, const std::string& szArg);

private:
	bool		add_item(CPlayer* pPlayer, const std::vector<std::string>& vecArg);
	bool		add_attr(CPlayer* pPlayer, const std::vector<std::string>& vecArg);

private:
	CGameService*	m_pGameService;
	std::map<std::string, std::function<bool(CPlayer*, const std::vector<std::string>&)>>
					m_mapFunction;
};