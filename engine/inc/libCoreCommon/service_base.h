#pragma once

#include "core_common.h"
#include "ticker.h"
#include "service_id_converter.h"
#include "message_serializer.h"
#include "service_selector.h"

#include "libBaseCommon/buf_file.h"

#include <set>

namespace core
{
	// 服务状态 eSRS_Start->eSRS_Normal->eSRS_Quitting->eSRS_Quit
	// 其中eSRS_Quitting状态切换到eSRS_Quit是有逻辑层主动调用doQuit来完成的，
	// 这样做保证了关服前做一些需要比较长的时间来确认一些事的行为，比如数据存储
	enum EServiceRunState
	{
		eSRS_Start		= 0,	// 启动状态
		eSRS_Normal		= 1,	// 正常状态
		eSRS_Quitting	= 2,	// 退出中
		eSRS_Quit		= 3,	// 最终退出
	};

	class CServiceInvoker;
	class CServiceInvokeHolder;
	class CCoreService;
	class CCoreServiceMgr;
	class CBaseConnectionMgr;
	/**
	@brief: 服务基础类
	*/
	class __CORE_COMMON_API__ CServiceBase :
		public base::noncopyable
	{
		friend class CCoreService;
		friend class CServiceInvoker;
		friend class CServiceInvokeHolder;
		friend class CCoreServiceMgr;

	public:
		CServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
		virtual ~CServiceBase();

		uint32_t			getServiceID() const;
		/**
		@brief: 获取服务基础信息
		*/
		const SServiceBaseInfo&
							getServiceBaseInfo() const;

		/**
		@brief: 添加服务选择器
		*/
		void				setServiceSelector(uint32_t nType, CServiceSelector* pServiceSelector);
		/**
		@brief: 获取服务选择器
		*/
		CServiceSelector*	getServiceSelector(uint32_t nType) const;

		/**
		@brief: 判断服务是否健康
		*/
		bool				isServiceHealth(uint32_t nServiceID) const;

		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void				unregisterTicker(CTicker* pTicker);
		/*
		@brief: 获取连接管理器
		*/
		CBaseConnectionMgr*	getBaseConnectionMgr() const;
		/**
		@brief: 注册普通服务消息
		*/
		void				registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const void*)>& callback);
		/**
		@brief: 注册经网关服务转发客户端的服务消息
		*/
		void				registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const void*)>& callback);
		
		/**
		@brief: 获取客户端转发消息名字
		*/
		const std::string&	getForwardMessageName(uint32_t nMessageID);

		/**
		@brief: 设置全局的服务连接成功回调
		*/
		void				setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);
		/**
		@brief: 设置全局的服务连接断开回调
		*/
		void				setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);

		/**
		@brief: 设置ToGate消息回调
		*/
		void				setToGateMessageCallback(const std::function<void(uint64_t, const void*, uint16_t)>& callback);
		/**
		@brief: 设置ToGate广播消息回调
		*/
		void				setToGateBroadcastMessageCallback(const std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& callback);
		
		/**
		@brief: 获取服务调用器
		*/
		CServiceInvoker*	getServiceInvoker() const;
		/*
		@brief: 获取运行状态
		*/
		EServiceRunState	getRunState() const;
		/*
		@brief: 获取配置文件名
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: 逻辑发出退出
		*/
		void				doQuit();

		/**
		@brief: 添加消息的序列化器
		*/
		void				addServiceMessageSerializer(CMessageSerializer* pMessageSerializer);
		/**
		@brief: 设置某一个service所使用的序列化器
		*/
		void				setServiceMessageSerializer(uint32_t nServiceID, uint32_t nType);
		/**
		@brief: 设置forward消息的序列化器
		*/
		void				setForwardMessageSerializer(CMessageSerializer* pMessageSerializer);
		/**
		@brief: 获取某一个跟指定服务通讯时消息的序列化器
		*/
		CMessageSerializer*	getServiceMessageSerializer(uint32_t nServiceID) const;
		/**
		@brief: 获取某一个跟指定服务通讯时消息的序列化器类型
		*/
		uint32_t			getServiceMessageSerializerType(uint32_t nServiceID) const;

		/**
		@brief: 获取forward消息的序列化器
		*/
		CMessageSerializer*	getForwardMessageSerializer() const;
		/**
		@brief: 设置service_id转换器
		*/
		virtual	CServiceIDConverter*
							getServiceIDConverter() { return nullptr; }

		/**
		@brief: 根据节点名字获取节点id
		*/
		uint32_t			getServiceID(const std::string& szName) const;
		/**
		@brief: 根据服务类型取到该类型服务的所有服务id
		*/
		const std::set<uint32_t>&
							getServiceIDByType(const std::string& szName) const;
		/**
		@brief: 根据服务类型取到该类型服务的所有激活的服务id
		*/
		const std::vector<uint32_t>&
							getActiveServiceIDByType(const std::string& szName) const;

		uint32_t			getQPS() const;

		virtual void		release() = 0;

	protected:
		virtual bool		onInit() = 0;
		virtual void		onFrame() { }
		virtual void		onQuit() = 0;

	private:
		CCoreService*	m_pCoreService;
	};
}