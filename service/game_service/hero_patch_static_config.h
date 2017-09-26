#pragma once

#include "shell_common.h"

#include "base_static_config.h"

#include <vector>
#include <map>

#include "libBaseCommon/singleton.h"

struct SHeroPatchConfigInfo
{
	uint32_t	nID;
	std::string	szName;
	uint32_t	nActiveReqItemID;
	uint32_t	nActiveReqPatchID;			// ������Ҫ����Ƭid
	uint32_t	nActiveReqHeroID;			// ������Ҫ��Ӣ��id
	uint16_t	nActiveReqLv;				// ����Ӣ����Ҫ�ĵȼ�
	std::vector<SAttributeValue>
				vecActiveCost;				// ��������

	SHeroPatchConfigInfo()
	{
		this->nID = 0;
		this->nActiveReqItemID = 0;
		this->nActiveReqPatchID = 0;
		this->nActiveReqHeroID = 0;
		this->nActiveReqLv = 0;
	}
};

class CHeroPatchStaticConfig :
	public CBaseStaticConfig,
	public base::CSingleton<CHeroPatchStaticConfig>
{
public:
	CHeroPatchStaticConfig();
	~CHeroPatchStaticConfig();

	virtual bool				init();
	virtual uint32_t			getConfigType() const;
	virtual std::string			getConfigName();
	virtual void				release();
	const SHeroPatchConfigInfo*	getHeroPatchConfigInfo(uint32_t nID) const;

private:
	std::map<uint32_t, SHeroPatchConfigInfo>	m_mapHeroPatchConfigInfo;
};