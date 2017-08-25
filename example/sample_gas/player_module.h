#pragma once
#include "player_module_event.h"

class CPlayer;
class CPlayerModule
{
public:
	CPlayerModule(CPlayer *pPlayer);
	virtual ~CPlayerModule();

	virtual void	onInit() { }										// ��ʼ������
	virtual void	onDestroy() { }										// �ͷ��ڴ�
	virtual void    onLoadData() { }									// �����������
	virtual void	onBackup() { }										// ���ݱ���
	virtual void	onPlayerLogin() { }									// �������[����׼����� ��ʽ������Ϸ]
	virtual void	onPlayerLogout() { }								// �������
	virtual void	onModuleEvent(const IPlayerModuleEvent* pEvent) { }	// ���ģ��ʱ��

	uint64_t		getPlayerID() const;
	CPlayer*		getPlayer() const;

protected:
	CPlayer*	m_pPlayer;
};