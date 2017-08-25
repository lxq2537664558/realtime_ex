#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/actor_base.h"
#include "libCoreCommon/ticker.h"

#include "player_module.h"

enum EPlayerModuleType
{
	ePMT_Attribute = 0,

	ePMT_Count,
};

enum EPlayerStatusType
{
	ePST_None,
	ePST_Init,
	ePST_LoadData,
	ePST_Login,
	ePST_Normal,
	ePST_Logout,
	ePST_Destroy,
};

class CPlayer :
	public core::CActorBase
{
public:
	CPlayer();
	virtual ~CPlayer();

	virtual void		onLoadData();
	virtual void		onBackup();
	virtual void		onPlayerLogin();
	virtual void		onPlayerLogout();

	EPlayerStatusType	getStatus() const;
	void				setGateServiceID(uint32_t nGateServiceID);
	uint32_t			getGateServiceID() const;

	void				sendClientMessage(const google::protobuf::Message* pMessage);

private:
	virtual void		onInit(const std::string& szContext);
	virtual void		onDestroy();

	virtual void		release();

	void				onHeartbeat(uint64_t nContext);

private:
	core::CTicker		m_tickHeartbeat;
	EPlayerStatusType	m_eStatus;
	uint32_t			m_nGateServiceID;
	CPlayerModule*		m_zPlayerModule[ePMT_Count];
};