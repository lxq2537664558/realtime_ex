#pragma once
#include "libBaseCommon/circle_queue.h"

#include "core_common.h"
#include "core_common_define.h"
#include "service_base.h"
#include "actor_base.h"
#include "ticker_runnable.h"

#include <map>

namespace core
{
	typedef base::CCircleQueue<SActorMessagePacket, false> CChannel;

	class CCoreService;
	class CCoreActor :
		public base::noncopyable
	{
	public:
		enum EActorBaseState
		{
			eABS_Pending,
			eABS_RecvPending,
			eABS_Normal,
		};

	public:
		CCoreActor(uint64_t nID, CActorBase* pActorBase, CCoreService* pCoreService);
		~CCoreActor();

		uint64_t				getID() const;
		EActorBaseState			getState() const;
		void					setState(EActorBaseState eState);

		CActorBase*				getActorBase() const;

		CCoreService*			getCoreService() const;

		void					process();
		
		CChannel*				getChannel();
		void					addTicker(CCoreTickerNode* pCoreTickerNode);

		SPendingResponseInfo*	addPendingResponseInfo(uint32_t nToServiceID, uint64_t nSessionID, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<void>, uint32_t)>& callback, uint64_t nHolderID);
		SPendingResponseInfo*	getPendingResponseInfo(uint64_t nSessionID);
		void					delPendingResponseInfo(uint64_t nHolderID);

		void					setSyncPendingResponseMessage(uint8_t nResult, void* pMessage, uint8_t nMessageSerializerType);
		uint64_t				getSyncPendingResponseSessionID() const;

		// 必须要用轮训的方式，因为定时器在actor塞住的时候也不是被塞住了的
		bool					onPendingTimer(int64_t nCurTime);

	private:
		void					onRequestMessageTimeout(uint64_t nContext);
		void					dispatch(const SActorMessagePacket& sActorMessagePacket);

	private:
		uint64_t									m_nID;
		CActorBase*									m_pActorBase;
		CCoreService*								m_pCoreService;
		CChannel									m_channel;
		std::list<CCoreTickerNode*>					m_listerTicker;
		EActorBaseState								m_eState;
		std::map<uint64_t, SPendingResponseInfo*>	m_mapPendingResponseInfo;
		SPendingResponseInfo*						m_pSyncPendingResponseInfo;	// 同步调用
		uint64_t									m_nSyncPendingResponseHolderID;
		uint8_t										m_nSyncPendingResponseResult;
		uint8_t										m_nSyncPendingResponseMessageSerializerType;
		void*										m_pSyncPendingResponseMessage;
		std::map<uint64_t, std::list<uint64_t>>		m_mapHolderSessionIDList;
	};
}