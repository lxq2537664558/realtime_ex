#include "stdafx.h"
#include "actor_base_impl.h"
#include "actor_base.h"
#include "service_base.h"
#include "coroutine.h"
#include "core_app.h"

#include "libCoreCommon/base_app.h"
#include "libBaseCommon/defer.h"

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

		for (uint32_t i = 0; i < CCoreApp::Inst()->getThroughput(); ++i)
		{
			SMessagePacket sMessagePacket;
			if (!this->m_channel.recv(sMessagePacket))
				break;

			if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_REQUEST)
			{
				this->m_sActorSessionInfo.nActorID = sMessagePacket.nID;
				this->m_sActorSessionInfo.nSessionID = sMessagePacket.nSessionID;

				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(sMessagePacket.pData);

				auto iter = s_mapMessageHandlerInfo.find(pMessage->GetTypeName());
				if (iter != s_mapMessageHandlerInfo.end())
				{
					auto& vecMessageHandlerInfo = iter->second;
					for (size_t k = 0; k < vecMessageHandlerInfo.size(); ++k)
					{
						auto& callback = vecMessageHandlerInfo[k];

						uint64_t nCoroutineID = coroutine::create(0, [&](uint64_t){ callback(this->m_pActorBase, this->m_sActorSessionInfo, pMessage); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
				
				this->m_sActorSessionInfo.nActorID = 0;
				this->m_sActorSessionInfo.nSessionID = 0;
			}
			else if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_RESPONSE)
			{
				SResponseWaitInfo* pResponseWaitInfo = this->getResponseWaitInfo(sMessagePacket.nSessionID, true);
				if (nullptr != pResponseWaitInfo)
				{
					Defer(delete pResponseWaitInfo);

					google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(sMessagePacket.pData);

					if (pResponseWaitInfo->callback != nullptr)
					{
						if (sMessagePacket.nID == eRRT_OK)
						{
							pResponseWaitInfo->callback(pMessage, (uint8_t)sMessagePacket.nID);
						}
						else
						{
							pResponseWaitInfo->callback(nullptr, (uint8_t)sMessagePacket.nID);
						}
					}
					else if (pResponseWaitInfo->nCoroutineID != 0)
					{
						coroutine::setLocalData(pResponseWaitInfo->nCoroutineID, "message", reinterpret_cast<uint64_t>(pMessage));
						coroutine::setLocalData(pResponseWaitInfo->nCoroutineID, "result", sMessagePacket.nID);

						coroutine::resume(pResponseWaitInfo->nCoroutineID, 0);
					}
				}
			}
			else if ((sMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_GATE_FORWARD)
			{
				SClientSessionInfo session((uint16_t)sMessagePacket.nID, sMessagePacket.nSessionID);
				
				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(sMessagePacket.pData);

				auto iter = s_mapForwardHandlerInfo.find(pMessage->GetTypeName());
				if (iter != s_mapForwardHandlerInfo.end())
				{
					auto& vecForwardHandlerInfo = iter->second;
					for (size_t k = 0; k < vecForwardHandlerInfo.size(); ++k)
					{
						auto& callback = vecForwardHandlerInfo[k];

						uint64_t nCoroutineID = coroutine::create(0, [&](uint64_t){ callback(this->m_pActorBase, session, pMessage); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
			}

			SAFE_DELETE_ARRAY(sMessagePacket.pData);
		}

		if (!this->m_channel.empty())
			CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(this);
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

		SMessagePacket sMessagePacket;
		sMessagePacket.nID = eRRT_TIME_OUT;
		sMessagePacket.nSessionID = pResponseWaitInfo->nSessionID;
		sMessagePacket.nType = eMT_RESPONSE;
		sMessagePacket.nDataSize = sizeof(core::response_cookice);
		sMessagePacket.pData = nullptr;
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
		pResponseWaitInfo->nBeginTime = 0;
		pResponseWaitInfo->tickTimeout.setCallback(std::bind(&CActorBaseImpl::onRequestMessageTimeout, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(CTicker::eTT_Logic, 0, &pResponseWaitInfo->tickTimeout, CCoreApp::Inst()->getInvokeTimeout(), 0, nSessionID);

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