#include "dispatch_service.h"
#include "dispatch_service_message_handler.h"

CDispatchService::CDispatchService()
	: m_pDispatchServiceMessageHandler(nullptr)
	, m_pDefaultProtobufFactory(nullptr)
	, m_pOnlineCountMgr(nullptr)
{

}

CDispatchService::~CDispatchService()
{

}

core::CProtobufFactory* CDispatchService::getProtobufFactory() const
{
	return this->m_pDefaultProtobufFactory;
}

void CDispatchService::release()
{
	delete this;
}

COnlineCountMgr* CDispatchService::getOnlineCountMgr() const
{
	return this->m_pOnlineCountMgr;
}

bool CDispatchService::onInit()
{
	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	this->m_pDispatchServiceMessageHandler = new CDispatchServiceMessageHandler(this);
	this->m_pOnlineCountMgr = new COnlineCountMgr();

	PrintInfo("CDispatchService::onInit");

	return true;
}

void CDispatchService::onFrame()
{
	//PrintInfo("CDispatchService::onFrame");
}

void CDispatchService::onQuit()
{
	PrintInfo("CDispatchService::onQuit");
}

extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CDispatchService();
}