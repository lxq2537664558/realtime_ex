#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"

#include "load_balance_policy.h"

namespace core
{
	/**
	@brief: 集群服务调用器
	*/
	class CClusterInvoker :
		public base::CSingleton<CClusterInvoker>
	{
	public:
		CClusterInvoker();
		~CClusterInvoker();

		bool				init();

		/**
		@brief: 调用指定远程服务的消息，远程服务不用返回消息
		*/
		bool				invok(const std::string& szServiceName, uint16_t nMessageFormat, const message_header* pData);
		/**
		@brief: 调用指定远程服务的消息，需要提供远程服务消息返回的响应函数回调
		*/
		bool				invok_r(const std::string& szServiceName, uint16_t nMessageFormat, const message_header* pData, InvokeCallback callback, uint64_t nContext = 0);
		/**
		@brief: 通过消息ID调用远程服务，需要提供负载均衡器，远程服务不用返回消息
		*/
		bool				invok(uint16_t nMessageFormat, const message_header* pData, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam);
		/**
		@brief: 通过消息ID调用远程服务，需要提供负载均衡器，需要提供远程服务消息返回的响应函数回调
		*/
		bool				invok_r(uint16_t nMessageFormat, const message_header* pData, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam, InvokeCallback callback, uint64_t nContext = 0);
		/**
		@brief: 响应远程服务的调用，发送响应消息
		*/
		void				response(uint16_t nMessageFormat, const message_header* pData);
		/**
		@brief: 响应远程服务的调用，发送响应消息，这个需要用户自己制定服务session，主要用于不能立即响应，需要等待其他信息的消息
		*/
		void				response(const SServiceSessionInfo& sServiceSessionInfo, uint16_t nMessageFormat, const message_header* pData);
		/**
		@brief: 获取当前服务session信息
		*/
		SServiceSessionInfo	getServiceSessionInfo() const;
		//=============================上面是对集群内部服务之间的消息发送函数=========================//

		//=================================下面是对客户端的消息处理函数==============================//
		/**
		@brief: 向客户端发送消息，客户端ID由SGateSessionInfo来表述
		*/
		bool				send(const SClientSessionInfo& sGateSessionInfo, uint16_t nMessageFormat, const message_header* pData);
		/**
		@brief: 向一批客户端广播消息，客户端ID由std::vector<SGateSessionInfo>来表述
		*/
		bool				broadcast(const std::vector<SClientSessionInfo>& vecGateSessionInfo, uint16_t nMessageFormat, const message_header* pData);
		/**
		@brief: 网关服务转发客户端消息
		*/
		bool				foward(uint64_t nSessionID, uint16_t nMessageFormat, const message_header* pData, ILoadBalancePolicy* pLoadBalancePolicy, uint64_t nLoadBalanceParam);
	};
}