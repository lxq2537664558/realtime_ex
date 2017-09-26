#pragma once

#include <stdint.h>
#include <string>
#include <map>

struct SBattleArrayInfo
{
	std::map<uint32_t, uint32_t>	mapHeroPosition;
};

enum EBattleArrayType
{
	eBT_Normal = 0,	// Õ®”√’Û»›

};

class IPlayerBattleArrayModule
{
public:
	virtual ~IPlayerBattleArrayModule() {}

	virtual const SBattleArrayInfo*	getBattleArrayInfo(EBattleArrayType eType) = 0;
	virtual void					updateBattleArrayInfo(EBattleArrayType eType, const SBattleArrayInfo& sBattleArrayInfo) = 0;
};