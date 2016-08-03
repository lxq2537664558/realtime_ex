#pragma once
#include "libBaseCommon/noncopyable.h"

#include "core_service_kit_define.h"

namespace core
{
	class CActorBase;
	class CActor :
		public base::noncopyable
	{
	public:
		CActor();
		virtual ~CActor();

		uint64_t			getID() const;

		bool				invoke(uint64_t nID, const message_header* pData);
		bool				invoke_r(uint64_t nID, const message_header* pData, CResponseFuture& sActorResponseFuture);
		bool				invoke_r(uint64_t nID, const message_header* pData, InvokeCallback& callback);
		uint32_t			invoke(uint64_t nID, const message_header* pData, CMessage& pResultData);

		SActorSessionInfo	getActorSessionInfo() const;
		void				response(const message_header* pData);
		void				response(const SActorSessionInfo& sActorSessionInfo, const message_header* pData);

		virtual void		onDispatch(uint64_t nFrom, uint8_t nMessageType, CMessage pMessage) { }
		virtual void		onForward(core::SClientSessionInfo sClientSessionInfo, uint8_t nMessageType, CMessage pMessage) { }

		static uint16_t		getServiceID(uint64_t nActorID);
		static uint64_t		getLocalActorID(uint64_t nActorID);
		static uint64_t		getRemoteActorID(uint16_t nServiceID, uint64_t nActorID);

	private:
		CActorBase*	m_pActorBase;
	};
}