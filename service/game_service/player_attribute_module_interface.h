#pragma once
#include <stdint.h>
#include <string>

#include "shell_common.h"

class IPlayerAttributeModule
{
public:
	virtual ~IPlayerAttributeModule() {}

	virtual const std::string&	getAccountName() const = 0;
	virtual uint32_t			getServerID() const = 0;
	virtual const std::string&	getName() const = 0;
	virtual int64_t				getLastLoginTime() const = 0;
	virtual int64_t				getLastLogoutTime() const = 0;

	virtual uint32_t			getFace() const = 0;
	virtual uint32_t			getLv() const = 0;
	virtual uint64_t			getExp() const = 0;
	virtual uint64_t			getGold() const = 0;
	virtual uint64_t			getMoney() const = 0;
	virtual uint64_t			getVitality() const = 0;

	virtual void				addLv(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
	virtual void				addExp(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
	virtual void				addGold(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
	virtual void				addMoney(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
	virtual void				addVitality(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;

	virtual int64_t				getAttribute(uint32_t nType) const = 0;
	virtual void				setAttribute(uint32_t nType, int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
	virtual void				addAttribute(uint32_t nType, int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr) = 0;
};