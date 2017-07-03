#pragma once
#include "libBaseCommon/circle_queue.h"

#include "libCoreCommon/core_common.h"

#include "core_service_kit_common.h"
#include "core_service_define.h"
#include "service_base.h"
#include "actor_base.h"

#include <map>

namespace core
{
	typedef base::CCircleQueue<SMessagePacket, false> CChannel;

	class CActorBase;
	class CActorBaseImpl :
		public base::noncopyable
	{
	public:
		enum EActorBaseState
		{
			eABS_Pending,
			eABS_Working,
			eABS_Empty,
		};

	public:
		CActorBaseImpl(uint64_t nID, CActorBase* pActorBase);
		~CActorBaseImpl();

		uint64_t			getID() const;
		EActorBaseState		getState() const;
		void				setState(EActorBaseState eState);

		void				process();
		CChannel*			getChannel();
		SActorSessionInfo	getActorSessionInfo() const;
		SResponseWaitInfo*	addResponseWaitInfo(uint64_t nSessionID, uint64_t nCoroutineID);
		SResponseWaitInfo*	getResponseWaitInfo(uint64_t nSessionID, bool bErase);

		static void			registerMessageHandler(uint16_t nMessageID, const std::function<void(CActorBase*, uint64_t, CMessagePtr<char>)>& handler, bool bAsync);
		static void			registerForwardHandler(uint16_t nMessageID, const std::function<void(CActorBase*, SClientSessionInfo, CMessagePtr<char>)>& handler, bool bAsync);

	private:
		void				onRequestMessageTimeout(uint64_t nContext);

	private:
		uint64_t			m_nID;
		CActorBase*			m_pActorBase;
		SActorSessionInfo	m_sActorSessionInfo;
		CChannel			m_channel;
		EActorBaseState		m_eState;
		std::map<uint64_t, SResponseWaitInfo*>
							m_mapResponseWaitInfo;

		struct SMessageHandlerInfo
		{
			std::function<void(CActorBase*, uint64_t, CMessagePtr<char>)>
							handler;
			bool			bAsync;
		};

		struct SForwardHandlerInfo
		{
			std::function<void(CActorBase*, SClientSessionInfo, CMessagePtr<char>)>
							handler;
			bool			bAsync;
		};

		static std::map<uint16_t, std::vector<SMessageHandlerInfo>>	s_mapMessageHandlerInfo;
		static std::map<uint16_t, std::vector<SForwardHandlerInfo>> s_mapForwardHandlerInfo;
	};
}