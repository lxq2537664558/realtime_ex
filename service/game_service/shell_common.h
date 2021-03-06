#pragma once
#include <stdint.h>
#include <map>

struct SAttributeValue
{
	uint32_t	nType;
	uint32_t	nValue;
};

struct SAttributeValue64
{
	uint32_t	nType;
	uint64_t	nValue;
};

enum EStaticConfigType
{
	eSCT_Lv,
	eSCT_Item,
	eSCT_Hero,
	eSCT_HeroPatch,
};

enum EHeroOccupationType
{
	eHOT_Soldier	= 1,	// 战士
	eHOT_Shooter	= 2,	// 射手
	eHOT_Master		= 3,	// 法师
};

enum EAttributeType
{
	eAT_Lv			= 1,	// 等级
	eAT_Gold		= 2,	// 金币
	eAT_Money		= 3,	// 砖石
	eAT_Vitality	= 4,	// 体力
	eAT_Exp			= 5,	// 经验
	eAT_Face		= 6,	// 头像

	eAT_Count,
};

enum EBattleDamageType
{
	eBDT_Normal = 0,	// 普通伤害	
	eBDT_Crit = 1,		// 暴击
	eBDT_Dodge = 2,		// 闪避

	eBDT_Dead = 1<<6,	// 死亡 
};

enum ESkillCampType
{
	eSCT_Enemy,
};

enum ESkillTargetType	//技能目标类型
{	
	
};

#define _PERCENT_VALUE 10000

// 阵容位置
enum EBattleArrayPosition
{
	eBAP_1 = 1,
	eBAP_2 = 2,
	eBAP_3 = 3,
	eBAP_4 = 4,
	eBAP_5 = 5,
	eBAP_6 = 6,
};

//阵容
struct SBattleArrayInfo
{
	std::map<uint32_t, uint32_t>	mapHeroPosition;	// 位置，英雄id
};

// 战斗类型
enum EBattleType
{
	eBT_None	= 0,
	eBT_PVE		= 1,
	eBT_PVP		= 2,
};

enum EBattleArrayType
{
	eBAT_Normal = 0,	// 通用阵容

};