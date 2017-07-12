#include "stdafx.h"
#include "actor_base_impl.h"
#include "actor_base.h"
#include "service_base.h"
#include "coroutine.h"
#include "core_app.h"

#include "libCoreCommon/base_app.h"
#include "libBaseCommon/defer.h"
#include "libBaseCommon/base_time.h"

#define _DEFAULT_CHANNEL_CAP 256

namespace core
{
	CActorBaseImpl::CActorBaseImpl(uint64_t nID, CActorBase* pActorBase)
		: m_channel(_DEFAULT_CHANNEL_CAP)
		, m_nID(nID)
		, m_pActorBase(pActorBase)
		, m_pSyncPendingResponseInfo(nullptr)
		, m_eState(eABS_Normal)
	{

	}

	CActorBaseImpl::~CActorBaseImpl()
	{
		SAFE_DELETE(this->m_pPendingResponseMessage);

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

		if (this->m_eState == eABS_RecvPending)
		{
			this->m_eState = eABS_Normal;

			SActorMessagePacket sActorMessagePacket;
			sActorMessagePacket.nData = this->m_nPendingResponseResult;
			sActorMessagePacket.nSessionID = this->getPendingResponseSessionID();
			sActorMessagePacket.nType = eMT_RESPONSE;
			sActorMessagePacket.pMessage = this->m_pPendingResponseMessage;

			this->dispatch(sActorMessagePacket);

			this->m_pPendingResponseMessage = nullptr;
			this->m_nPendingResponseResult = 0;
			
		}

		for (uint32_t i = 0; i < CCoreApp::Inst()->getThroughput(); ++i)
		{
			SActorMessagePacket sActorMessagePacket;
			if (!this->m_channel.recv(sActorMessagePacket))
				break;

			if (this->m_eState == eABS_Pending)
				break;

			this->dispatch(sActorMessagePacket);
		}

		if (!this->m_channel.empty() && this->m_eState == eABS_Normal)
			CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(this);
	}

