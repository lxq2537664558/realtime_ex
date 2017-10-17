#pragma once

#include "shell_common.h"

#include "base_static_config.h"

#include <vector>
#include <map>

#include "libBaseCommon/singleton.h"

struct SHeroConfigInfo
{
	uint32_t	nID;
	std::string	szName;
	uint16_t	nQuality;		// 品质
	uint16_t	nOccupation;	// 职业
	uint16_t	nSex;			// 性别
	uint16_t	nLevel;			// 等级
	uint16_t	nActiveReqLv;	// 解锁英雄需要的等级
	uint32_t	nActiveReqPatchID;	// 解锁英雄需要的碎片id
	std::vector<SAttributeValue>
				vecActiveCost;	// 解锁消耗
	
	uint32_t	nBaseHP;		// 基础生命
	uint32_t	nGrowupHP;		// 生命成长
	uint32_t	nBaseDef;		// 基础防御
	uint32_t	nGrowupDef;		// 防御成长
	uint32_t	nBaseAtk;		// 基础攻击
	uint32_t	nGrowupAtk;		// 攻击成长
	float		nAtkCD;			// 攻击CD
	float		nCrit;			// 暴击率
	float		nDodge;			// 闪避率
	float		nAvoidInjury;	// 免伤率
	uint32_t	nNormalSkillID;	// 普通攻击技能ID
	std::vector<uint32_t>
				vecSkillID;		// 英雄技能

	SHeroConfigInfo()
	{
		this->nID = 0;
		this->nQuality = 0;
		this->nOccupation = 0;
		this->nSex = 0;
		this->nLevel = 0;
		this->nActiveReqLv = 0;
		this->nActiveReqPatchID = 0;
		this->nBaseHP = 0;
		this->nGrowupHP = 0;
		this->nBaseDef = 0;
		this->nGrowupDef = 0;
		this->nBaseAtk = 0;
		this->nGrowupAtk = 0;
		this->nAtkCD = 0;
		this->nCrit = 0;
		this->nDodge = 0;
		this->nAvoidInjury = 0;
	}
};

class CHeroStaticConfig :
	public CBaseStaticConfig,
	public base::CSingleton<CHeroStaticConfig>
{
public:
	CHeroStaticConfig();
	~CHeroStaticConfig();

	virtual bool		    init();
	virtual uint32_t	    getConfigType() const;
	virtual std::string     getConfigName();
	virtual void			release();
	const SHeroConfigInfo*	getHeroConfigInfo(uint32_t nID) const;

private:
	std::map<uint32_t, SHeroConfigInfo>	m_mapHeroConfigInfo;
};