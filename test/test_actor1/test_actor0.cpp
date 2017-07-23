#include "test_actor0.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"

#include "libCoreCommon/when_all.h"
#include "libCoreCommon/base_app.h"

DEFINE_OBJECT(CTestActor0, 1);

CTestActor0::CTestActor0()
	: m_pServiceBase(nullptr)
{
}

CTestActor0::~CTestActor0()
{

}

void CTestActor0::onInit(const std::string& szContext)
{
	PrintInfo("CTestActor0::onInit");

	uint32_t nServiceID = 0;
	base::crt::atoui(szContext.c_str(), nServiceID);
	this->m_pServiceBase = CBaseApp::Inst()->getServiceBase(nServiceID);
	DebugAst(this->m_pServiceBase != nullptr);
}

void CTestActor0::onDestroy()
{
	PrintInfo("CTestActor0::onDestroy");
}