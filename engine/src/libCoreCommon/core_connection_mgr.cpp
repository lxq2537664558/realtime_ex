#include "stdafx.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "core_connection.h"
#include "base_connection.h"
#include "base_connection_factory.h"
#include "base_app.h"
#include "core_app.h"

#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>

namespace core
{
	base::INetConnecterHandler* CCoreConnectionMgr::SNetAccepterHandler::onAccept(base::INetConnecter* pNetConnecter)
	{
		return pCoreConnectionMgr->onAccept(this, pNetConnecter);
	}

	void CCoreConnectionMgr::SNetActiveWaitConnecterHandler::onConnect()
	{
		pCoreConnectionMgr->onConnect(this);
	}

	void CCoreConnectionMgr::SNetActiveWaitConnecterHandler::onDisconnect()
	{
		CBaseApp::Inst()->getBaseConnectionMgr()->onConnectRefuse(szContext);

		pCoreConnectionMgr->delActiveWaitConnecterHandler(this);
	}

	CCoreConnectionMgr::CCoreConnectionMgr()
		: m_pNetEventLoop(nullptr)
		, m_pBaseConnectionMgr(nullptr)
		, m_nNextCoreConnectionID(1)
	{
	}

	CCoreConnectionMgr::~CCoreConnectionMgr()
	{
		for (size_t i = 0; i < this->m_vecNetAccepterHandler.size(); ++i)
		{
			SAFE_DELETE(this->m_vecNetAccepterHandler[i]);
		}
		SAFE_DELETE(this->m_pBaseConnectionMgr);
		SAFE_RELEASE(this->m_pNetEventLoop);
	}

	bool CCoreConnectionMgr::init(uint32_t nMaxConnectionCount)
	{
		if (!base::startupNetwork())
			return false;

		this->m_pBaseConnectionMgr = new CBaseConnectionMgr();
		this->m_pBaseConnectionMgr->m_pCoreConnectionMgr = this;

		this->m_pNetEventLoop = base::createNetEventLoop();
		return this->m_pNetEventLoop->init(nMaxConnectionCount);
	}

	base::INetConnecterHandler* CCoreConnectionMgr::onAccept(SNetAccepterHandler* pNetAccepterHandler, base::INetConnecter* pNetConnecter)
	{
		DebugAstEx(pNetConnecter != nullptr && pNetAccepterHandler != nullptr, nullptr);

		CCoreConnection* pCoreConnection = this->createCoreConnection(pNetAccepterHandler->nType, pNetAccepterHandler->szContext);
		DebugAstEx(nullptr != pCoreConnection, nullptr);

		return pCoreConnection;
	}

	void CCoreConnectionMgr::onConnect(SNetActiveWaitConnecterHandler* pNetActiveWaitConnecterHandler)
	{
		DebugAst(pNetActiveWaitConnecterHandler != nullptr && pNetActiveWaitConnecterHandler->getNetConnecter() != nullptr);

		CCoreConnection* pCoreConnection = this->createCoreConnection(pNetActiveWaitConnecterHandler->nType, pNetActiveWaitConnecterHandler->szContext);
		if (nullptr == pCoreConnection)
			return;

		base::INetConnecter* pNetConnecter = pNetActiveWaitConnecterHandler->getNetConnecter();

		pNetConnecter->setHandler(pCoreConnection);
		this->delActiveWaitConnecterHandler(pNetActiveWaitConnecterHandler);

		pCoreConnection->onConnect();
	}

	void CCoreConnectionMgr::delActiveWaitConnecterHandler(SNetActiveWaitConnecterHandler* pWaitActiveConnecterHandler)
	{
		this->m_listActiveNetWaitConnecterHandler.remove(pWaitActiveConnecterHandler);
		SAFE_DELETE(pWaitActiveConnecterHandler);
	}

