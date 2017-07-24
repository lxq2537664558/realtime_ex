#include "test_actor2.h"


#include "libCoreCommon/when_all.h"
#include "libCoreCommon/base_app.h"

DEFINE_OBJECT(CTestActor2, 1);

CTestActor2::CTestActor2()
	: m_pServiceBase(nullptr)
{
}

CTestActor2::~CTestActor2()
{

}

void CTestActor2::onInit(const std::string& szContext)
{
	PrintInfo("CTestActor2::onInit");

	uint32_t nServiceID = 0;
	base::crt::atoui(szContext.c_str(), nServiceID);
	this->m_pServiceBase = CBaseApp::Inst()->getServiceBase(nServiceID);
	DebugAst(this->m_pServiceBase != nullptr);
}

void CTestActor2::onDestroy()
{
	PrintInfo("CTestActor2::onDestroy");
}