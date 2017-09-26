#pragma once

#include "shell_common.h"

#include "base_static_config.h"

#include <vector>
#include <map>

#include "libBaseCommon/singleton.h"

struct SLvConfigInfo
{
	uint32_t	nID;
	uint32_t	nBaseWorkGold;		// 基础内城打工金币
	uint32_t	nBaseAdventureGold;	// 基础冒险金币

	SLvConfigInfo()
	{
		this->nID = 0;
		this->nBaseWorkGold = 0;
		this->nBaseAdventureGold = 0;
	}
};

class CLvStaticConfig :
	public CBaseStaticConfig,
	public base::CSingleton<CLvStaticConfig>
{
public:
	CLvStaticConfig();
	~CLvStaticConfig();

	virtual bool		    init();
	virtual uint32_t	    getConfigType() const;
	virtual std::string     getConfigName();
	virtual void			release();
	const SLvConfigInfo*	getLvConfigInfo(uint32_t nID) const;

private:
	std::map<uint32_t, SLvConfigInfo>	m_mapLvConfigInfo;
};