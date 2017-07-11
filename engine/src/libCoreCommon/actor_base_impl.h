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
		SResponseWaitInfo*	addResponseWaitInfo(uint64_t nSessionID, uint64_t nCoroutineID, uint64_t nToID, const std::string& szMessageName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		SResponseWaitInfo*	getResponseWaitInfo(uint64_t nSessionID, bool bErase);

		static void			registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>& handler);
		static void			registerForwardMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler);
		
	private:
		void				onRequestMessageTimeout(uint64_t nContext);

	private:
		uint64_t			m_nID;
		CActorBase*			m_pActorBase;
		CChannel			m_channel;
		EActorBaseState		m_eState;
		std::map<uint64_t, SResponseWaitInfo*>
							m_mapResponseWaitInfo;

		static std::map<std::string, std::vector<std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>>>	s_mapActorMessageHandlerInfo;
		static std::map<std::string, std::vector<std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>>>	s_mapForwardMessageHandlerInfo;
	};
}