#pragma once

#include "core_common.h"

#include "libBaseNetwork/network.h"
#include "libBaseCommon/noncopyable.h"

namespace core
{
	class CCoreConnection;
	class CBaseConnectionMgrImpl;

	/**
	@brief: 连接基础类，框架的所有连接都继承于这个类
	*/
	class __CORE_COMMON_API__ CBaseConnection :
		public base::noncopyable
	{
		friend class CBaseConnectionMgrImpl;

	public:
		CBaseConnection();
		virtual ~CBaseConnection();

		/**
		@brief: 释放对象
		*/
		virtual void			release() = 0;
		/**
		@brief: 连接成功回调
		*/
		virtual void			onConnect() = 0;
		/**
		@brief: 连接断开回调
		*/
		virtual void			onDisconnect() = 0;
		/**
		@brief: 消息回调
		*/
		virtual void			onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize) = 0;
		/**
		@brief: 获取连接类型
		*/
		const std::string&		getType() const;
		/**
		@brief: 获取连接模式，主动连接，被动连接
		*/
		base::ENetConnecterMode	getMode() const;
		/**
		@brief: 发送消息
		*/
		void					send(uint8_t nMessageType, const void* pData, uint16_t nSize);
		/**
		@brief: 发送消息，可以带上额外数据
		*/
		void					send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);
		/**
		@brief: 关闭连接
		*/
		void					shutdown(bool bForce, const std::string& szMsg);
		/**
		@brief: 消息解析器
		*/
		void					setMessageParser(MessageParser parser);
		/**
		@brief: 打开关闭心跳
		*/
		void					enableHeartbeat(bool bEnable);
		/**
		@brief: 获取连接ID
		*/
		uint64_t				getID() const;
		/**
		@brief: 获取本地地址
		*/
		const SNetAddr&			getLocalAddr() const;
		/**
		@brief: 获取远程地址
		*/
		const SNetAddr&			getRemoteAddr() const;
		/**
		@brief: 获取上下文
		*/
		const std::string&		getContext() const;
		/**
		@brief: 设置session_id
		*/
		void					setSessionID(uint64_t nSessionID);

	private:
		CCoreConnection*	m_pCoreConnection;
	};
}