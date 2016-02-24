#include "stdafx.h"
#include "service_mgr.h"
#include "connection_from_service.h"
#include "master_app.h"

#include "libCoreCommon\base_connection_mgr.h"

CServiceMgr::CServiceMgr()
{

}

CServiceMgr::~CServiceMgr()
{

}

bool CServiceMgr::init()
{
	return true;
}

CConnectionFromService* CServiceMgr::getServiceConnection(const std::string& szName) const
{
	auto iter = this->m_mapServiceConnection.find(szName);
	if (iter == this->m_mapServiceConnection.end())
		return nullptr;

	return iter->second;
}

void CServiceMgr::addServiceConnection(CConnectionFromService* pServiceConnection)
{
	DebugAst(pServiceConnection != nullptr);

	auto iter = this->m_mapServiceConnection.find(pServiceConnection->getServiceBaseInfo().szName);
	DebugAst(iter == this->m_mapServiceConnection.end());

	this->m_mapServiceConnection[pServiceConnection->getServiceBaseInfo().szName] = pServiceConnection;
}

void CServiceMgr::delServiceConnection(const std::string& szName)
{
	auto iter = this->m_mapServiceConnection.find(szName);
	if (iter == this->m_mapServiceConnection.end())
		return;

	this->m_mapServiceConnection.erase(iter);
}