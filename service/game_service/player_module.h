#pragma once
#include "player_module_event.h"
#include "event_handler.h"

class CPlayer;
class CPlayerModule 
	: public IEventHandler
{
public:
	CPlayerModule(CPlayer *pPlayer);
	virtual ~CPlayerModule();

	virtual void		onInit() { }	// ��ʼ������(��ʱ�����ܸ��ͻ���ͨѶ)
	virtual void		onDestroy() { }	// �ͷ��ڴ�
	virtual void		onLoadData() { }	// �����������(��ʱ�����ܸ��ͻ���ͨѶ)
	virtual void		onBackup() { }	// ���ݱ���
	virtual void		onPlayerLogin() { }	// �������(����׼����� ��ʽ������Ϸ����ʱ�ܸ��ͻ���ͨѶ)
	virtual void		onPlayerLogout() { }	// �������
	virtual void		onModuleEvent(const IPlayerModuleEvent* pEvent) { }	// ���ģ���¼�

	virtual uint32_t	getModuleType() const = 0;

	uint64_t			getPlayerID() const;
	CPlayer*			getPlayer() const;

private:
	virtual void		onEvent(const IEvent* pEvent);

protected:
	CPlayer*	m_pPlayer;
};