	bool CCoreConnectionMgr::connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize)
	{
		PrintInfo("start connect host: %s  port: %u type: %u context: %s", szHost.c_str(), nPort, nType, szContext.c_str());
		SNetActiveWaitConnecterHandler* pWaitActiveConnecterHandler = new SNetActiveWaitConnecterHandler();
		pWaitActiveConnecterHandler->szContext = szContext;
		pWaitActiveConnecterHandler->nType = nType;
		pWaitActiveConnecterHandler->pCoreConnectionMgr = this;

		SNetAddr sNetAddr;
		base::crt::strcpy(sNetAddr.szHost, _countof(sNetAddr.szHost), szHost.c_str());
		sNetAddr.nPort = nPort;
		if (!this->m_pNetEventLoop->connect(sNetAddr, nSendBufferSize, nRecvBufferSize, pWaitActiveConnecterHandler))
		{
			SAFE_DELETE(pWaitActiveConnecterHandler);
			return false;
		}

		this->m_listActiveNetWaitConnecterHandler.push_back(pWaitActiveConnecterHandler);
		return true;
	}

	bool CCoreConnectionMgr::listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize)
	{
		SNetAccepterHandler* pNetAccepterHandler = new SNetAccepterHandler();
		pNetAccepterHandler->szContext = szContext;
		pNetAccepterHandler->nType = nType;
		pNetAccepterHandler->pCoreConnectionMgr = this;

		SNetAddr sNetAddr;
		base::crt::strcpy(sNetAddr.szHost, _countof(sNetAddr.szHost), szHost.c_str());
		sNetAddr.nPort = nPort;
		if (!this->m_pNetEventLoop->listen(sNetAddr, nSendBufferSize, nRecvBufferSize, pNetAccepterHandler))
		{
			SAFE_DELETE(pNetAccepterHandler);
			return false;
		}
		this->m_vecNetAccepterHandler.push_back(pNetAccepterHandler);

		return true;
	}

	void CCoreConnectionMgr::update(int64_t nTime)
	{
		this->m_pNetEventLoop->update(nTime);
	}

	CBaseConnectionMgr* CCoreConnectionMgr::getBaseConnectionMgr() const
	{
		return this->m_pBaseConnectionMgr;
	}

	uint32_t CCoreConnectionMgr::getCoreConnectionCount(uint32_t nType) const
	{
		auto iter = this->m_mapCoreConnectionByTypeID.find(nType);
		if (iter == this->m_mapCoreConnectionByTypeID.end())
			return 0;

		return (uint32_t)iter->second.size();
	}

	CCoreConnection* CCoreConnectionMgr::getCoreConnectionByID(uint64_t nID) const
	{
		auto iter = this->m_mapCoreConnectionByID.find(nID);
		if (iter == this->m_mapCoreConnectionByID.end())
			return nullptr;

		return iter->second;
	}

	std::vector<CBaseConnection*> CCoreConnectionMgr::getBaseConnection(uint32_t nType) const
	{
		auto iter = this->m_mapCoreConnectionByTypeID.find(nType);
		if (iter == this->m_mapCoreConnectionByTypeID.end())
			return std::vector<CBaseConnection*>();

		std::vector<CBaseConnection*> vecBaseConnection;
		auto& listCoreConnection = iter->second;
		vecBaseConnection.reserve(listCoreConnection.size());
		for (auto iter = listCoreConnection.begin(); iter != listCoreConnection.end(); ++iter)
		{
			CCoreConnection* pCoreConnection = *iter;
			if (nullptr == pCoreConnection)
				continue;

			CBaseConnection* pBaseConnection = pCoreConnection->getBaseConnection();
			if (nullptr == pBaseConnection)
				continue;

			vecBaseConnection.push_back(pBaseConnection);
		}

		return vecBaseConnection;
	}

	void CCoreConnectionMgr::broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nSize, const std::vector<uint64_t>* vecExcludeID)
	{
		auto iter = this->m_mapCoreConnectionByTypeID.find(nType);
		if (iter == this->m_mapCoreConnectionByTypeID.end())
			return;

		std::list<CCoreConnection*>& listCoreConnection = iter->second;
		for (auto iter = listCoreConnection.begin(); iter != listCoreConnection.end(); ++iter)
		{
			CCoreConnection* pCoreConnection = *iter;
			if (nullptr == pCoreConnection)
				continue;

			if (vecExcludeID != nullptr)
			{
				bool bMatch = false;
				for (size_t i = 0; i < vecExcludeID->size(); ++i)
				{
					if (vecExcludeID->at(i) == pCoreConnection->getID())
					{
						bMatch = true;
						break;
					}
				}
				if (bMatch)
					continue;
			}
			
			pCoreConnection->send(nMessageType, pData, nSize);
		}
	}

	CCoreConnection* CCoreConnectionMgr::createCoreConnection(uint32_t nType, const std::string& szContext)
	{
		CBaseConnectionFactory* pBaseConnectionFactory = this->m_pBaseConnectionMgr->getBaseConnectionFactory(nType);
		if (nullptr == pBaseConnectionFactory)
		{
			PrintWarning("can't find base connection factory type: %d context: %s", nType, szContext.c_str());
			return nullptr;
		}
		CBaseConnection* pBaseConnection = pBaseConnectionFactory->createBaseConnection(nType, szContext);
		if (nullptr == pBaseConnection)
		{
			PrintWarning("create base connection error type: %d context: %s", nType, szContext.c_str());
			return nullptr;
		}

		CCoreConnection* pCoreConnection = new CCoreConnection();
		if (!pCoreConnection->init(pBaseConnection, this->m_nNextCoreConnectionID++, nType))
		{
			SAFE_DELETE(pCoreConnection);
			PrintWarning("init core connection error type: %d context: %s", nType, szContext.c_str());
			return nullptr;
		}

		this->m_mapCoreConnectionByID[pCoreConnection->getID()] = pCoreConnection;
		this->m_mapCoreConnectionByTypeID[nType].push_back(pCoreConnection);

		return pCoreConnection;
	}

	void CCoreConnectionMgr::destroyCoreConnection(CCoreConnection* pCoreConnection)
	{
		DebugAst(pCoreConnection != nullptr);

		auto iterType = this->m_mapCoreConnectionByTypeID.find(pCoreConnection->getType());
		if (iterType != this->m_mapCoreConnectionByTypeID.end())
			iterType->second.remove(pCoreConnection);

		this->m_mapCoreConnectionByID.erase(pCoreConnection->getID());

		CBaseConnection* pBaseConnection = pCoreConnection->m_pBaseConnection;
		SAFE_RELEASE(pBaseConnection);

		pCoreConnection->m_pBaseConnection = nullptr;

		SAFE_DELETE(pCoreConnection);
	}

}