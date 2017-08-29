#include "uc_service.h"
#include "uc_service_message_handler.h"

#include "libCoreCommon/service_invoker.h"

#include "tinyxml2/tinyxml2.h"

#include "msg_proto_src/u2d_active_count_notify.pb.h"

CUCService::CUCService()
	: m_pUCServiceMessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
	, m_pUserInfoMgr(nullptr)
{

}

CUCService::~CUCService()
{
	SAFE_DELETE(this->m_pNormalProtobufFactory);
	SAFE_DELETE(this->m_pUCServiceMessageHandler);
	SAFE_DELETE(this->m_pUserInfoMgr);
}

core::CProtobufFactory* CUCService::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

CUserInfoMgr* CUCService::getUserInfoMgr() const
{
	return this->m_pUserInfoMgr;
}

void CUCService::release()
{
	delete this;
}

bool CUCService::onInit()
{
	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	this->m_pUCServiceMessageHandler = new CUCServiceMessageHandler(this);
	this->m_pUserInfoMgr = new CUserInfoMgr(this);

	this->setServiceConnectCallback(std::bind(&CUCService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyActiveCount.setCallback(std::bind(&CUCService::onNotifyActiveCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyActiveCount, 5000, 5000, 0);

	PrintInfo("CUCService::onInit");

	return true;
}

void CUCService::onFrame()
{
}

void CUCService::onQuit()
{
	PrintInfo("CUCService::onQuit");
}

void CUCService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		u2d_active_count_notify msg;
		msg.set_count(this->m_pUserInfoMgr->getUserCount());
		this->getServiceInvoker()->send(nServiceID, &msg);
	}
}

void CUCService::onNotifyActiveCount(uint64_t nContext)
{
	u2d_active_count_notify msg;
	msg.set_count(this->m_pUserInfoMgr->getUserCount());
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CUCService();
}