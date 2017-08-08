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
	����һ�������࣬Ϊ�˷�����ò�����this����һЩ���󼶱��������ڵĶ����ڶ������ٺ���δ���ص�rpc��Ӧ�����
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
		//==================================ָ������֮�����=======================================//



		//==================================ָ����������֮�����=======================================//
		template<class T>
		inline void		async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		
		template<class T>
		inline void		async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		//==================================ָ����������֮�����=======================================//

		uint64_t		getHolderID() const;
		CServiceBase*	getServiceBase() const;

	private:
		CServiceBase*	m_pServiceBase;
		uint64_t		m_nID;
	};
}