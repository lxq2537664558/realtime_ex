#pragma once
#include "libBaseCommon/circle_queue.h"

#include "core_common.h"
#include "core_common_define.h"
#include "service_base.h"
#include "actor_base.h"

#include <map>

namespace core
{
	typedef base::CCircleQueue<SActorMessagePacket, false> CChannel;

	class CServiceBaseImpl;
	class CActorBaseImpl :
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
		CActorBaseImpl(uint64_t nID, CActorBase* pActorBase, CServiceBaseImpl* pServiceBaseImpl);
		~CActorBaseImpl();

		uint64_t				getID() const;
		EActorBaseState			getState() const;
		void					setState(EActorBaseState eState);

		CActorBase*				getActorBase() const;

		CServiceBaseImpl*		getServiceBaseImpl() const;

		void					registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void					unregisterTicker(CTicker* pTicker);

		void					process();
		CChannel*				getChannel();
		SPendingResponseInfo*	addPendingResponseInfo(uint64_t nSessionID, uint64_t nCoroutineID, uint64_t nToID, const std::string& szMessageName, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback);
		SPendingResponseInfo*	getPendingResponseInfo(uint64_t nSessionID, bool bErase);
		
		void					setPendingResponseMessage(uint8_t nResult, google::protobuf::Message* pMessage);

		uint64_t				getPendingResponseSessionID() const;

		bool					onPendingTimer(int64_t nCurTime);

	private:
		void					onRequestMessageTimeout(uint64_t nContext);
		void					dispatch(const SActorMessagePacket& sActorMessagePacket);

	private:
		uint64_t									m_nID;
		CActorBase*									m_pActorBase;
		CServiceBaseImpl*							m_pServiceBaseImpl;
		CChannel									m_channel;
		EActorBaseState								m_eState;
		std::map<uint64_t, SPendingResponseInfo*>	m_mapPendingResponseInfo;
		SPendingResponseInfo*						m_pSyncPendingResponseInfo;	// 同步调用
		uint8_t										m_nPendingResponseResult;
		google::protobuf::Message*					m_pPendingResponseMessage;
	};
}