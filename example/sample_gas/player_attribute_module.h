#pragma once

#include "player_attribute_module_interface.h"
#include "player_module.h"

#include "libCoreCommon/db_service_invoke_holder.h"

class CPlayerAttributeModule
	: public IPlayerAttributeModule
	, public CPlayerModule
	, public core::CDbServiceInvokeHolder
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

	virtual const std::string&	getName() const;
	virtual uint32_t			getLv() const;
	virtual uint64_t			getGold() const;
	virtual uint64_t			getMoney() const;
	virtual uint64_t			getVitality() const;

private:
	uint64_t	m_nLastLoginTime;
	uint64_t	m_nLastLogoutTime;
	std::string	m_szAccountName;
	uint32_t	m_nServerID;
	std::string m_szName;		// �������
	uint32_t	m_nLv;			// �ȼ�
	uint64_t	m_nGold;		// ���
	uint64_t	m_nMoney;		// שʯ
	uint64_t	m_nVitality;	// ����
};