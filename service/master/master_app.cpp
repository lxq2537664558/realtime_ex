#include "stdafx.h"
#include "master_app.h"
#include "connection_from_service.h"


#include "libCoreCommon/base_connection_mgr.h"

CMasterApp::CMasterApp()
	: m_pServiceMgr(nullptr)
{

}

CMasterApp::~CMasterApp()
{

}

CMasterApp* CMasterApp::Inst()
{
	return static_cast<CMasterApp*>(CBaseApp::Inst());
}

bool CMasterApp::onInit()
{
	CConnectionFromService::registClassInfo();
	
	this->m_pServiceMgr = new CServiceMgr();
	if (!this->m_pServiceMgr->init())
		return false;

	this->getBaseConnectionMgr()->listen(this->getServiceBaseInfo().szHost, this->getServiceBaseInfo().nPort, "", _GET_CLASS_NAME(CConnectionFromService), this->getServiceBaseInfo().nSendBufSize, this->getServiceBaseInfo().nRecvBufSize * 1024, nullptr);

	return true;
}

void CMasterApp::onDestroy()
{
	SAFE_DELETE(this->m_pServiceMgr);
}

void CMasterApp::onQuit()
{
	this->doQuit();
}

CServiceMgr* CMasterApp::getServiceMgr() const
{
	return this->m_pServiceMgr;
}

int32_t main(int32_t argc, char* argv[])
{
	CMasterApp* pMasterApp = new CMasterApp();
	pMasterApp->run(false, argc, argv, "master_config.xml");

	return 0;
}