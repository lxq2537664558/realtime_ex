#pragma once
#include <stdint.h>

#include "event_handler.h"

class IPlayerModuleEvent :
	public IEvent
{
public:
	virtual ~IPlayerModuleEvent() {};
};

enum EPlayerModuleEventType
{
	ePMET_Lvup,
	ePMET_AddItem,
	ePMET_DelItem,
	ePMET_ActiveHero,
	ePMET_ActiveHeroPatch,
};

class CPlayerLvupEvent :
	public IPlayerModuleEvent
{
public:
	virtual uint32_t getEventType() const 
	{
		return ePMET_Lvup;
	}
};

class CPlayerAddItemEvent :
	public IPlayerModuleEvent
{
public:
	virtual uint32_t getEventType() const
	{
		return ePMET_AddItem;
	}

public:
	uint32_t nItemID;
	uint32_t nCount;
};

class CPlayerDelItemEvent :
	public IPlayerModuleEvent
{
public:
	virtual uint32_t getEventType() const
	{
		return ePMET_DelItem;
	}

public:
	uint32_t nItemID;
	uint32_t nCount;
};

class CPlayerActiveHeroEvent :
	public IPlayerModuleEvent
{
public:
	virtual uint32_t getEventType() const
	{
		return ePMET_ActiveHero;
	}

public:
	uint32_t nHeroID;
};

class CPlayerActiveHeroPatchEvent :
	public IPlayerModuleEvent
{
public:
	virtual uint32_t getEventType() const
	{
		return ePMET_ActiveHeroPatch;
	}

public:
	uint32_t nHeroPatchID;
};