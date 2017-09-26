#pragma once
#include <stdint.h>
#include <string>

class IPlayerItemModule
{
public:
	virtual ~IPlayerItemModule() {}

	virtual void		addItem(uint32_t nID, uint32_t nCount, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
	virtual void		delItem(uint32_t nID, uint32_t nCount, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
	virtual uint32_t	getItemCount(uint32_t nID) const = 0;
};