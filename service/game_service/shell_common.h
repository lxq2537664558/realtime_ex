#pragma once
#include <stdint.h>

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