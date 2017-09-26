#pragma once

#include "shell_common.h"

#include "base_static_config.h"

#include <vector>
#include <map>

#include "libBaseCommon/singleton.h"

struct SItemConfigInfo
{
	uint32_t	nID;
	std::string	szName;
	uint32_t	nType;
	uint32_t	nSubType;

	SItemConfigInfo()
	{
		this->nID = 0;
		this->nType = 0;
		this->nSubType = 0;
	}
};

class CItemStaticConfig :
	public CBaseStaticConfig,
	public base::CSingleton<CItemStaticConfig>
{
public:
	CItemStaticConfig();
	~CItemStaticConfig();

	virtual bool		    init();
	virtual uint32_t	    getConfigType() const;
	virtual std::string     getConfigName();
	virtual void			release();
	const SItemConfigInfo*	getItemConfigInfo(uint32_t nID) const;

private:
	std::map<uint32_t, SItemConfigInfo>	m_mapItemConfigInfo;
};