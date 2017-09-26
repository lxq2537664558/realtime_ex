#pragma once

#include "player_attribute_module_interface.h"
#include "player_module.h"
#include "player.h"

class CPlayerAttributeModule
	: public IPlayerAttributeModule
	, public CPlayerModule
{
public:
	CPlayerAttributeModule(CPlayer* pPlayer);
	virtual ~CPlayerAttributeModule();

	virtual void				onInit();
	virtual void				onDestroy();
	virtual void				onLoadData();
	virtual void				onBackup();
	virtual void				onPlayerLogin();
	virtual void				onPlayerLogout();
	virtual void				onModuleEvent(const IPlayerModuleEvent* pEvent);
	virtual uint32_t			getModuleType() const { return ePMT_Attribute; }

	virtual const std::string&	getAccountName() const;
	virtual uint32_t			getServerID() const;
	virtual const std::string&	getName() const;
	virtual int64_t				getLastLoginTime() const;
	virtual int64_t				getLastLogoutTime() const;

	virtual uint32_t			getFace() const;
	virtual uint32_t			getLv() const;
	virtual uint64_t			getExp() const;
	virtual uint64_t			getGold() const;
	virtual uint64_t			getMoney() const;
	virtual uint64_t			getVitality() const;

	virtual void				addLv(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr);
	virtual void				addExp(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr);
	virtual void				addGold(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr);
	virtual void				addMoney(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr);
	virtual void				addVitality(int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr);

	virtual int64_t				getAttribute(uint32_t nType) const;
	virtual void				setAttribute(uint32_t nType, int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr);
	virtual void				addAttribute(uint32_t nType, int64_t nData, uint32_t nActionType = 0, const void* pContext = nullptr);

private:
	void						syncAttribute(uint32_t nType, uint64_t nData);

private:
	uint64_t	m_nLastLoginTime;
	uint64_t	m_nLastLogoutTime;
	std::string	m_szAccountName;
	uint32_t	m_nServerID;
	std::string m_szName;		// 玩家名字
	uint32_t	m_nFace;		// 头像
	uint32_t	m_nLv;			// 等级
	uint64_t	m_nExp;			// 经验
	uint64_t	m_nGold;		// 金币
	uint64_t	m_nMoney;		// 砖石
	uint64_t	m_nVitality;	// 体力

	std::function<uint64_t(void)>							m_zGetFunc[eAT_Count];
	std::function<void(uint64_t, uint32_t, const void*)>	m_zAddFunc[eAT_Count];
}; 