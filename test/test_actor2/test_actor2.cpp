#include "test_actor2.h"


#include "libCoreCommon/when_all.h"
#include "libCoreCommon/base_app.h"

CTestActor2::CTestActor2()
{
}

CTestActor2::~CTestActor2()
{

}

void CTestActor2::onInit(const std::string& szContext)
{
	PrintInfo("CTestActor2::onInit");
}

void CTestActor2::onDestroy()
{
	PrintInfo("CTestActor2::onDestroy");
}

void CTestActor2::release()
{
	delete this;
}
