#include "user_info.h"
#include "uc_service.h"

#include "libBaseCommon/logger.h"

CUserInfo::CUserInfo(CUCService* pUCService, uint64_t nPlayerID, uint32_t nGasID)
	: m_pUCService(pUCService)
	, m_nPlayerID(nPlayerID)
	, m_nGasID(nGasID)
{
	this->m_tickerTimeout.setCallback(std::bind(&CUserInfo::onTimeout, this, std::placeholders::_1));
}

CUserInfo::~CUserInfo()
{

}

uint64_t CUserInfo::getPlayerID() const
{
	return this->m_nPlayerID;
}

uint32_t CUserInfo::getGasID() const
{
	return this->m_nGasID;
}

void CUserInfo::setGasID(uint32_t nGasID)
{
	this->m_nGasID = nGasID;
}

void CUserInfo::onHeartbeat()
{
	if (this->m_tickerTimeout.isRegister())
	{
		this->m_pUCService->unregisterTicker(&this->m_tickerTimeout);

		PrintInfo("CUserInfo::onHeartbeat player_id: {} gas_id: {}", this->m_nPlayerID, this->m_nGasID);
	}
}

void CUserInfo::startTimout()
{
	if (this->m_tickerTimeout.isRegister())
		this->m_pUCService->unregisterTicker(&this->m_tickerTimeout);

	this->m_pUCService->registerTicker(&this->m_tickerTimeout, 5000, 5000, 0);

	PrintInfo("CUserInfo::startTimout player_id: {} gas_id: {}", this->m_nPlayerID, this->m_nGasID);
}

void CUserInfo::onTimeout(uint64_t nContext)
{
	PrintInfo("CUserInfo::onTimeout player_id: {} gas_id: {}", this->m_nPlayerID, this->m_nGasID);

	this->m_pUCService->getUserInfoMgr()->destroyUserInfo(this->m_nPlayerID);
}