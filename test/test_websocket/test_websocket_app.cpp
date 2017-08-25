#include "stdafx.h"
#include "test_websocket_app.h"
#include "connection_from_client.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#include "tinyxml2/tinyxml2.h"

using namespace core;

CTestWebsocketApp::CTestWebsocketApp()
	: m_pClientConnectionFactory(nullptr)
{

}

CTestWebsocketApp::~CTestWebsocketApp()
{
}

bool CTestWebsocketApp::onInit()
{
	this->m_pClientConnectionFactory = new CClientConnectionFactory();
	CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory("CConnectionFromClient", this->m_pClientConnectionFactory);

	CBaseApp::Inst()->getBaseConnectionMgr()->listen("0.0.0.0", 8000, false, "CConnectionFromClient", "", 10 * 1024, 10 * 1024, nullptr, eCCT_Websocket);

	PrintInfo("CTestWebsocketApp::onInit");
	return true;
}

void CTestWebsocketApp::onDestroy()
{

}

CTestWebsocketApp* CTestWebsocketApp::Inst()
{
	return reinterpret_cast<CTestWebsocketApp*>(CBaseApp::Inst());
}

int32_t main(int32_t argc, char* argv[])
{
	CTestWebsocketApp* pTestWebsocketApp = new CTestWebsocketApp();
	pTestWebsocketApp->runAndServe(argv[0], "test_websocket_config.xml");
	SAFE_DELETE(pTestWebsocketApp);

	return 0;
}