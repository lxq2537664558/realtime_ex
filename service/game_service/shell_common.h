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
	eHOT_Soldier	= 1,	// սʿ
	eHOT_Shooter	= 2,	// ����
	eHOT_Master		= 3,	// ��ʦ
};

enum EAttributeType
{
	eAT_Lv			= 1,	// �ȼ�
	eAT_Gold		= 2,	// ���
	eAT_Money		= 3,	// שʯ
	eAT_Vitality	= 4,	// ����
	eAT_Exp			= 5,	// ����
	eAT_Face		= 6,	// ͷ��

	eAT_Count,
};

enum EBattleDamageType
{
	eBDT_Normal = 0,	// ��ͨ�˺�	
	eBDT_Crit = 1,		// ����
	eBDT_Dodge = 2,		// ����

	eBDT_Dead = 1<<6,	// ���� 
};

enum ESkillCampType
{
	eSCT_Enemy,
};

enum ESkillTargetType	//����Ŀ������
{	
	
};

#define _PERCENT_VALUE 10000

// ����λ��
enum EBattleArrayPosition
{
	eBAP_1 = 1,
	eBAP_2 = 2,
	eBAP_3 = 3,
	eBAP_4 = 4,
	eBAP_5 = 5,
	eBAP_6 = 6,
};

//����
struct SBattleArrayInfo
{
	std::map<uint32_t, uint32_t>	mapHeroPosition;	// λ�ã�Ӣ��id
};

// ս������
enum EBattleType
{
	eBT_None	= 0,
	eBT_PVE		= 1,
	eBT_PVP		= 2,
};

enum EBattleArrayType
{
	eBAT_Normal = 0,	// ͨ������

};