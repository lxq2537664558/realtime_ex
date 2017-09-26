#pragma once
#include <stdint.h>
#include <string>

class IPlayerHeroModule
{
public:
	virtual ~IPlayerHeroModule() {}

	virtual bool	isPatchActive(uint32_t nID) const = 0;
	virtual bool	isHeroActive(uint32_t nID) const = 0;

	virtual void	activePatch(uint32_t nID) = 0;
	virtual void	activeHero(uint32_t nID) = 0;
};