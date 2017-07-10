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
		for (uint32_t i = 0; i < CCoreApp::Inst()->getThroughput(); ++i)
		{
			SActorMessagePacket sActorMessagePacket;
			if (!this->m_channel.recv(sActorMessagePacket))
				break;

			SAFE_DELETE(sActorMessagePacket.pMessage);
		}
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

		// 这里协程回调使用的pMessage的生命周期跟协程一致，采用值捕获lambda的方式来达到这一目的
		for (uint32_t i = 0; i < CCoreApp::Inst()->getThroughput(); ++i)
		{
			SActorMessagePacket sActorMessagePacket;
			if (!this->m_channel.recv(sActorMessagePacket))
				break;

			if ((sActorMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_REQUEST)
			{
				this->m_sActorSessionInfo.nActorID = sActorMessagePacket.nData;
				this->m_sActorSessionInfo.nSessionID = sActorMessagePacket.nSessionID;
				
				auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);
				auto iter = s_mapMessageHandlerInfo.find(pMessage->GetTypeName());
				if (iter != s_mapMessageHandlerInfo.end())
				{
					auto& vecMessageHandlerInfo = iter->second;
					for (size_t k = 0; k < vecMessageHandlerInfo.size(); ++k)
					{
						auto& callback = vecMessageHandlerInfo[k];

						uint64_t nCoroutineID = coroutine::create(0, [&callback, this, pMessage](uint64_t){ callback(this->m_pActorBase, this->m_sActorSessionInfo, pMessage.get()); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
				
				this->m_sActorSessionInfo.nActorID = 0;
				this->m_sActorSessionInfo.nSessionID = 0;
			}
			else if ((sActorMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_RESPONSE)
			{
				auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);
				auto pResponseWaitInfo = std::unique_ptr<SResponseWaitInfo>(this->getResponseWaitInfo(sActorMessagePacket.nSessionID, true));
				if (nullptr != pResponseWaitInfo)
				{
					if (pResponseWaitInfo->nCoroutineID != 0)
					{
						coroutine::setLocalData(pResponseWaitInfo->nCoroutineID, "message", reinterpret_cast<uint64_t>(&pMessage));
						coroutine::setLocalData(pResponseWaitInfo->nCoroutineID, "result", sActorMessagePacket.nData);

						coroutine::resume(pResponseWaitInfo->nCoroutineID, 0);
					}
					else if (pResponseWaitInfo->callback != nullptr)
					{
						if (sActorMessagePacket.nData == eRRT_OK)
						{
							pResponseWaitInfo->callback(pMessage.get(), (uint8_t)sActorMessagePacket.nData);
						}
						else
						{
							pResponseWaitInfo->callback(nullptr, (uint8_t)sActorMessagePacket.nData);
						}
					}
					else
					{
						PrintWarning("invalid response session_id: "UINT64FMT" actor_id: "UINT64FMT" message_name: %s", pResponseWaitInfo->nSessionID, this->getID(), pResponseWaitInfo->szMessageName.c_str());
					}
				}
			}
			else if ((sActorMessagePacket.nType&eMT_TYPE_MASK) == eMT_ACTOR_GATE_FORWARD)
			{
				SClientSessionInfo session((uint16_t)sActorMessagePacket.nData, sActorMessagePacket.nSessionID);
				
				auto pMessage = std::unique_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);

				auto iter = s_mapForwardHandlerInfo.find(pMessage->GetTypeName());
				if (iter != s_mapForwardHandlerInfo.end())
				{
					auto& vecForwardHandlerInfo = iter->second;
					for (size_t k = 0; k < vecForwardHandlerInfo.size(); ++k)
					{
						auto& callback = vecForwardHandlerInfo[k];

						uint64_t nCoroutineID = coroutine::create(0, [&](uint64_t){ callback(this->m_pActorBase, session, pMessage.get()); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
			}
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

		SActorMessagePacket sActorMessagePacket;
		sActorMessagePacket.nData = eRRT_TIME_OUT;
		sActorMessagePacket.nSessionID = pResponseWaitInfo->nSessionID;
		sActorMessagePacket.nType = eMT_ACTOR_RESPONSE;
		sActorMessagePacket.pMessage = nullptr;

		this->m_channel.send(sActorMessagePacket);
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