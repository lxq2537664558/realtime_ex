#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"
#include "actor_base.h"

#include "google/protobuf/message.h"

namespace core
{
	/*
	这是一个帮助类，为了服务调用捕获了this或者一些对象级别生命周期的对象，在对象销毁后还有未返回的rpc响应的情况
	*/
	class CActorInvokeHolder
	{
	public:
		CActorInvokeHolder(CActorBase* pActorBase);
		virtual ~CActorInvokeHolder();

		
		template<class T>
		inline void		async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		
		template<class T>
		inline void		async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		template<class T>
		inline uint32_t	sync_call(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);

		template<class T>
		inline void		async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);

		template<class T>
		inline void		async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		uint64_t		getHolderID() const;
		CActorBase*		getActorBase() const;

	private:
		CActorBase*		m_pActorBase;
		uint64_t		m_nID;
	};
}