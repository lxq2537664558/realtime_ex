#include "test_actor0.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"

#include "libCoreCommon/when_all.h"
#include "libCoreCommon/base_app.h"

CTestActor0::CTestActor0()
{
}

CTestActor0::~CTestActor0()
{

}

void CTestActor0::onInit(const std::string& szContext)
{
	PrintInfo("CTestActor0::onInit");
}

void CTestActor0::onDestroy()
{
	PrintInfo("CTestActor0::onDestroy");
}

void CTestActor0::release()
{
	delete this;
}
