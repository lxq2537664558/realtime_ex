#include "player.h"

#include "libCoreCommon/base_app.h"

DEFINE_OBJECT(CPlayer, 100);

CPlayer::CPlayer()
{
}

CPlayer::~CPlayer()
{

}

void CPlayer::setName(const std::string& szName)
{
	this->m_szName = szName;
}

const std::string& CPlayer::getName() const
{
	return this->m_szName;
}

void CPlayer::onInit(const std::string& szContext)
{
	PrintInfo("CPlayer::onInit");

	
}

void CPlayer::onDestroy()
{
	PrintInfo("CPlayer::onDestroy");
}