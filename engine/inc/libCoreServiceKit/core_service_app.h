#pragma once
#include "libCoreCommon/base_app.h"

#include "libBaseLua/lua_facade.h"

#include "core_service_kit_common.h"
#include "actor_id_converter.h"
#include "service_invoker.h"

namespace core
{
	class CCoreServiceApp :
		public CBaseApp
	{
	public:
		CCoreServiceApp();
		~CCoreServiceApp();

		static CCoreServiceApp* Inst();

		/**
		@brief: ��ȡ���ڵ������Ϣ
		*/
		const SNodeBaseInfo&	getNodeBaseInfo() const;
		/**
		@brief: ���ݽڵ����ֻ�ȡ�ڵ�id
		*/
		uint16_t				getServiceID(const std::string& szName) const;
		/**
		@brief: ����ȫ�ֵĽڵ����ӳɹ��ص�
		*/
		void					setNodeConnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: ����ȫ�ֵĽڵ����ӶϿ��ص�
		*/
		void					setNodeDisconnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: ����actoridת����
		*/
		void					setActorIDConverter(CActorIDConverter* pActorIDConverter);
		/**
		@brief: ע����ͨ�ڵ���Ϣ
		*/
		void					registerMessageHandler(uint16_t nServiceID, const std::string& szMessageName, const std::function<bool(uint16_t, google::protobuf::Message*)>& callback);
		/**
		@brief: ע�ᾭ���ؽڵ�ת���ͻ��˵���Ϣ
		*/
		void					registerForwardHandler(uint16_t nServiceID, const std::string& szMessageName, const std::function<bool(SClientSessionInfo, google::protobuf::Message*)>& callback);
		/**
		@brief: ��ȡlua��װ����
		*/
		base::CLuaFacade*		getLuaFacade() const;
		/**
		@brief: ��ȡĳһ������ĵ�����
		*/
		CServiceInvoker*		getServiceInvoker(uint16_t nServiceID) const;
		
	protected:
		virtual bool			onInit();
		virtual void			onProcess();
		virtual void			onDestroy();
	};

}