	void CActorBaseImpl::dispatch(const SActorMessagePacket& sActorMessagePacket)
	{
		// 这里协程回调使用的pMessage的生命周期跟协程一致，采用值捕获lambda的方式来达到这一目的
		if (sActorMessagePacket.nType == eMT_REQUEST)
		{
			auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);
			auto iter = s_mapMessageHandlerInfo.find(pMessage->GetTypeName());
			if (iter != s_mapMessageHandlerInfo.end())
			{
				SSessionInfo sSessionInfo;
				sSessionInfo.nFromID = sActorMessagePacket.nData;
				sSessionInfo.nSessionID = sActorMessagePacket.nSessionID;
				sSessionInfo.eTargetType = eMTT_Actor;

				auto& vecMessageHandlerInfo = iter->second;
				for (size_t k = 0; k < vecMessageHandlerInfo.size(); ++k)
				{
					auto& callback = vecMessageHandlerInfo[k];

					uint64_t nCoroutineID = coroutine::create(0, [&callback, this, pMessage, sSessionInfo](uint64_t){ callback(this->m_pActorBase, sSessionInfo, pMessage.get()); });
					coroutine::resume(nCoroutineID, 0);
				}
			}
		}
		else if (sActorMessagePacket.nType == eMT_RESPONSE)
		{
			auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);
			auto pPendingResponseInfo = std::unique_ptr<SPendingResponseInfo>(this->getPendingResponseInfo(sActorMessagePacket.nSessionID, true));
			if (nullptr != pPendingResponseInfo)
			{
				if (pPendingResponseInfo->nCoroutineID != 0)
				{
					// 这里不能直接传pMessage的地址
					SSyncCallResultInfo* pSyncCallResultInfo = new SSyncCallResultInfo();
					pSyncCallResultInfo->nResult = (uint8_t)sActorMessagePacket.nData;
					pSyncCallResultInfo->pMessage = pMessage;
					coroutine::setLocalData(pPendingResponseInfo->nCoroutineID, "response", reinterpret_cast<uint64_t>(pSyncCallResultInfo));
					
					coroutine::resume(pPendingResponseInfo->nCoroutineID, 0);
				}
				else if (pPendingResponseInfo->callback != nullptr)
				{
					uint8_t nResult = (uint8_t)sActorMessagePacket.nData;
					if (sActorMessagePacket.nData == eRRT_OK)
					{
						uint64_t nCoroutineID = coroutine::create(0, [&pPendingResponseInfo, pMessage, nResult](uint64_t){ pPendingResponseInfo->callback(pMessage.get(), nResult); });
						coroutine::resume(nCoroutineID, 0);
					}
					else
					{
						uint64_t nCoroutineID = coroutine::create(0, [&pPendingResponseInfo, nResult](uint64_t){ pPendingResponseInfo->callback(nullptr, nResult); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
				else
				{
					PrintWarning("invalid response session_id: "UINT64FMT" actor_id: "UINT64FMT" message_name: %s", pPendingResponseInfo->nSessionID, this->getID(), pPendingResponseInfo->szMessageName.c_str());
				}
			}
		}
		else if (sActorMessagePacket.nType == eMT_GATE_FORWARD)
		{
			SClientSessionInfo sClientSessionInfo;
			sClientSessionInfo.nGateServiceID = (uint16_t)sActorMessagePacket.nData;
			sClientSessionInfo.nSessionID = sActorMessagePacket.nSessionID;
			auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);

			auto iter = s_mapForwardMessageHandlerInfo.find(pMessage->GetTypeName());
			if (iter != s_mapForwardMessageHandlerInfo.end())
			{
				auto& vecForwardHandlerInfo = iter->second;
				for (size_t k = 0; k < vecForwardHandlerInfo.size(); ++k)
				{
					auto& callback = vecForwardHandlerInfo[k];

					uint64_t nCoroutineID = coroutine::create(0, [&callback, this, pMessage, sClientSessionInfo](uint64_t){ callback(this->m_pActorBase, sClientSessionInfo, pMessage.get()); });
					coroutine::resume(nCoroutineID, 0);
				}
			}
		}
	}

	CChannel* CActorBaseImpl::getChannel()
	{
		return &this->m_channel;
	}

	uint64_t CActorBaseImpl::getPendingResponseSessionID() const
	{
		if (this->m_pSyncPendingResponseInfo == nullptr)
			return 0;

		return this->m_pSyncPendingResponseInfo->nSessionID;
	}

	void CActorBaseImpl::setPendingResponseMessage(uint8_t nResult, google::protobuf::Message* pMessage)
	{
		DebugAst(this->m_eState == eABS_Pending);

		this->m_nPendingResponseResult = nResult;
		this->m_pPendingResponseMessage = pMessage;

		this->m_eState = eABS_RecvPending;
	}

	void CActorBaseImpl::onRequestMessageTimeout(uint64_t nContext)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nContext);
		if (iter == this->m_mapPendingResponseInfo.end())
		{
			PrintWarning("iter == this->m_mapResponseWaitInfo.end() session_id: "UINT64FMT, nContext);
			return;
		}

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		if (nullptr == pPendingResponseInfo)
		{
			PrintWarning("nullptr == pPendingResponseInfo session_id: "UINT64FMT, nContext);
			return;
		}

		SActorMessagePacket sActorMessagePacket;
		sActorMessagePacket.nData = eRRT_TIME_OUT;
		sActorMessagePacket.nSessionID = pPendingResponseInfo->nSessionID;
		sActorMessagePacket.nType = eMT_RESPONSE;
		sActorMessagePacket.pMessage = nullptr;

