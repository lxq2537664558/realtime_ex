#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"
#include "service_base.h"

#include "google/protobuf/message.h"

namespace core
{
	/*
	这是一个帮助类，为了服务调用捕获了this或者一些对象级别生命周期的对象，在对象销毁后还有未返回的rpc响应的情况
	*/
	class CServiceInvokeHolder
	{
	public:
		CServiceInvokeHolder(CServiceBase* pServiceBase);
		virtual ~CServiceInvokeHolder();

		template<class T>
		inline void		async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		
		template<class T>
		inline void		async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		//==================================指定服务之间调用=======================================//



		//==================================指定服务类型之间调用=======================================//
		template<class T>
		inline void		async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		
		template<class T>
		inline void		async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		//==================================指定服务类型之间调用=======================================//

		uint64_t		getHolderID() const;
		CServiceBase*	getServiceBase() const;

	private:
		CServiceBase*	m_pServiceBase;
		uint64_t		m_nID;
	};
}