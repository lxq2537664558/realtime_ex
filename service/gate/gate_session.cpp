#include "stdafx.h"
#include "gate_session.h"

CGateSession::CGateSession(uint64_t nSessionID)
	: m_nSessionID(nSessionID)
	, m_nSocketID(0)
{

}

CGateSession::~CGateSession()
{

}

uint64_t CGateSession::getSessionID() const
{
	return this->m_nSessionID;
}

uint64_t CGateSession::getSocketID() const
{
	return this->m_nSocketID;
}