#include "stdafx.h"
#include "core_service_app_impl.h"
#include "core_connection_to_master.h"
#include "message_dispatcher.h"
#include "cluster_invoker.h"
#include "core_service_kit_define.h"
#include "core_other_node_proxy.h"
#include "node_connection_factory.h"

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
		, m_pCoreMessageRegistry(nullptr)
		, m_pCoreOtherNodeProxy(nullptr)
		, m_pNodeConnectionFactory(nullptr)
		, m_pCoreConnectionToMaster(nullptr)
		, m_pInvokerTrace(nullptr)
		, m_pMessageDispatcher(nullptr)
		, m_pLuaFacade(nullptr)
	{
		this->m_tickCheckConnectMaster.setCallback(std::bind(&CCoreServiceAppImpl::onCheckConnectMaster, this, std::placeholders::_1));
	
		this->m_sNodeBaseInfo.nPort = 0;
		this->m_sNodeBaseInfo.nRecvBufSize = 0;
		this->m_sNodeBaseInfo.nSendBufSize = 0;
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

		tinyxml2::XMLElement* pNodeInfoXML = pRootXML->FirstChildElement("node_info");
		if (pNodeInfoXML == nullptr)
		{
			PrintWarning("pNodeInfoXML == nullptr");
			return false;
		}

		uint32_t nID = pNodeInfoXML->UnsignedAttribute("node_id");
		if (nID > UINT16_MAX)
		{
			PrintWarning("too big node id: %d", nID);
			return false;
		}
		// 加载节点基本信息
		this->m_sNodeBaseInfo.nID = (uint16_t)nID;
		this->m_sNodeBaseInfo.szName = pNodeInfoXML->Attribute("node_name");
		this->m_sNodeBaseInfo.szGroup = pNodeInfoXML->Attribute("node_group");
		this->m_nInvokTimeout = (uint32_t)(pNodeInfoXML->UnsignedAttribute("invoke_timeout"));
		tinyxml2::XMLElement* pHostInfoXML = pNodeInfoXML->FirstChildElement("host_info");
		if (pHostInfoXML != nullptr)
		{
			this->m_sNodeBaseInfo.szHost = pHostInfoXML->Attribute("host");
			this->m_sNodeBaseInfo.nPort = (uint16_t)pHostInfoXML->UnsignedAttribute("port");
			this->m_sNodeBaseInfo.nRecvBufSize = pHostInfoXML->UnsignedAttribute("recv_buf_size");
			this->m_sNodeBaseInfo.nSendBufSize = pHostInfoXML->UnsignedAttribute("send_buf_size");
		}

		this->m_pNodeConnectionFactory = new CNodeConnectionFactory();
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromOtherNode, this->m_pNodeConnectionFactory);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToOtherNode, this->m_pNodeConnectionFactory);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToMaster, this->m_pNodeConnectionFactory);

		this->m_pCoreOtherNodeProxy = new CCoreOtherNodeProxy();
		if (!this->m_pCoreOtherNodeProxy->init())
		{
			PrintWarning("this->m_pCoreOtherNodeProxy->init()");
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

		this->m_pMessageDispatcher = new CMessageDispatcher();
		if (!this->m_pMessageDispatcher->init())
		{
			PrintWarning("this->m_pMessageDispatcher->init()");
			return false;
		}

		this->m_pTransporter = new CTransporter();
		if (!this->m_pTransporter->init())
		{
			PrintWarning("this->m_pTransporter->init()");
			return false;
		}

		this->m_pCoreMessageRegistry = new CCoreMessageRegistry();
		if (!this->m_pCoreMessageRegistry->init())
		{
			PrintWarning("this->m_pCoreMessageRegistry->init()");
			return false;
		}

		if (!CClusterInvoker::Inst()->init())
		{
			PrintWarning("CClusterInvoker::Inst()->init()");
			return false;
		}

		CBaseApp::Inst()->getBaseConnectionMgr()->setConnectRefuseCallback(std::bind(&CCoreServiceAppImpl::onConnectRefuse, this, std::placeholders::_1));

		if (this->m_sNodeBaseInfo.nPort != 0)
		{
			// 在所有网卡上监听
			CBaseApp::Inst()->getBaseConnectionMgr()->listen("0.0.0.0", this->m_sNodeBaseInfo.nPort, eBCT_ConnectionFromOtherNode, "", this->m_sNodeBaseInfo.nSendBufSize, this->m_sNodeBaseInfo.nRecvBufSize);
		}

		this->m_szMasterHost = szMasterHost;
		this->m_nMasterPort = nMasterPort;
		
		if (!this->m_szMasterHost.empty() && this->m_nMasterPort != 0)
		{
			CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, eBCT_ConnectionToMaster, "master", 1024, 1024);
			
			CBaseApp::Inst()->registerTicker(&this->m_tickCheckConnectMaster, 5 * 1000, 5 * 1000, 0);
		}

		this->m_pLuaFacade = new base::CLuaFacade();

		return true;
	}

	void CCoreServiceAppImpl::release()
	{
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionFromOtherNode, nullptr);
		CBaseApp::Inst()->getBaseConnectionMgr()->setBaseConnectionFactory(eBCT_ConnectionToMaster, nullptr);

		SAFE_DELETE(this->m_pTransporter);
		SAFE_DELETE(this->m_pCoreMessageRegistry);
		SAFE_DELETE(this->m_pCoreOtherNodeProxy);
		SAFE_DELETE(this->m_pNodeConnectionFactory);
		SAFE_DELETE(this->m_pInvokerTrace);
		SAFE_DELETE(this->m_pMessageDispatcher);
		SAFE_DELETE(this->m_pScheduler);
		SAFE_DELETE(this->m_pLuaFacade);
	}

	void CCoreServiceAppImpl::run()
	{
		this->m_pScheduler->run();
	}

	void CCoreServiceAppImpl::onCheckConnectMaster(uint64_t nContext)
	{
		if ( !this->m_szMasterHost.empty() && this->m_nMasterPort != 0 && this->getConnectionToMaster() == nullptr)
		{
			CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, eBCT_ConnectionToMaster, "master", 1024, 1024);
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

	const SNodeBaseInfo& CCoreServiceAppImpl::getNodeBaseInfo() const
	{
		return this->m_sNodeBaseInfo;
	}

	void CCoreServiceAppImpl::addGlobalBeforeFilter(const GlobalBeforeFilter& callback)
	{
		this->m_vecGlobalBeforeFilter.push_back(callback);
	}

	void CCoreServiceAppImpl::addGlobalAfterFilter(const GlobalAfterFilter& callback)
	{
		this->m_vecGlobalAfterFilter.push_back(callback);
	}

	const std::vector<GlobalBeforeFilter>& CCoreServiceAppImpl::getGlobalBeforeFilter()
	{
		return this->m_vecGlobalBeforeFilter;
	}

	const std::vector<GlobalAfterFilter>& CCoreServiceAppImpl::getGlobalAfterFilter()
	{
		return this->m_vecGlobalAfterFilter;
	}

	CTransporter* CCoreServiceAppImpl::getTransporter() const
	{
		return this->m_pTransporter;
	}

	CCoreOtherNodeProxy* CCoreServiceAppImpl::getCoreOtherNodeProxy() const
	{
		return this->m_pCoreOtherNodeProxy;
	}

	CCoreMessageRegistry* CCoreServiceAppImpl::getCoreMessageRegistry() const
	{
		return this->m_pCoreMessageRegistry;
	}

	CInvokerTrace* CCoreServiceAppImpl::getInvokerTrace() const
	{
		return this->m_pInvokerTrace;
	}

	CMessageDispatcher* CCoreServiceAppImpl::getMessageDispatcher() const
	{
		return this->m_pMessageDispatcher;
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

	void CCoreServiceAppImpl::setNodeConnectCallback(const std::function<void(uint16_t)>& callback)
	{
		this->m_fnNodeConnectCallback = callback;
	}

	void CCoreServiceAppImpl::setNodeDisconnectCallback(const std::function<void(uint16_t)>& callback)
	{
		this->m_fnNodeDisconnectCallback = callback;
	}

	std::function<void(uint16_t)>& CCoreServiceAppImpl::getNodeConnectCallback()
	{
		return this->m_fnNodeConnectCallback;
	}

	std::function<void(uint16_t)>& CCoreServiceAppImpl::getNodeDisconnectCallback()
	{
		return this->m_fnNodeDisconnectCallback;
	}

}