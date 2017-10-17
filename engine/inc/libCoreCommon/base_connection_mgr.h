#pragma once

#include <functional>
#include <vector>

#include "libBaseCommon/noncopyable.h"

#include "core_common.h"

namespace core
{
	class CCoreApp;
	class CCoreService;
	class CBaseConnection;
	class CLogicMessageQueue;
	class CBaseConnectionMgrImpl;
	class CBaseConnectionFactory;
	/**
	@brief: 基础连接管理类，主要管理基础连接以及发起连接跟发起监听
	*/
	class __CORE_COMMON_API__ CBaseConnectionMgr :
		public base::noncopyable
	{
		friend class CCoreApp;
		friend class CCoreService;

	public:
		CBaseConnectionMgrImpl*	getBaseConnectionMgrImpl() const;
		/**
		@brief: 主动发起一个连接（异步）
		*/
		void					connect(const std::string& szHost, uint16_t nPort, const std::string& szType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, const MessageParser& messageParser);
		/**
		@brief: 发起一个监听
		*/
		bool					listen(const std::string& szHost, uint16_t nPort, bool bReusePort, const std::string& szType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser, ECoreConnectionType eCoreConnectionType = eCCT_Normal);
		/**
		@brief: 根据socket_id 发送数据
		*/
		void					send(uint64_t nSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize);
		/**
		@brief: 根据socket_id 发送数据
		*/
		void					send(uint64_t nSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);

		/**
		@brief: 设置某一个类型的连接创建工厂
		*/
		void					setBaseConnectionFactory(const std::string& szType, CBaseConnectionFactory* pBaseConnectionFactory);
		/**
		@brief: 获取某一个类型的连接创建工厂
		*/
		CBaseConnectionFactory*	getBaseConnectionFactory(const std::string& szType) const;
		/**
		@brief: 根据连接ID获取一个连接
		*/
		CBaseConnection*		getBaseConnectionBySocketID(uint64_t nID) const;
		/**
		@brief: 根据连接的类型枚举连接对象,回调函数返回false停止枚举
		*/
		void					enumBaseConnection(const std::string& szType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const;
		/**
		@brief: 根据连接的类型来获取所有基于这个连接类创建的连接数量
		*/
		uint32_t				getBaseConnectionCount(const std::string& szType) const;
		/**
		@brief: 根据链接的类型来广播消息
		*/
		void					broadcast(const std::string& szType, uint8_t nMessageType, const void* pData, uint16_t nDataSize, const std::vector<uint64_t>* vecExcludeID);
		/**
		@brief: 向一批特定链接广播消息
		*/
		void					broadcast(const std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nDataSize);
		/**
		@brief: 添加全局的连接成功回调
		*/
		void					addConnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
		/**
		@brief: 删除全局的连接成功回调
		*/
		void					delConnectCallback(const std::string& szKey);
		/**
		@brief: 添加全局的连接断开回调
		*/
		void					addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
		/**
		@brief: 删除全局的连接断开回调
		*/
		void					delDisconnectCallback(const std::string& szKey);
		/**
		@brief: 添加全局的主动发起连接被失败回调
		*/
		void					addConnectFailCallback(const std::string& szKey, const std::function<void(const std::string&)>& callback);
		/**
		@brief: 删除全局的主动发起连接被失败回调
		*/
		void					delConnectFailCallback(const std::string& szKey);

	private:
		CBaseConnectionMgr(CLogicMessageQueue* pMessageQueue);
		~CBaseConnectionMgr();

	private:
		CBaseConnectionMgrImpl*	m_pBaseConnectionMgrImpl;
	};
}