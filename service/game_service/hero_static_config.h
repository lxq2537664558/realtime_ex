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
	uint16_t	nQuality;		// Ʒ��
	uint16_t	nOccupation;	// ְҵ
	uint16_t	nSex;			// �Ա�
	uint16_t	nLevel;			// �ȼ�
	uint16_t	nActiveReqLv;	// ����Ӣ����Ҫ�ĵȼ�
	uint32_t	nActiveReqPatchID;	// ����Ӣ����Ҫ����Ƭid
	std::vector<SAttributeValue>
				vecActiveCost;	// ��������
	
	uint32_t	nBaseHP;		// ��������
	uint32_t	nGrowupHP;		// �����ɳ�
	uint32_t	nBaseDef;		// ��������
	uint32_t	nGrowupDef;		// �����ɳ�
	uint32_t	nBaseAtk;		// ��������
	uint32_t	nGrowupAtk;		// �����ɳ�
	float		nAtkCD;			// ����CD
	float		nCrit;			// ������
	float		nDodge;			// ������
	float		nAvoidInjury;	// ������
	uint32_t	nNormalSkillID;	// ��ͨ��������ID
	std::vector<uint32_t>
				vecSkillID;		// Ӣ�ۼ���

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