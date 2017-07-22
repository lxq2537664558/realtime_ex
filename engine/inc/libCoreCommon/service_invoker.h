#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "google/protobuf/message.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"

#include <vector>

namespace core
{
	class CServiceBaseImpl;
	class CServiceInvoker :
		public base::noncopyable
	{
	public:
		CServiceInvoker(CServiceBaseImpl* pServiceBaseImpl);
		~CServiceInvoker();

		bool				init();

		bool				send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage);

		bool				broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage);

		/**
		@brief: ͨ��callback�ķ�ʽ����Զ�̵��ã����õ�ʱ�����÷�����Ϣ������ʵ����ģ�庯��
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);

		/**
		@brief: ͨ��future�ķ�ʽ����Զ�̵���
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		static bool			send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);

		static bool			broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback);
	
	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}

#include "service_invoker.inl"