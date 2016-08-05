#pragma once
#include "libBaseCommon/noncopyable.h"

#include "core_service_kit_define.h"

namespace core
{
	class CBaseActorImpl;
	class CBaseActorFactory;
	class CBaseActor :
		public base::noncopyable
	{
	protected:
		CBaseActor();
		virtual ~CBaseActor();

	public:
		virtual bool		onInit(void* pContext) { return true; }
		virtual void		onDestroy() { }

		uint64_t			getID() const;

		bool				invoke(uint64_t nID, const message_header* pData);
		bool				invoke_r(uint64_t nID, const message_header* pData, CResponseFuture& sActorResponseFuture);
		bool				invoke_r(uint64_t nID, const message_header* pData, InvokeCallback& callback);
		uint32_t			invoke(uint64_t nID, const message_header* pData, CMessage& pResultData);

		SActorSessionInfo	getActorSessionInfo() const;
		void				response(const message_header* pData);
		void				response(const SActorSessionInfo& sActorSessionInfo, const message_header* pData);

		void				registerCallback(uint16_t nMessageID, ActorCallback callback);
		void				registerGateForwardCallback(uint16_t nMessageID, ActorGateForwardCallback callback);
		
		void				release();

		static CBaseActor*	createActor(void* pContext, CBaseActorFactory* pBaseActorFactory);

		static uint16_t		getServiceID(uint64_t nActorID);
		static uint64_t		getLocalActorID(uint64_t nActorID);
		static uint64_t		makeRemoteActorID(uint16_t nServiceID, uint64_t nActorID);

	private:
		CBaseActorImpl*	m_pBaseActorImpl;
	};
}