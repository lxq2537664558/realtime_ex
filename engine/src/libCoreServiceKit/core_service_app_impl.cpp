#include "stdafx.h"
#include "core_service_app_impl.h"
#include "core_connection_to_master.h"
#include "message_dispatcher.h"
#include "cluster_invoker.h"
#include "core_service_kit_define.h"
#include "core_service_proxy.h"
#include "service_connection_factory.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"

#include "tinyxml2/tinyxml2.h"

#define _DEFAULT_INVOKE_TIMEOUT 10000
#define _DEFAULT_THROUGHPUT		10

namespace core
{

	CCoreServiceAppImpl::CCoreServiceAppImpl()
		: m_nMasterPort(0)
		, m_nInvokTimeout(_DEFAULT_INVOKE_TIMEOUT)
		, m_nThroughput(_DEFAULT_THROUGHPUT)
		, m_pTransporter(nullptr)
		, m_pScheduler(nullptr)
		, m_pCoreServiceInvoker(nullptr)
		, m_pCoreServiceProxy(nullptr)
		, m_pServiceConnectionFactory(nullptr)
		, m_pCoreConnectionToMaster(nullptr)
		, m_pInvokerTrace(nullptr)
		, m_pLuaFacade(nullptr)
	{
		this->m_tickCheckConnectMaster.setCallback(std::bind(&CCoreServiceAppImpl::onCheckConnectMaster, this, std::placeholders::_1));
	
		this->m_sServiceBaseInfo.nPort = 0;
		this->m_sServiceBaseInfo.nRecvBufSize = 0;
		this->m_sServiceBaseInfo.nSendBufSize = 0;
	}

	CCoreServiceAppImpl::~CCoreServiceAppImpl()
	{
		
	}

