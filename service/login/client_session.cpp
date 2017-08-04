#include "stdafx.h"
#include "client_session.h"

CClientSession::CClientSession()
	: m_nSessionID(0)
	, m_nSocketID(0)
	, m_nServiceID(0)
	, m_nPlayerID(0)
	, m_eState(eCSS_None)
{

}

CClientSession::~CClientSession()
{

}

bool CClientSession::init(uint64_t nSocketID, uint64_t nSessionID, uint32_t nServiceID, uint64_t nPlayerID, const std::string& szToken)
{
	this->m_nSocketID = nSocketID;
	this->m_nSessionID = nSessionID;
	this->m_nPlayerID = nPlayerID;
	this->m_nServiceID = nServiceID;
	this->m_szToken = szToken;
	this->m_eState = eCSS_WaitEnter;

	return true;
}

uint64_t CClientSession::getPlayerID() const
{
	return this->m_nPlayerID;
}

uint32_t CClientSession::getServiceID() const
{
	return this->m_nServiceID;
}

EClientSessionState CClientSession::getState() const
{
	return this->m_eState;
}

void CClientSession::setState(EClientSessionState eState)
{
	this->m_eState = eState;
}

uint64_t CClientSession::getSessionID() const
{
	return this->m_nSessionID;
}

uint64_t CClientSession::getSocketID() const
{
	return this->m_nSocketID;
}

const std::string& CClientSession::getToken() const
{
	return this->m_szToken;
}
