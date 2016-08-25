#include "stdafx.h"
#include "base_actor_impl.h"
#include "base_actor.h"
#include "service_base.h"
#include "core_service_app_impl.h"

#include "libCoreCommon/coroutine.h"
#include "libCoreCommon/base_app.h"


#define _DEFAULT_CHANNEL_CAP 256

namespace core
{
	CBaseActorImpl::CBaseActorImpl(uint64_t nID, CBaseActor* pActor)
		: m_channel(_DEFAULT_CHANNEL_CAP)
		, m_nID(nID)
		, m_pBaseActor(pActor)
	{

	}

	CBaseActorImpl::~CBaseActorImpl()
	{

	}

	uint64_t CBaseActorImpl::getID() const
	{
		return this->m_nID;
	}

	void CBaseActorImpl::process()
	{
		if (this->m_channel.empty())
			return;

		for (uint32_t i = 0; i < CCoreServiceAppImpl::Inst()->getThroughput(); ++i)
		{
			SMessagePacket sMessagePacket;
			if (!this->m_channel.recv(sMessagePacket))
				break;

			std::shared_ptr<message_header> pMessage = nullptr;
			if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_REQUEST)
			{
				DebugAst(sMessagePacket.nDataSize > sizeof(request_cookice));

				const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(sMessagePacket.pData);
				// °þµôcookice
				const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceBeginRecv(pCookice->nTraceID, pHeader->nMessageID, pCookice->nFromActorID);

				this->m_sActorSessionInfo.nActorID = pCookice->nFromActorID;
				this->m_sActorSessionInfo.nSessionID = pCookice->nSessionID;

				void* pData = sMessagePacket.pData;
				pMessage = std::shared_ptr<message_header>(const_cast<message_header*>(pHeader), [pData](const void*){ delete[] reinterpret_cast<const char*>(pData); });

				auto iter = s_mapMessageHandlerInfo.find(pMessage->nMessageID);
				if (iter != s_mapMessageHandlerInfo.end())
				{
					std::vector<CBaseActorImpl::SMessageHandlerInfo>& vecMessageHandlerInfo = iter->second;
					for (size_t k = 0; k < vecMessageHandlerInfo.size(); ++k)
					{
						SMessageHandlerInfo& sMessageHandlerInfo = vecMessageHandlerInfo[k];

						if (!sMessageHandlerInfo.bAsync)
						{
							uint64_t nCoroutineID = coroutine::create([&](uint64_t){ sMessageHandlerInfo.handler(this->m_pBaseActor, pCookice->nFromActorID, pMessage); });
							coroutine::resume(nCoroutineID, 0);
						}
						else
						{
							sMessageHandlerInfo.handler(this->m_pBaseActor, pCookice->nFromActorID, pMessage);
						}
					}
				}
				
				this->m_sActorSessionInfo.nActorID = 0;
				this->m_sActorSessionInfo.nSessionID = 0;
				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceEndRecv();
			}
			else if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_RESPONSE)
			{
				const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(sMessagePacket.pData);
				// °þµôcookice
				const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

				SResponseWaitInfo* pResponseWaitInfo = this->getResponseWaitInfo(pCookice->nSessionID, true);
				if (nullptr != pResponseWaitInfo)
				{
					Defer(delete pResponseWaitInfo);

					CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceSend(pResponseWaitInfo->nTraceID, pResponseWaitInfo->nMessageID, pResponseWaitInfo->nToID, pResponseWaitInfo->nBeginTime);
					CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceBeginRecv(pResponseWaitInfo->nTraceID, pHeader->nMessageID, pResponseWaitInfo->nToID);
					if (pResponseWaitInfo->callback != nullptr)
					{
						if (pCookice->nResult == eRRT_OK)
						{
							void* pData = sMessagePacket.pData;
							pMessage = std::shared_ptr<message_header>(const_cast<message_header*>(pHeader), [pData](const void*){ delete[] reinterpret_cast<const char*>(pData); });
							pResponseWaitInfo->callback(pMessage, pCookice->nResult);
						}
						else if (pCookice->nResult != eRRT_OK)
						{
							pResponseWaitInfo->callback(nullptr, pCookice->nResult);
						}
					}
					else if (pResponseWaitInfo->nCoroutineID != 0)
					{
						void* pData = sMessagePacket.pData;
						pMessage = std::shared_ptr<message_header>(const_cast<message_header*>(pHeader), [pData](const void*){ delete[] reinterpret_cast<const char*>(pData); });

						CMessage* pNewMessage = new CMessage();
						*pNewMessage = pMessage;
						coroutine::sendMessage(pResponseWaitInfo->nCoroutineID, pNewMessage);
						coroutine::sendMessage(pResponseWaitInfo->nCoroutineID, reinterpret_cast<void*>(pCookice->nResult));

						coroutine::resume(pResponseWaitInfo->nCoroutineID, 0);
					}
					CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceEndRecv();
				}
			}
			else if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
			{
				const gate_forward_cookice* pCookice = reinterpret_cast<const gate_forward_cookice*>(sMessagePacket.pData);
				// °þµôcookice
				const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceBeginRecv(pCookice->nTraceID, pHeader->nMessageID, 0);
				SClientSessionInfo session((uint16_t)sMessagePacket.nID, pCookice->nSessionID);
				void* pData = sMessagePacket.pData;
				pMessage = std::shared_ptr<message_header>(const_cast<message_header*>(pHeader), [pData](const void*){ delete[] reinterpret_cast<const char*>(pData); });

				auto iter = s_mapForwardHandlerInfo.find(pMessage->nMessageID);
				if (iter != s_mapForwardHandlerInfo.end())
				{
					std::vector<SForwardHandlerInfo>& vecForwardHandlerInfo = iter->second;
					for (size_t k = 0; k < vecForwardHandlerInfo.size(); ++k)
					{
						SForwardHandlerInfo& sForwardHandlerInfo = vecForwardHandlerInfo[k];

						if (!sForwardHandlerInfo.bAsync)
						{
							uint64_t nCoroutineID = coroutine::create([&](uint64_t){ sForwardHandlerInfo.handler(this->m_pBaseActor, session, pMessage); });
							coroutine::resume(nCoroutineID, 0);
						}
						else
						{
							sForwardHandlerInfo.handler(this->m_pBaseActor, session, pMessage);
						}
					}
				}

				CCoreServiceAppImpl::Inst()->getInvokerTrace()->traceEndRecv();
			}
		}

		if (!this->m_channel.empty())
			CCoreServiceAppImpl::Inst()->getScheduler()->addWorkBaseActor(this);
	}

	CChannel* CBaseActorImpl::getChannel()
	{
		return &this->m_channel;
	}

	core::SActorSessionInfo CBaseActorImpl::getActorSessionInfo() const
	{
		return this->m_sActorSessionInfo;
	}

	void CBaseActorImpl::onRequestMessageTimeout(uint64_t nContext)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nContext);
		if (iter == this->m_mapResponseWaitInfo.end())
		{
			PrintWarning("iter == this->m_mapResponseWaitInfo.end() session_id: "UINT64FMT, nContext);
			return;
		}

		SResponseWaitInfo* pResponseWaitInfo = iter->second;
		if (nullptr == pResponseWaitInfo)
		{
			PrintWarning("nullptr == pResponseInfo session_id: "UINT64FMT, nContext);
			return;
		}
		
		core::response_cookice* pCookice = new core::response_cookice();
		pCookice->nActorID = this->getID();
		pCookice->nSessionID = pResponseWaitInfo->nSessionID;
		pCookice->nResult = eRRT_TIME_OUT;

		core::SMessagePacket sMessagePacket;
		sMessagePacket.nID = 0;
		sMessagePacket.nType = eMT_RESPONSE;
		sMessagePacket.nDataSize = sizeof(core::response_cookice);
		sMessagePacket.pData = pCookice;
		this->m_channel.send(sMessagePacket);
	}

	SResponseWaitInfo* CBaseActorImpl::addResponseWaitInfo(uint64_t nSessionID, uint64_t nCoroutineID)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		DebugAstEx(iter == this->m_mapResponseWaitInfo.end(), nullptr);

		SResponseWaitInfo* pResponseWaitInfo = new SResponseWaitInfo();
		pResponseWaitInfo->callback = nullptr;
		pResponseWaitInfo->nSessionID = nSessionID;
		pResponseWaitInfo->nCoroutineID = nCoroutineID;
		pResponseWaitInfo->nTraceID = 0;
		pResponseWaitInfo->nToID = 0;
		pResponseWaitInfo->nMessageID = 0;
		pResponseWaitInfo->nBeginTime = 0;
		pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CBaseActorImpl::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pResponseWaitInfo->tickTimeout, CCoreServiceAppImpl::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;

		return pResponseWaitInfo;
	}

	SResponseWaitInfo* CBaseActorImpl::getResponseWaitInfo(uint64_t nSessionID, bool bErase)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		if (iter == this->m_mapResponseWaitInfo.end())
			return nullptr;

		SResponseWaitInfo* pResponseWaitInfo = iter->second;
		if (bErase)
			this->m_mapResponseWaitInfo.erase(iter);

		return pResponseWaitInfo;
	}

	void CBaseActorImpl::registerMessageHandler(uint16_t nMessageID, const std::function<void(CBaseActor*, uint64_t, CMessage)>& handler, bool bAsync)
	{
		SMessageHandlerInfo sMessageHandlerInfo;
		sMessageHandlerInfo.handler = handler;
		sMessageHandlerInfo.bAsync = bAsync;

		s_mapMessageHandlerInfo[nMessageID].push_back(sMessageHandlerInfo);
	}

	void CBaseActorImpl::registerForwardHandler(uint16_t nMessageID, const std::function<void(CBaseActor*, SClientSessionInfo, CMessage)>& handler, bool bAsync)
	{
		SForwardHandlerInfo sForwardHandlerInfo;
		sForwardHandlerInfo.handler = handler;
		sForwardHandlerInfo.bAsync = bAsync;

		s_mapForwardHandlerInfo[nMessageID].push_back(sForwardHandlerInfo);
	}

	std::map<uint16_t, std::vector<CBaseActorImpl::SForwardHandlerInfo>> CBaseActorImpl::s_mapForwardHandlerInfo;

	std::map<uint16_t, std::vector<CBaseActorImpl::SMessageHandlerInfo>> CBaseActorImpl::s_mapMessageHandlerInfo;
}