	bool CCoreServiceAppImpl::init()
	{
		tinyxml2::XMLDocument* pConfigXML = new tinyxml2::XMLDocument();
		if (pConfigXML->LoadFile(CBaseApp::Inst()->getConfigFileName().c_str()) != tinyxml2::XML_SUCCESS)
		{
			PrintWarning("load etc config error");
			return false;
		}
		tinyxml2::XMLElement* pRootXML = pConfigXML->RootElement();
		if (pRootXML == nullptr)
		{
			PrintWarning("pRootXML == nullptr");
			return false;
		}

		tinyxml2::XMLElement* pServiceInfoXML = pRootXML->FirstChildElement("service_info");
		if (pServiceInfoXML == nullptr)
		{
			PrintWarning("pServiceInfoXML == nullptr");
			return false;
		}

		uint32_t nID = pServiceInfoXML->UnsignedAttribute("service_id");
		if (nID > UINT16_MAX)
		{
			PrintWarning("too big service id: %d", nID);
			return false;
		}
		// 加载服务基本信息
		this->m_sServiceBaseInfo.nID = (uint16_t)nID;
		this->m_sServiceBaseInfo.szType = pServiceInfoXML->Attribute("service_type");
		this->m_sServiceBaseInfo.szName = pServiceInfoXML->Attribute("service_name");
		this->m_sServiceBaseInfo.szGroup = pServiceInfoXML->Attribute("service_group");
		this->m_nInvokTimeout = (uint32_t)(pServiceInfoXML->FloatAttribute("invoke_timeout"));
		tinyxml2::XMLElement* pHostInfoXML = pServiceInfoXML->FirstChildElement("host_info");
		if (pHostInfoXML != nullptr)
		{
			this->m_sServiceBaseInfo.szHost = pHostInfoXML->Attribute("host");
			this->m_sServiceBaseInfo.nPort = (uint16_t)pHostInfoXML->UnsignedAttribute("port");
			this->m_sServiceBaseInfo.nRecvBufSize = pHostInfoXML->UnsignedAttribute("recv_buf_size");
			this->m_sServiceBaseInfo.nSendBufSize = pHostInfoXML->UnsignedAttribute("send_buf_size");
		}

		this->m_pServiceConnectionFactory = new CServiceConnectionFactory();
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, this->m_pServiceConnectionFactory);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToService, this->m_pServiceConnectionFactory);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToMaster, this->m_pServiceConnectionFactory);

		this->m_pCoreServiceProxy = new CCoreServiceProxy();
		if (!this->m_pCoreServiceProxy->init())
		{
			PrintWarning("this->m_pTransporter->init()");
			return false;
		}

		this->m_pScheduler = new CScheduler();
		if (!this->m_pScheduler->init())
		{
			PrintWarning("this->m_pScheduler->init()");
			return false;
		}

		std::string szMasterHost;
		uint16_t nMasterPort = 0;
		tinyxml2::XMLElement* pMasterAddrXML = pRootXML->FirstChildElement("master");
		if (pMasterAddrXML != nullptr)
		{
			szMasterHost = pMasterAddrXML->Attribute("host");
			nMasterPort = (uint16_t)pMasterAddrXML->IntAttribute("port");
		}

		this->m_pInvokerTrace = new CInvokerTrace();
		if (!this->m_pInvokerTrace->init())
		{
			PrintWarning("this->m_pInvokerTrace->init()");
			return false;
		}

		this->m_pTransporter = new CTransporter();
		if (!this->m_pTransporter->init())
		{
			PrintWarning("this->m_pTransporter->init()");
			return false;
		}

		this->m_pCoreServiceInvoker = new CCoreServiceInvoker();
		if (!this->m_pCoreServiceInvoker->init())
		{
			PrintWarning("this->m_pMessageDirectory->init()");
			return false;
		}

		if (!CMessageDispatcher::Inst()->init())
		{
			PrintWarning("CMessageDispatcher::Inst()->init()");
			return false;
		}

		if (!CClusterInvoker::Inst()->init())
		{
			PrintWarning("CClusterInvoker::Inst()->init()");
			return false;
		}

		CBaseApp::Inst()->getBaseConnectionMgr()->setConnectRefuseCallback(std::bind(&CCoreServiceAppImpl::onConnectRefuse, this, std::placeholders::_1));

		if (this->m_sServiceBaseInfo.nPort != 0)
		{
			// 在所有网卡上监听
			CBaseApp::Inst()->getBaseConnectionMgr()->listen("0.0.0.0", this->m_sServiceBaseInfo.nPort, eBCT_ConnectionFromService, "", this->m_sServiceBaseInfo.nSendBufSize, this->m_sServiceBaseInfo.nRecvBufSize, nullptr);
		}

		this->m_szMasterHost = szMasterHost;
		this->m_nMasterPort = nMasterPort;
		
		if (!this->m_szMasterHost.empty() && this->m_nMasterPort != 0)
		{
			CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, eBCT_ConnectionToMaster, "master", 1024, 1024, nullptr);
			
			CBaseApp::Inst()->registerTicker(&this->m_tickCheckConnectMaster, 5 * 1000, 5 * 1000, 0);
		}

		this->m_pLuaFacade = new base::CLuaFacade();

		return true;
	}

	void CCoreServiceAppImpl::release()
	{
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, nullptr);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToMaster, nullptr);

		SAFE_DELETE(this->m_pTransporter);
		SAFE_DELETE(this->m_pCoreServiceInvoker);
		SAFE_DELETE(this->m_pCoreServiceProxy);
		SAFE_DELETE(this->m_pServiceConnectionFactory);
		SAFE_DELETE(this->m_pInvokerTrace);
		SAFE_DELETE(this->m_pScheduler);
		SAFE_DELETE(this->m_pLuaFacade);

		CMessageDispatcher::Inst()->release();
	}

	void CCoreServiceAppImpl::run()
	{
		this->m_pScheduler->run();
	}

	void CCoreServiceAppImpl::onCheckConnectMaster(uint64_t nContext)
	{
		if ( !this->m_szMasterHost.empty() && this->m_nMasterPort != 0 && this->getConnectionToMaster() == nullptr)
		{
			CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, eBCT_ConnectionToMaster, "master", 1024, 1024, nullptr);
		}
	}

	void CCoreServiceAppImpl::onConnectRefuse(const std::string& szContext)
	{
		
	}

	CCoreConnectionToMaster* CCoreServiceAppImpl::getConnectionToMaster() const
	{
		return this->m_pCoreConnectionToMaster;
	}

	void CCoreServiceAppImpl::setCoreConnectionToMaster(CCoreConnectionToMaster* pCoreConnectionToMaster)
	{
		this->m_pCoreConnectionToMaster = pCoreConnectionToMaster;
	}

	const SServiceBaseInfo& CCoreServiceAppImpl::getServiceBaseInfo() const
	{
		return this->m_sServiceBaseInfo;
	}

	void CCoreServiceAppImpl::addGlobalBeforeFilter(const ServiceGlobalFilter& callback)
	{
		this->m_vecGlobalBeforeFilter.push_back(callback);
	}

	void CCoreServiceAppImpl::addGlobalAfterFilter(const ServiceGlobalFilter& callback)
	{
		this->m_vecGlobalAfterFilter.push_back(callback);
	}

	const std::vector<ServiceGlobalFilter>& CCoreServiceAppImpl::getGlobalBeforeFilter()
	{
		return this->m_vecGlobalBeforeFilter;
	}

	const std::vector<ServiceGlobalFilter>& CCoreServiceAppImpl::getGlobalAfterFilter()
	{
		return this->m_vecGlobalAfterFilter;
	}

	CTransporter* CCoreServiceAppImpl::getTransporter() const
	{
		return this->m_pTransporter;
	}

	CCoreServiceProxy* CCoreServiceAppImpl::getCoreServiceProxy() const
	{
		return this->m_pCoreServiceProxy;
	}

	CCoreServiceInvoker* CCoreServiceAppImpl::getCoreServiceInvoker() const
	{
		return this->m_pCoreServiceInvoker;
	}

	CInvokerTrace* CCoreServiceAppImpl::getInvokerTrace() const
	{
		return this->m_pInvokerTrace;
	}

	CScheduler* CCoreServiceAppImpl::getScheduler() const
	{
		return this->m_pScheduler;
	}

	base::CLuaFacade* CCoreServiceAppImpl::getLuaFacade() const
	{
		return this->m_pLuaFacade;
	}

	uint32_t CCoreServiceAppImpl::getInvokeTimeout() const
	{
		return this->m_nInvokTimeout;
	}

	uint32_t CCoreServiceAppImpl::getThroughput() const
	{
		return this->m_nThroughput;
	}

	void CCoreServiceAppImpl::setServiceConnectCallback(std::function<void(uint16_t)> funConnect)
	{
		this->m_serviceConnectCallback = funConnect;
	}

	void CCoreServiceAppImpl::setServiceDisconnectCallback(std::function<void(uint16_t)> funDisconnect)
	{
		this->m_serviceDisconnectCallback = funDisconnect;
	}

	std::function<void(uint16_t)>& CCoreServiceAppImpl::getServiceConnectCallback()
	{
		return this->m_serviceConnectCallback;
	}

	std::function<void(uint16_t)>& CCoreServiceAppImpl::getServiceDisconnectCallback()
	{
		return this->m_serviceDisconnectCallback;
	}

}