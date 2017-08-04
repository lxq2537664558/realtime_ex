#include "my_actor_factory.h"
#include "test_actor2.h"

CMyActorFactory::CMyActorFactory()
{

}

CMyActorFactory::~CMyActorFactory()
{

}

core::CActorBase* CMyActorFactory::createActor(const std::string& szType)
{
	if (szType == "CTestActor2")
		return new CTestActor2();
	
	return nullptr;
}
