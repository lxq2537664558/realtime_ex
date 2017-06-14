#include "stdafx.h"
#include "core_other_service_proxy.h"
#include "core_service_app_impl.h"
#include "native_serialize_adapter.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#define _CHECK_CONNECT_TIME 5000

namespace core
{

	CCoreOtherServiceProxy::CCoreOtherServiceProxy()
		: m_pDefaultSerializeAdapter(new CNativeSerializeAdapter())
	{

	}

	CCoreOtherServiceProxy::~CCoreOtherServiceProxy()
	{
		SAFE_DELETE(this->m_pDefaultSerializeAdapter);
	}

	bool CCoreOtherServiceProxy::init()
	{
		return true;
	}

	void CCoreOtherServiceProxy::addServiceBaseInfo(const SServiceBaseInfo& sServiceBaseInfo)
	{
		auto iter = this->m_mapServiceInfo.find(sServiceBaseInfo.nID);
		if (iter != this->m_mapServiceInfo.end())
			return;

		DebugAst(this->m_mapServiceName.find(sServiceBaseInfo.szName) == this->m_mapServiceName.end());

		SServiceInfo& sNodeInfo = this->m_mapServiceInfo[sServiceBaseInfo.nID];
		sNodeInfo.pCoreConnectionOtherService = nullptr;
		sNodeInfo.sServiceBaseInfo = sServiceBaseInfo;
		
		this->m_mapServiceName[sServiceBaseInfo.szName] = sServiceBaseInfo.nID;

		PrintInfo("add proxy service service_id: %d service_name: %s", sServiceBaseInfo.nID, sServiceBaseInfo.szName.c_str());
	}
	
	void CCoreOtherServiceProxy::delServiceBaseInfo(uint16_t nID, bool bForce)
	{
		auto iter = this->m_mapServiceInfo.find(nID);
		if (iter == this->m_mapServiceInfo.end())
			return;

		SServiceInfo& sNodeInfo = iter->second;

		std::string szName = sNodeInfo.sServiceBaseInfo.szName;
		// 考虑网络只是暂时不可用的情况
		if (!bForce && sNodeInfo.pCoreConnectionOtherService != nullptr)
			return;

		if (sNodeInfo.pCoreConnectionOtherService != nullptr)
			sNodeInfo.pCoreConnectionOtherService->shutdown(base::eNCCT_Force, "del node");

		this->m_mapServiceName.erase(szName);

		this->m_mapServiceInfo.erase(iter);
		PrintInfo("del proxy service service_id: %d service_name: %s", nID, szName.c_str());
	}

	uint16_t CCoreOtherServiceProxy::getServiceID(const std::string& szName) const
	{
		auto iter = this->m_mapServiceName.find(szName);
		if (iter == this->m_mapServiceName.end())
			return 0;

		return iter->second;
	}

	const SServiceBaseInfo* CCoreOtherServiceProxy::getServiceBaseInfo(uint16_t nID) const
	{
		auto iter = this->m_mapServiceInfo.find(nID);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;
		
		return &iter->second.sServiceBaseInfo;
	}

	void CCoreOtherServiceProxy::delCoreConnectionOtherService(uint16_t nID)
	{
		auto iter = this->m_mapServiceInfo.find(nID);
		if (iter == this->m_mapServiceInfo.end())
			return;

		iter->second.pCoreConnectionOtherService = nullptr;
	}

	bool CCoreOtherServiceProxy::addCoreConnectionOtherService(uint16_t nID, CCoreConnectionOtherService* pCoreConnectionOtherService)
	{
		DebugAstEx(pCoreConnectionOtherService != nullptr, false);

		auto iter = this->m_mapServiceInfo.find(nID);
		if (iter == this->m_mapServiceInfo.end())
		{
			PrintWarning("unknwon node service_id: %d remote_addr: %s %d", nID, pCoreConnectionOtherService->getRemoteAddr().szHost, pCoreConnectionOtherService->getRemoteAddr().nPort);
			return false;
		}

		DebugAstEx(iter->second.pCoreConnectionOtherService == nullptr, false);

		iter->second.pCoreConnectionOtherService = pCoreConnectionOtherService;

		return true;
	}

	CCoreConnectionOtherService* CCoreOtherServiceProxy::getCoreConnectionOtherService(uint16_t nID) const
	{
		auto iter = this->m_mapServiceInfo.find(nID);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;

		return iter->second.pCoreConnectionOtherService;
	}

	void CCoreOtherServiceProxy::delCoreConnectionOtherService(uint16_t nID)
	{
		auto iter = this->m_mapServiceInfo.find(nID);
		if (iter == this->m_mapServiceInfo.end())
			return;

		iter->second.pCoreConnectionOtherService = nullptr;
	}

	void CCoreOtherServiceProxy::setSerializeAdapter(uint16_t nID, CSerializeAdapter* pSerializeAdapter)
	{
		DebugAst(pSerializeAdapter != nullptr);

		this->m_mapSerializeAdapter[nID] = pSerializeAdapter;
	}

	CSerializeAdapter* CCoreOtherServiceProxy::getSerializeAdapter(uint16_t nID) const
	{
		auto iter = this->m_mapSerializeAdapter.find(nID);
		if (iter == this->m_mapSerializeAdapter.end())
			return this->m_pDefaultSerializeAdapter;

		return iter->second;
	}

}