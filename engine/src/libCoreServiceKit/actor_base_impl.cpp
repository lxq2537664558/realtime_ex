#include "stdafx.h"
#include "actor_base_impl.h"
#include "actor_base.h"
#include "service_base.h"
#include "coroutine.h"
#include "core_service_app_impl.h"


#include "libCoreCommon/base_app.h"


#define _DEFAULT_CHANNEL_CAP 256

namespace core
{
	CActorBaseImpl::CActorBaseImpl(uint64_t nID, CActorBase* pActorBase)
		: m_channel(_DEFAULT_CHANNEL_CAP)
		, m_nID(nID)
		, m_pActorBase(pActorBase)
	{

	}

	CActorBaseImpl::~CActorBaseImpl()
	{

	}

	uint64_t CActorBaseImpl::getID() const
	{
		return this->m_nID;
	}


	CActorBaseImpl::EActorBaseState CActorBaseImpl::getState() const
	{
		return this->m_eState;
	}

	void CActorBaseImpl::setState(EActorBaseState eState)
	{
		this->m_eState = eState;
	}


	void CActorBaseImpl::process()
	{
		if (this->m_channel.empty())
			return;

		for (uint32_t i = 0; i < CCoreServiceAppImpl::Inst()->getThroughput(); ++i)
		{
			SMessagePacket sMessagePacket;
			if (!this->m_channel.recv(sMessagePacket))
				break;

			if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_REQUEST)
			{
				DebugAst(sMessagePacket.nDataSize > sizeof(request_cookice));

				const actor_request_cookice* pCookice = reinterpret_cast<const actor_request_cookice*>(sMessagePacket.pData);
				// °þµôcookice
				const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

				this->m_sActorSessionInfo.nActorID = pCookice->nFromActorID;
				this->m_sActorSessionInfo.nSessionID = pCookice->nSessionID;

				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(0);
				DebugAst(pSerializeAdapter != nullptr);

				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
				
				auto iter = s_mapMessageHandlerInfo.find(pHeader->nMessageID);
				if (iter != s_mapMessageHandlerInfo.end())
				{
					std::vector<CActorBaseImpl::SMessageHandlerInfo>& vecMessageHandlerInfo = iter->second;
					for (size_t k = 0; k < vecMessageHandlerInfo.size(); ++k)
					{
						SMessageHandlerInfo& sMessageHandlerInfo = vecMessageHandlerInfo[k];

						if (!sMessageHandlerInfo.bAsync)
						{
							uint64_t nCoroutineID = coroutine::create(0, [&](uint64_t){ sMessageHandlerInfo.handler(this->m_pActorBase, pCookice->nFromActorID, pMessage); });
							coroutine::resume(nCoroutineID, 0);
						}
						else
						{
							sMessageHandlerInfo.handler(this->m_pActorBase, pCookice->nFromActorID, pMessage);
						}
					}
				}
				
				this->m_sActorSessionInfo.nActorID = 0;
				this->m_sActorSessionInfo.nSessionID = 0;
			}
			else if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_RESPONSE)
			{
				const actor_response_cookice* pCookice = reinterpret_cast<const actor_response_cookice*>(sMessagePacket.pData);
				// °þµôcookice
				const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

				SResponseWaitInfo* pResponseWaitInfo = this->getResponseWaitInfo(pCookice->nSessionID, true);
				if (nullptr != pResponseWaitInfo)
				{
					Defer(delete pResponseWaitInfo);

					if (pResponseWaitInfo->callback != nullptr)
					{
						if (pCookice->nResult == eRRT_OK)
						{
							CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(0);
							DebugAst(pSerializeAdapter != nullptr);

							CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);

							pResponseWaitInfo->callback(pMessage, pCookice->nResult);
						}
						else if (pCookice->nResult != eRRT_OK)
						{
							pResponseWaitInfo->callback(nullptr, pCookice->nResult);
						}
					}
					else if (pResponseWaitInfo->nCoroutineID != 0)
					{
						CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter((uint16_t)sMessagePacket.nID);
						DebugAst(pSerializeAdapter != nullptr);

						CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);

						CMessagePtr<char>* pNewMessage = new CMessagePtr<char>();
						*pNewMessage = pMessage;
						coroutine::setLocalData(pResponseWaitInfo->nCoroutineID, "message", reinterpret_cast<uint64_t>(pNewMessage));
						coroutine::setLocalData(pResponseWaitInfo->nCoroutineID, "result", pCookice->nResult);

						coroutine::resume(pResponseWaitInfo->nCoroutineID, 0);
					}
				}
			}
			else if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_GATE_FORWARD)
			{
				const actor_gate_forward_cookice* pCookice = reinterpret_cast<const actor_gate_forward_cookice*>(sMessagePacket.pData);
				// °þµôcookice
				const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

				SClientSessionInfo session((uint16_t)sMessagePacket.nID, pCookice->nSessionID);
				
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(0);
				DebugAst(pSerializeAdapter != nullptr);

				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);

				auto iter = s_mapForwardHandlerInfo.find(pHeader->nMessageID);
				if (iter != s_mapForwardHandlerInfo.end())
				{
					std::vector<SForwardHandlerInfo>& vecForwardHandlerInfo = iter->second;
					for (size_t k = 0; k < vecForwardHandlerInfo.size(); ++k)
					{
						SForwardHandlerInfo& sForwardHandlerInfo = vecForwardHandlerInfo[k];

						if (!sForwardHandlerInfo.bAsync)
						{
							uint64_t nCoroutineID = coroutine::create(0, [&](uint64_t){ sForwardHandlerInfo.handler(this->m_pActorBase, session, pMessage); });
							coroutine::resume(nCoroutineID, 0);
						}
						else
						{
							sForwardHandlerInfo.handler(this->m_pActorBase, session, pMessage);
						}
					}
				}
			}

			SAFE_DELETE_ARRAY(sMessagePacket.pData);
		}

		if (!this->m_channel.empty())
			CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(this);
	}

	CChannel* CActorBaseImpl::getChannel()
	{
		return &this->m_channel;
	}

	core::SActorSessionInfo CActorBaseImpl::getActorSessionInfo() const
	{
		return this->m_sActorSessionInfo;
	}

	void CActorBaseImpl::onRequestMessageTimeout(uint64_t nContext)
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

	SResponseWaitInfo* CActorBaseImpl::addResponseWaitInfo(uint64_t nSessionID, uint64_t nCoroutineID)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		DebugAstEx(iter == this->m_mapResponseWaitInfo.end(), nullptr);

		SResponseWaitInfo* pResponseWaitInfo = new SResponseWaitInfo();
		pResponseWaitInfo->callback = nullptr;
		pResponseWaitInfo->nSessionID = nSessionID;
		pResponseWaitInfo->nCoroutineID = nCoroutineID;
		pResponseWaitInfo->nToID = 0;
		pResponseWaitInfo->nMessageID = 0;
		pResponseWaitInfo->nBeginTime = 0;
		pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CActorBaseImpl::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&pResponseWaitInfo->tickTimeout, CCoreServiceAppImpl::Inst()->getInvokeTimeout(), 0, nSessionID);

		this->m_mapResponseWaitInfo[pResponseWaitInfo->nSessionID] = pResponseWaitInfo;

		return pResponseWaitInfo;
	}

	SResponseWaitInfo* CActorBaseImpl::getResponseWaitInfo(uint64_t nSessionID, bool bErase)
	{
		auto iter = this->m_mapResponseWaitInfo.find(nSessionID);
		if (iter == this->m_mapResponseWaitInfo.end())
			return nullptr;

		SResponseWaitInfo* pResponseWaitInfo = iter->second;
		if (bErase)
			this->m_mapResponseWaitInfo.erase(iter);

		return pResponseWaitInfo;
	}

	void CActorBaseImpl::registerMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>& handler)
	{
		s_mapMessageHandlerInfo[szMessageName].push_back(handler);
	}

	void CActorBaseImpl::registerForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler)
	{
		s_mapForwardHandlerInfo[szMessageName].push_back(handler);
	}

	std::map<std::string, std::vector<std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>>> CActorBaseImpl::s_mapForwardHandlerInfo;

	std::map<std::string, std::vector<std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>>> CActorBaseImpl::s_mapMessageHandlerInfo;
}