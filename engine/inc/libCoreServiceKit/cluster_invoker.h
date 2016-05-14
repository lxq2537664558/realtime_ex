#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"

#include "google/protobuf/message.h"

#include "core_service_kit_define.h"
#include "load_balance.h"

namespace core
{
	/**
	@brief: ��Ⱥ���������
	*/
	class CClusterInvoker :
		public base::CSingleton<CClusterInvoker>
	{
	public:
		CClusterInvoker();
		~CClusterInvoker();

		bool				init();

		/**
		@brief: ����ָ��Զ�̷������Ϣ��Զ�̷����÷�����Ϣ
		*/
		bool				invok(const std::string& szServiceName, const google::protobuf::Message* pMessage);
		/**
		@brief: ����ָ��Զ�̷������Ϣ����Ҫ�ṩԶ�̷�����Ϣ���ص���Ӧ�����ص�
		*/
		bool				invok_r(const std::string& szServiceName, const google::protobuf::Message* pMessage, InvokeCallback callback, uint64_t nContext = 0);
		/**
		@brief: ͨ����Ϣ���ֵ���Զ�̷��񣬸��ݸ��ؾ�����ѡ�����
		*/
		bool				invok(const google::protobuf::Message* pMessage, uint64_t nSessionID, const std::string& szServiceGroup);
		/**
		@brief: ͨ����Ϣ���ֵ���Զ�̷��񣬸��ݸ��ؾ�����ѡ�������Ҫ�ṩԶ�̷�����Ϣ���ص���Ӧ�����ص�
		*/
		bool				invok_r(const google::protobuf::Message* pMessage, uint64_t nSessionID, const std::string& szServiceGroup, InvokeCallback callback, uint64_t nContext = 0);
		/**
		@brief: ��ӦԶ�̷���ĵ��ã�������Ӧ��Ϣ
		*/
		void				response(const google::protobuf::Message* pMessage);
		/**
		@brief: ��ӦԶ�̷���ĵ��ã�������Ӧ��Ϣ�������Ҫ�û��Լ��ƶ�����session����Ҫ���ڲ���������Ӧ����Ҫ�ȴ�������Ϣ����Ϣ
		*/
		void				response(const SServiceSessionInfo& sServiceSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: ��ȡ��ǰ����session��Ϣ
		*/
		SServiceSessionInfo	getServiceSessionInfo() const;
		//=============================�����ǶԼ�Ⱥ�ڲ�����֮�����Ϣ���ͺ���=========================//

		//=================================�����ǶԿͻ��˵���Ϣ��������==============================//
		/**
		@brief: ��ͻ��˷�����Ϣ���ͻ���ID��SGateSessionInfo������
		*/
		bool				send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: ��һ���ͻ��˹㲥��Ϣ���ͻ���ID��std::vector<SGateSessionInfo>������
		*/
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: ���ط���ת���ͻ�����Ϣ
		*/
		bool				forward(const message_header* pHeader, uint64_t nSessionID, const std::string& szServiceGroup);
	};
}