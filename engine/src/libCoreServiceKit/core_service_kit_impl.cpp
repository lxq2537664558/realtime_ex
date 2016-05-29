#include "stdafx.h"
#include "core_service_kit_impl.h"
#include "core_connection_to_service.h"
#include "core_connection_from_service.h"
#include "core_connection_to_master.h"
#include "message_dispatcher.h"
#include "cluster_invoker.h"
#include "message_registry.h"
#include "core_service_kit_define.h"
#include "core_service_proxy.h"
#include "load_balance_mgr.h"
#include "service_connection_factory.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"

#include "tinyxml2/tinyxml2.h"

namespace core
{

	CCoreServiceKitImpl::CCoreServiceKitImpl()
		: m_nMasterPort(0)
		, m_pTransporter(nullptr)
		, m_pCoreServiceInvoker(nullptr)
		, m_pCoreServiceProxy(nullptr)
		, m_pLoadBalanceMgr(nullptr)
		, m_pServiceConnectionFactory(nullptr)
		, m_pInvokerTrace(nullptr)
	{
		this->m_tickCheckConnectMaster.setCallback(std::bind(&CCoreServiceKitImpl::onCheckConnectMaster, this, std::placeholders::_1));
	
		this->m_sServiceBaseInfo.nWeight = _SERVICE_WEIGHT_PERCENT_MULT;
		this->m_sServiceBaseInfo.nPort = 0;
		this->m_sServiceBaseInfo.nRecvBufSize = 0;
		this->m_sServiceBaseInfo.nSendBufSize = 0;
	}

	CCoreServiceKitImpl::~CCoreServiceKitImpl()
	{
		
	}

	bool CCoreServiceKitImpl::init()
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
			PrintWarning("pServiceInfoXML == nullptr\n");
			return false;
		}

		// 加载服务基本信息
		this->m_sServiceBaseInfo.szType = pServiceInfoXML->Attribute("service_type");
		this->m_sServiceBaseInfo.szName = pServiceInfoXML->Attribute("service_name");
		this->m_sServiceBaseInfo.szGroup = pServiceInfoXML->Attribute("service_group");
		this->m_sServiceBaseInfo.nWeight = (uint32_t)(pServiceInfoXML->FloatAttribute("weight") * _SERVICE_WEIGHT_PERCENT_MULT);
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
		if (!this->m_pCoreServiceProxy->init(pRootXML))
		{
			PrintWarning("this->m_pTransporter->init()");
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

		this->m_pLoadBalanceMgr = new CLoadBalanceMgr();
		if (!this->m_pLoadBalanceMgr->init())
		{
			PrintWarning("this->m_pLoadBalancePolicyMgr->init()");
			return false;
		}

		if (!CClusterInvoker::Inst()->init())
		{
			PrintWarning("CClusterInvoker::Inst()->init()");
			return false;
		}

		if (!CMessageRegistry::Inst()->init())
		{
			PrintWarning("CMessageRegistry::Inst()->init()");
			return false;
		}

		if (!CMessageDispatcher::Inst()->init())
		{
			PrintWarning("CMessageDispatcher::Inst()->init()");
			return false;
		}

		CBaseApp::Inst()->getBaseConnectionMgr()->setConnectRefuseCallback(std::bind(&CCoreServiceKitImpl::onConnectRefuse, this, std::placeholders::_1));

		if (this->m_sServiceBaseInfo.nPort != 0)
		{
			// 在所有网卡上监听
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->listen("0.0.0.0", this->m_sServiceBaseInfo.nPort, eBCT_ConnectionFromService, "", this->m_sServiceBaseInfo.nSendBufSize, this->m_sServiceBaseInfo.nRecvBufSize, nullptr))
				return false;
		}

		this->m_szMasterHost = szMasterHost;
		this->m_nMasterPort = nMasterPort;
		
		if (!this->m_szMasterHost.empty() && this->m_nMasterPort != 0)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, eBCT_ConnectionToMaster, "master", 1024, 1024, nullptr))
				return false;

			CBaseApp::Inst()->registerTicker(&this->m_tickCheckConnectMaster, 5 * 1000, 5 * 1000, 0);
		}

		return true;
	}

	void CCoreServiceKitImpl::release()
	{
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromService, nullptr);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToService, nullptr);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToMaster, nullptr);

		SAFE_DELETE(this->m_pTransporter);
		SAFE_DELETE(this->m_pCoreServiceInvoker);
		SAFE_DELETE(this->m_pCoreServiceProxy);
		SAFE_DELETE(this->m_pLoadBalanceMgr);
		SAFE_DELETE(this->m_pServiceConnectionFactory);

		CMessageDispatcher::Inst()->release();
		CMessageRegistry::Inst()->release();
		CClusterInvoker::Inst()->release();
	}

	void CCoreServiceKitImpl::onCheckConnectMaster(uint64_t nContext)
	{
		if ( !this->m_szMasterHost.empty() && this->m_nMasterPort != 0 && this->getConnectionToMaster() == nullptr)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, eBCT_ConnectionToMaster, "master", 1024, 1024, nullptr))
			{
				PrintWarning("connect master error");
			}
		}
	}

	void CCoreServiceKitImpl::onConnectRefuse(const std::string& szContext)
	{
		this->m_pTransporter->onConnectRefuse(szContext);
	}

	CCoreConnectionToMaster* CCoreServiceKitImpl::getConnectionToMaster() const
	{
		std::vector<CBaseConnection*> vecBaseConnection;
		CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnection(eBCT_ConnectionToMaster, vecBaseConnection);
		if (vecBaseConnection.empty())
			return nullptr;

		return dynamic_cast<CCoreConnectionToMaster*>(vecBaseConnection[0]);
	}

	const SServiceBaseInfo& CCoreServiceKitImpl::getServiceBaseInfo() const
	{
		return this->m_sServiceBaseInfo;
	}

	void CCoreServiceKitImpl::addGlobalBeforeFilter(const ServiceGlobalFilter& callback)
	{
		this->m_vecServiceGlobalBeforeFilter.push_back(callback);
	}

	void CCoreServiceKitImpl::addGlobalAfterFilter(const ServiceGlobalFilter& callback)
	{
		this->m_vecServiceGlobalAfterFilter.push_back(callback);
	}

	const std::vector<ServiceGlobalFilter>& CCoreServiceKitImpl::getGlobalBeforeFilter()
	{
		return this->m_vecServiceGlobalBeforeFilter;
	}

	const std::vector<ServiceGlobalFilter>& CCoreServiceKitImpl::getGlobalAfterFilter()
	{
		return this->m_vecServiceGlobalAfterFilter;
	}

	CTransporter* CCoreServiceKitImpl::getTransporter() const
	{
		return this->m_pTransporter;
	}

	CLoadBalanceMgr* CCoreServiceKitImpl::getLoadBalanceMgr() const
	{
		return this->m_pLoadBalanceMgr;
	}

	CCoreServiceProxy* CCoreServiceKitImpl::getCoreServiceProxy() const
	{
		return this->m_pCoreServiceProxy;
	}

	CCoreServiceInvoker* CCoreServiceKitImpl::getCoreServiceInvoker() const
	{
		return this->m_pCoreServiceInvoker;
	}

	CInvokerTrace* CCoreServiceKitImpl::getInvokerTrace() const
	{
		return this->m_pInvokerTrace;
	}

}