		this->m_channel.send(sActorMessagePacket);
	}

	SPendingResponseInfo* CActorBaseImpl::addPendingResponseInfo(uint64_t nSessionID, uint64_t nCoroutineID, uint64_t nToID, const std::string& szMessageName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		if (nCoroutineID == 0)
		{
			auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
			DebugAstEx(iter == this->m_mapPendingResponseInfo.end(), nullptr);

			SPendingResponseInfo* pPendingResponseInfo = new SPendingResponseInfo();
			pPendingResponseInfo->callback = nullptr;
			pPendingResponseInfo->nSessionID = nSessionID;
			pPendingResponseInfo->nCoroutineID = nCoroutineID;
			pPendingResponseInfo->nToID = nToID;
			pPendingResponseInfo->szMessageName = szMessageName;
			pPendingResponseInfo->nBeginTime = base::getGmtTime();
			pPendingResponseInfo->tickTimeout.setCallback(std::bind(&CActorBaseImpl::onRequestMessageTimeout, this, std::placeholders::_1));
			CBaseApp::Inst()->registerTicker(CTicker::eTT_Logic, 0, &pPendingResponseInfo->tickTimeout, CCoreApp::Inst()->getInvokeTimeout(), 0, nSessionID);

			this->m_mapPendingResponseInfo[pPendingResponseInfo->nSessionID] = pPendingResponseInfo;

			return pPendingResponseInfo;
		}
		else
		{
			if (nCoroutineID != 0 && this->m_pSyncPendingResponseInfo != nullptr)
			{
				DebugAstEx(!"nCoroutineID != 0 && this->m_pSyncPendingResponseInfo != nullptr", false);
			}

			this->m_pSyncPendingResponseInfo = new SPendingResponseInfo();
			this->m_pSyncPendingResponseInfo->callback = nullptr;
			this->m_pSyncPendingResponseInfo->nSessionID = nSessionID;
			this->m_pSyncPendingResponseInfo->nCoroutineID = nCoroutineID;
			this->m_pSyncPendingResponseInfo->nToID = nToID;
			this->m_pSyncPendingResponseInfo->szMessageName = szMessageName;
			this->m_pSyncPendingResponseInfo->nBeginTime = base::getGmtTime();

			CCoreApp::Inst()->getActorScheduler()->addPendingActorBase(this);

			return this->m_pSyncPendingResponseInfo;
		}
	}

	SPendingResponseInfo* CActorBaseImpl::getPendingResponseInfo(uint64_t nSessionID, bool bErase)
	{
		if (this->m_pSyncPendingResponseInfo != nullptr && this->m_pSyncPendingResponseInfo->nSessionID == nSessionID)
		{
			SPendingResponseInfo* pPendingResponseInfo = this->m_pSyncPendingResponseInfo;
			if (bErase)
				this->m_pSyncPendingResponseInfo = nullptr;

			return pPendingResponseInfo;
		}

		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		if (iter == this->m_mapPendingResponseInfo.end())
			return nullptr;

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		if (bErase)
			this->m_mapPendingResponseInfo.erase(iter);

		return pPendingResponseInfo;
	}

	bool CActorBaseImpl::onPendingTimer(int64_t nCurTime)
	{
		if (this->m_pSyncPendingResponseInfo == nullptr)
			return false;

		if (nCurTime - this->m_pSyncPendingResponseInfo->nBeginTime < CCoreApp::Inst()->getInvokeTimeout())
			return false;
		
		this->setPendingResponseMessage(eRRT_TIME_OUT, nullptr);
		
		CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(this);

		return true;
	}

	void CActorBaseImpl::registerMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& handler)
	{
		s_mapMessageHandlerInfo[szMessageName].push_back(handler);
	}

	void CActorBaseImpl::registerForwardMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler)
	{
		s_mapForwardMessageHandlerInfo[szMessageName].push_back(handler);
	}

	std::map<std::string, std::vector<std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>>> CActorBaseImpl::s_mapForwardMessageHandlerInfo;

	std::map<std::string, std::vector<std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>>> CActorBaseImpl::s_mapMessageHandlerInfo;
}