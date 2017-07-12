#pragma once

#include "core_common.h"

namespace core
{

	class CBaseConnectionMgr;
	class CCoreConnection;

	/**
	@brief: 连接基础类，框架的所有连接都继承于这个类
	*/
	class CBaseConnection
	{
		friend class CBaseConnectionMgr;
		friend class CCoreConnection;

	public:
		CBaseConnection();
		virtual ~CBaseConnection();

		virtual bool			init(uint32_t nType, const std::string& szContext);
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
		virtual void			onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pContext) = 0;
		/**
		@brief: 获取连接类型
		*/
		uint32_t				getType() const;
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
		void					shutdown(base::ENetConnecterCloseType eType, const std::string& szMsg);
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

	private:
		void					onHeartbeat(uint64_t nContext);

	private:
		uint32_t	m_nType;
		uint64_t	m_nID;
		SNetAddr	m_sLocalAddr;
		SNetAddr	m_sRemoteAddr;
	};
}