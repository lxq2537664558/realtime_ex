#include "my_actor_factory.h"
#include "test_actor0.h"
#include "test_actor1.h"

CMyActorFactory::CMyActorFactory()
{

}

CMyActorFactory::~CMyActorFactory()
{

}

core::CActorBase* CMyActorFactory::createActor(const std::string& szType)
{
	if (szType == "CTestActor0")
		return new CTestActor0();
	else if (szType == "CTestActor1")
		return new CTestActor1();

	return nullptr;
}
