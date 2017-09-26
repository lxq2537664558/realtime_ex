#pragma once
#include "shell_common.h"
#include <vector>

struct SBattleDamageInfo
{
	uint32_t	nDefHeroID;
	uint32_t	nDamage;
	uint8_t		nDamageType;
};

struct SBattleAttackInfo
{
	uint32_t	nAtkHeroID;
	uint32_t	nSkillID;
	std::vector<SBattleDamageInfo>
				vecBattleDamageInfo;
};

struct SBattleFrameInfo
{
	uint32_t	nFrameIndex;	// µÚ¼¸Ö¡
	std::vector<SBattleAttackInfo>
				vecBattleAttackInfo;
};

struct SBattleInfo
{
	uint64_t	nAtkID;
	std::string	szAtkName;
	uint32_t	nAtkLevel;
	uint32_t	nAtkBattleValue;
	uint64_t	nDefID;
	std::string	szDefName;
	uint32_t	nDefLevel;
	uint32_t	nDefBattleValue;
	std::vector<SBattleFrameInfo>
				vecBattleFrameInfo;
};

class CBattle
{
public:

};