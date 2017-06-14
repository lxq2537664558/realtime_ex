#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/coroutine.h"

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

		bool				invoke(uint64_t nID, const void* pData);

		template<class T>
		bool				invoke_r(uint64_t nID, const void* pData, const std::function<void(CMessagePtr<T>, uint32_t)>& callback);

		template<class T>
		inline uint32_t		invoke(uint64_t nID, const void* pData, CMessagePtr<T>& pResultMessage);

		template<class T>
		inline bool			invoke_r(uint64_t nID, const void* pData, CFuture<CMessagePtr<T>>& sFuture);

		SActorSessionInfo	getActorSessionInfo() const;
		void				response(const void* pData);
		void				response(const SActorSessionInfo& sActorSessionInfo, const void* pData);

		void				release();

		static void			registerMessageHandler(uint16_t nMessageID, const std::function<void(CBaseActor*, uint64_t, CMessagePtr<char>)>& handler, bool bAsync);
		static void			registerForwardHandler(uint16_t nMessageID, const std::function<void(CBaseActor*, SClientSessionInfo, CMessagePtr<char>)>& handler, bool bAsync);

		static CBaseActor*	createActor(void* pContext, CBaseActorFactory* pBaseActorFactory);

		static uint16_t		getServiceID(uint64_t nActorID);
		static uint64_t		getLocalActorID(uint64_t nActorID);
		static uint64_t		makeRemoteActorID(uint16_t nServiceID, uint64_t nActorID);

	private:
		bool				invokeImpl(uint64_t nID, const void* pData, uint64_t nCoroutineID, const std::function<void(CMessagePtr<char>, uint32_t)>& callback);

	private:
		CBaseActorImpl*	m_pBaseActorImpl;
	};
}

#include "base_actor.inl"