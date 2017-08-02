#include "stdafx.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "core_connection.h"
#include "base_connection.h"
#include "base_connection_factory.h"
#include "base_app.h"
#include "message_command.h"
#include "logic_runnable.h"
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
		DebugAst(!"SNetActiveWaitConnecterHandler::onDisconnect");
	}

	void CCoreConnectionMgr::SNetActiveWaitConnecterHandler::onConnectFail()
	{
		SMCT_NOTIFY_SOCKET_CONNECT_FAIL* pContext = new SMCT_NOTIFY_SOCKET_CONNECT_FAIL();
		pContext->szContext = this->szContext;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_NOTIFY_SOCKET_CONNECT_FAIL;
		sMessagePacket.pData = pContext;
		sMessagePacket.nDataSize = sizeof(SMCT_NOTIFY_SOCKET_CONNECT_FAIL);

		CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);

		pCoreConnectionMgr->delActiveWaitConnecterHandler(this);
	}

	CCoreConnectionMgr::CCoreConnectionMgr()
		: m_pNetEventLoop(nullptr)
		, m_nNextCoreConnectionID(1)
	{
	}

	CCoreConnectionMgr::~CCoreConnectionMgr()
	{
		for (size_t i = 0; i < this->m_vecNetAccepterHandler.size(); ++i)
		{
			SAFE_DELETE(this->m_vecNetAccepterHandler[i]);
		}
		SAFE_RELEASE(this->m_pNetEventLoop);
	}

	bool CCoreConnectionMgr::init(uint32_t nMaxConnectionCount)
	{
		if (!base::startupNetwork())
			return false;

		this->m_pNetEventLoop = base::createNetEventLoop();
		return this->m_pNetEventLoop->init(nMaxConnectionCount);
	}

	base::INetConnecterHandler* CCoreConnectionMgr::onAccept(SNetAccepterHandler* pNetAccepterHandler, base::INetConnecter* pNetConnecter)
	{
		DebugAstEx(pNetConnecter != nullptr && pNetAccepterHandler != nullptr, nullptr);

		CCoreConnection* pCoreConnection = this->createCoreConnection(pNetAccepterHandler->nType, pNetAccepterHandler->szContext, pNetAccepterHandler->messageParser);
		DebugAstEx(nullptr != pCoreConnection, nullptr);

		return pCoreConnection;
	}

	void CCoreConnectionMgr::onConnect(SNetActiveWaitConnecterHandler* pNetActiveWaitConnecterHandler)
	{
		DebugAst(pNetActiveWaitConnecterHandler != nullptr && pNetActiveWaitConnecterHandler->getNetConnecter() != nullptr);

		CCoreConnection* pCoreConnection = this->createCoreConnection(pNetActiveWaitConnecterHandler->nType, pNetActiveWaitConnecterHandler->szContext, pNetActiveWaitConnecterHandler->messageParser);
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

	bool CCoreConnectionMgr::connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser)
	{
		PrintInfo("start connect host: %s  port: %u type: %u context: %s", szHost.c_str(), nPort, nType, szContext.c_str());
		SNetActiveWaitConnecterHandler* pWaitActiveConnecterHandler = new SNetActiveWaitConnecterHandler();
		pWaitActiveConnecterHandler->szContext = szContext;
		pWaitActiveConnecterHandler->nType = nType;
		pWaitActiveConnecterHandler->pCoreConnectionMgr = this;
		pWaitActiveConnecterHandler->messageParser = messageParser;

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

	bool CCoreConnectionMgr::listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser)
	{
		SNetAccepterHandler* pNetAccepterHandler = new SNetAccepterHandler();
		pNetAccepterHandler->szContext = szContext;
		pNetAccepterHandler->nType = nType;
		pNetAccepterHandler->pCoreConnectionMgr = this;
		pNetAccepterHandler->messageParser = messageParser;

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

	void CCoreConnectionMgr::update(uint32_t nTime)
	{
		this->m_pNetEventLoop->update(nTime);
	}

	uint32_t CCoreConnectionMgr::getCoreConnectionCount(uint32_t nType) const
	{
		auto iter = this->m_mapCoreConnectionByTypeID.find(nType);
		if (iter == this->m_mapCoreConnectionByTypeID.end())
			return 0;

		return (uint32_t)iter->second.size();
	}

	CCoreConnection* CCoreConnectionMgr::getCoreConnectionBySocketID(uint64_t nID) const
	{
		auto iter = this->m_mapCoreConnectionByID.find(nID);
		if (iter == this->m_mapCoreConnectionByID.end())
			return nullptr;

		return iter->second;
	}

	void CCoreConnectionMgr::broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nSize, uint64_t* pExcludeID, uint16_t nExcludeIDCount)
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

			if (pExcludeID != nullptr)
			{
				bool bMatch = false;
				for (size_t i = 0; i < nExcludeIDCount; ++i)
				{
					if (pExcludeID[i] == pCoreConnection->getID())
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

	CCoreConnection* CCoreConnectionMgr::createCoreConnection(uint32_t nType, const std::string& szContext, const MessageParser& messageParser)
	{
		CCoreConnection* pCoreConnection = new CCoreConnection();
		if (!pCoreConnection->init(nType, this->m_nNextCoreConnectionID++, szContext, messageParser))
		{
			SAFE_DELETE(pCoreConnection);
			PrintWarning("init core connection error type: %d context: %s", nType, szContext.c_str());
			return nullptr;
		}

		this->m_mapCoreConnectionByID[pCoreConnection->getID()] = pCoreConnection;
		this->m_mapCoreConnectionByTypeID[nType].push_back(pCoreConnection);

		return pCoreConnection;
	}

	void CCoreConnectionMgr::destroyCoreConnection(uint64_t nSocketID)
	{
		auto iter = this->m_mapCoreConnectionByID.find(nSocketID);
		if (iter == this->m_mapCoreConnectionByID.end())
			return;

		CCoreConnection* pCoreConnection = iter->second;
		if (nullptr == pCoreConnection)
		{
			PrintWarning("destroy core connection error socket_id: "UINT64FMT, nSocketID);
			this->m_mapCoreConnectionByID.erase(iter);
			return;
		}
		auto iterType = this->m_mapCoreConnectionByTypeID.find(pCoreConnection->getType());
		if (iterType != this->m_mapCoreConnectionByTypeID.end())
			iterType->second.remove(pCoreConnection);

		this->m_mapCoreConnectionByID.erase(pCoreConnection->getID());

		SAFE_DELETE(pCoreConnection);
	}

	void CCoreConnectionMgr::wakeup()
	{
		this->m_pNetEventLoop->wakeup();
	}
}