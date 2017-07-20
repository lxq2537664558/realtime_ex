#include "stdafx.h"
#include "gate_session.h"

CGateSession::CGateSession()
	: m_nSessionID(0)
	, m_nSocketID(0)
{

}

CGateSession::~CGateSession()
{

}

bool CGateSession::init(uint64_t nSocketID, uint64_t nSessionID)
{
	this->m_nSocketID = nSocketID;
	this->m_nSessionID = nSessionID;

	return true;
}

uint64_t CGateSession::getSessionID() const
{
	return this->m_nSessionID;
}

uint64_t CGateSession::getSocketID() const
{
	return this->m_nSocketID;
}
