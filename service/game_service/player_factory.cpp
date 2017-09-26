#include "player_factory.h"
#include "player.h"

CPlayerFactory::CPlayerFactory()
{

}

CPlayerFactory::~CPlayerFactory()
{

}

core::CActorBase* CPlayerFactory::createActor(const std::string& szType)
{
	return new CPlayer();
}
