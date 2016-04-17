#include "stdafx.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "core_connection.h"
#include "base_connection.h"
#include "base_app.h"

#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"

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
		if (CBaseApp::Inst()->getBaseConnectionMgr()->m_funConnectRefuse != nullptr)
			CBaseApp::Inst()->getBaseConnectionMgr()->m_funConnectRefuse(szContext);

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

	bool CCoreConnectionMgr::init(uint32_t nMaxSocketCount)
	{
		if (!base::startupNetwork())
			return false;
		
		this->m_pBaseConnectionMgr = new CBaseConnectionMgr();
		this->m_pBaseConnectionMgr->m_pCoreConnectionMgr = this;

		this->m_pNetEventLoop = base::createNetEventLoop();
		return this->m_pNetEventLoop->init(nMaxSocketCount);
	}

	base::INetConnecterHandler* CCoreConnectionMgr::onAccept(SNetAccepterHandler* pNetAccepterHandler, base::INetConnecter* pNetConnecter)
	{
		DebugAstEx(pNetConnecter != nullptr && pNetAccepterHandler != nullptr, nullptr);

		CCoreConnection* pCoreConnection = this->createCoreConnection(pNetAccepterHandler->szContext, pNetAccepterHandler->nClassID, pNetAccepterHandler->clientDataCallback);
		DebugAstEx(nullptr != pCoreConnection, nullptr);

		return pCoreConnection;
	}

	void CCoreConnectionMgr::onConnect(SNetActiveWaitConnecterHandler* pNetActiveWaitConnecterHandler)
	{
		DebugAst(pNetActiveWaitConnecterHandler != nullptr && pNetActiveWaitConnecterHandler->getNetConnecter() != nullptr);

		CCoreConnection* pCoreConnection = this->createCoreConnection(pNetActiveWaitConnecterHandler->szContext, pNetActiveWaitConnecterHandler->nClassID, pNetActiveWaitConnecterHandler->clientDataCallback);
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

	bool CCoreConnectionMgr::connect(const std::string& szHost, uint16_t nPort, const std::string& szContext, uint32_t nClassID, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, ClientDataCallback clientDataCallback)
	{
		PrintInfo("start connect host: %s  port: %u context: %s", szHost.c_str(), nPort, szContext.c_str());
		SNetActiveWaitConnecterHandler* pWaitActiveConnecterHandler = new SNetActiveWaitConnecterHandler();
		pWaitActiveConnecterHandler->szContext = szContext;
		pWaitActiveConnecterHandler->nClassID = nClassID;
		pWaitActiveConnecterHandler->pCoreConnectionMgr = this;
		pWaitActiveConnecterHandler->clientDataCallback = clientDataCallback;

		SNetAddr sNetAddr;
		base::crt::strncpy(sNetAddr.szHost, _countof(sNetAddr.szHost), szHost.c_str(), _TRUNCATE);
		sNetAddr.nPort = nPort;
		if (!this->m_pNetEventLoop->connect(sNetAddr, nSendBufferSize, nRecvBufferSize, pWaitActiveConnecterHandler))
		{
			SAFE_DELETE(pWaitActiveConnecterHandler);
			return false;
		}

		this->m_listActiveNetWaitConnecterHandler.push_back(pWaitActiveConnecterHandler);
		return true;
	}

	bool CCoreConnectionMgr::listen(const std::string& szHost, uint16_t nPort, const std::string& szContext, uint32_t nClassID, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, ClientDataCallback clientDataCallback)
	{
		SNetAccepterHandler* pNetAccepterHandler = new SNetAccepterHandler();
		pNetAccepterHandler->szContext = szContext;
		pNetAccepterHandler->nClassID = nClassID;
		pNetAccepterHandler->pCoreConnectionMgr = this;
		pNetAccepterHandler->clientDataCallback = clientDataCallback;

		SNetAddr sNetAddr;
		base::crt::strncpy(sNetAddr.szHost, _countof(sNetAddr.szHost), szHost.c_str(), _TRUNCATE);
		sNetAddr.nPort = nPort;
		if (!this->m_pNetEventLoop->listen(sNetAddr, nSendBufferSize, nRecvBufferSize, pNetAccepterHandler))
		{
			SAFE_DELETE(pNetAccepterHandler);
			return false;
		}
		this->m_vecNetAccepterHandler.push_back(pNetAccepterHandler);

		return true;
	}

	int32_t CCoreConnectionMgr::update(int32_t nTime)
	{
		return this->m_pNetEventLoop->update(nTime);
	}

	CBaseConnectionMgr* CCoreConnectionMgr::getBaseConnectionMgr() const
	{
		return this->m_pBaseConnectionMgr;
	}

	uint32_t CCoreConnectionMgr::getCoreConnectionCount(uint32_t nClassID) const
	{
		auto iter = this->m_mapCoreConnectionByClassID.find(nClassID);
		if (iter == this->m_mapCoreConnectionByClassID.end())
			return 0;

		return (uint32_t)iter->second.size();
	}

	void CCoreConnectionMgr::getBaseConnection(uint32_t nClassID, std::vector<CBaseConnection*>& vecBaseConnection) const
	{
		auto iter = this->m_mapCoreConnectionByClassID.find(nClassID);
		if (iter == this->m_mapCoreConnectionByClassID.end())
			return;

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
	}

	CCoreConnection* CCoreConnectionMgr::getCoreConnection(uint64_t nID) const
	{
		auto iter = this->m_mapCoreConnectionByID.find(nID);
		if (iter == this->m_mapCoreConnectionByID.end())
			return nullptr;

		return iter->second;
	}

	void CCoreConnectionMgr::broadcast(uint32_t nClassID, uint16_t nMsgType, const void* pData, uint16_t nSize)
	{
		auto iter = this->m_mapCoreConnectionByClassID.find(nClassID);
		if (iter == this->m_mapCoreConnectionByClassID.end())
			return;
		
		std::list<CCoreConnection*>& listCoreConnection = iter->second;
		for (auto iter = listCoreConnection.begin(); iter != listCoreConnection.end(); ++iter)
		{
			CCoreConnection* pCoreConnection = *iter;
			if (nullptr == pCoreConnection)
				continue;

			pCoreConnection->send(nMsgType, pData, nSize);
		}
	}

	CCoreConnection* CCoreConnectionMgr::createCoreConnection(const std::string& szContext, uint32_t nClassID, ClientDataCallback clientDataCallback)
	{
		CBaseConnection* pBaseConnection = dynamic_cast<CBaseConnection*>(CBaseObject::createObject(nClassID));
		if (nullptr == pBaseConnection)
			return nullptr;

		CCoreConnection* pCoreConnection = new CCoreConnection();
		if (!pCoreConnection->init(pBaseConnection, this->m_nNextCoreConnectionID++, clientDataCallback))
		{
			SAFE_DELETE(pCoreConnection);
			CBaseObject::destroyObject(pBaseConnection);

			return nullptr;
		}

		this->m_mapCoreConnectionByID[pCoreConnection->getID()] = pCoreConnection;
		this->m_mapCoreConnectionByClassID[nClassID].push_back(pCoreConnection);
		return pCoreConnection;
	}

	void CCoreConnectionMgr::onDisconnect(CCoreConnection* pCoreConnection)
	{
		DebugAst(pCoreConnection != nullptr);

		CBaseConnection* pBaseConnection = pCoreConnection->m_pBaseConnection;
		DebugAst(pBaseConnection != nullptr);

		auto iter = this->m_mapCoreConnectionByClassID.find(pBaseConnection->getClassID());
		if (iter != this->m_mapCoreConnectionByClassID.end())
			iter->second.remove(pCoreConnection);

		this->m_mapCoreConnectionByID.erase(pCoreConnection->getID());

		CBaseObject::destroyObject(pBaseConnection);

		SAFE_DELETE(pCoreConnection);
	}
}