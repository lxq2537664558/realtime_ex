#pragma once
#include "libBaseCommon/circle_queue.h"

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"
#include "service_base.h"

#include <map>

namespace core
{
	typedef base::CCircleQueue<SMessagePacket, false> CChannel;

	class CBaseActor;
	class CBaseActorImpl :
		public base::noncopyable
	{
	public:
		CBaseActorImpl(uint64_t nID, CBaseActor* pActor);
		~CBaseActorImpl();

		uint64_t			getID() const;
		void				process();
		CChannel*			getChannel();
		SActorSessionInfo	getActorSessionInfo() const;
		void				addResponseWaitInfo(uint64_t nSessionID, uint64_t nTraceID, uint64_t nCoroutineID);
		SResponseWaitInfo*	getResponseWaitInfo(uint64_t nSessionID, bool bErase);

		void				registerCallback(uint16_t nMessageID, ActorCallback callback);
		void				registerGateForwardCallback(uint16_t nMessageID, ActorGateForwardCallback callback);

	private:
		void				onRequestMessageTimeout(uint64_t nContext);

	private:
		std::map<uint16_t, ActorCallback>
							m_mapActorCallback;
		std::map<uint16_t, ActorGateForwardCallback>
							m_mapActorGateForwardCallback;
		uint64_t			m_nID;
		CBaseActor*			m_pBaseActor;
		SActorSessionInfo	m_sActorSessionInfo;
		CChannel			m_channel;

		std::map<uint64_t, SResponseWaitInfo*>
							m_mapResponseWaitInfo;
	};
}