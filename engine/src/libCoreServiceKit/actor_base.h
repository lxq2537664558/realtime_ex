#pragma once
#include "libBaseCommon/circle_queue.h"

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"
#include "service_base.h"

#include <map>

namespace core
{
	typedef base::CCircleQueue<SMessagePacket, false> CChannel;

	class CActor;
	class CActorBase :
		public base::noncopyable
	{
	public:
		CActorBase(uint64_t nID, CActor* pActor);
		~CActorBase();

		uint64_t			getID() const;
		void				run();
		CChannel*			getChannel();
		SActorSessionInfo	getActorSessionInfo() const;
		void				addResponseWaitInfo(uint64_t nSessionID, uint64_t nTraceID, uint64_t nCoroutineID);
		SResponseWaitInfo*	getResponseWaitInfo(uint64_t nSessionID, bool bErase);

	private:
		void				onRequestMessageTimeout(uint64_t nContext);

	private:
		uint64_t								m_nID;
		CActor*									m_pActor;
		SActorSessionInfo						m_sActorSessionInfo;
		CChannel								m_channel;

		std::map<uint64_t, SResponseWaitInfo*>	m_mapResponseWaitInfo;
	};
}