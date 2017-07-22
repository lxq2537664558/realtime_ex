#pragma once

#include <list>
#include <map>
#include <vector>
#include <functional>

#include "core_common.h"

#include "libBaseCommon\noncopyable.h"
#include "libBaseNetwork\network.h"

namespace core
{
	class CBaseConnection;
	class CLogicRunnable;
	class CBaseConnectionFactory;
	/**
	@brief: 基础连接管理类，主要管理基础连接以及发起连接跟发起监听
	*/
	class CBaseConnectionMgr :
		public base::noncopyable
	{
		friend class CBaseConnection;
		friend class CLogicRunnable;

	public:
		CBaseConnectionMgr();
		~CBaseConnectionMgr();

		bool							init();
		/**
		@brief: 主动发起一个连接（异步）
		*/
		void							connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, const MessageParser& messageParser);
		/**
		@brief: 直接连接目标节点
		*/
		void							connect_n(const std::string& szHost, uint16_t nPort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize);
		/**
		@brief: 发起一个监听
		*/
		void							listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser);

		/**
		@brief: 设置某一个类型的连接创建工厂
		*/
		void							setBaseConnectionFactory(uint32_t nType, CBaseConnectionFactory* pBaseConnectionFactory);
		/**
		@brief: 获取某一个类型的连接创建工厂
		*/
		CBaseConnectionFactory*			getBaseConnectionFactory(uint32_t nType) const;
		/**
		@brief: 根据连接ID获取一个连接
		*/
		CBaseConnection*				getBaseConnectionByID(uint64_t nID) const;
		/**
		@brief: 根据连接的类型来获取所有基于这个连接类创建的连接对象
		*/
		std::vector<CBaseConnection*>	getBaseConnection(uint32_t nType) const;
		/**
		@brief: 根据连接的类型来获取所有基于这个连接类创建的连接数量
		*/
		uint32_t						getBaseConnectionCount(uint32_t nType) const;
		/**
		@brief: 根据链接的类型来广播消息
		*/
		void							broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nSize, const std::vector<uint64_t>* vecExcludeID);
		/**
		@brief: 向一批特定链接广播消息
		*/
		void							broadcast(std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize);
		/**
		@brief: 添加全局的连接成功回调
		*/
		void							addConnectCallback(const std::string& szKey, std::function<void(CBaseConnection*)> callback);
		/**
		@brief: 删除全局的连接成功回调
		*/
		void							delConnectCallback(const std::string& szKey);
		/**
		@brief: 添加全局的连接断开回调
		*/
		void							addDisconnectCallback(const std::string& szKey, std::function<void(CBaseConnection*)> callback);/**
		/**
		@brief: 删除全局的连接断开回调
		*/
		void							delDisconnectCallback(const std::string& szKey);
		/**
		@brief: 添加全局的主动发起连接被失败回调
		*/
		void							addConnectFailCallback(const std::string& szKey, std::function<void(const std::string&)> callback);
		/**
		@brief: 删除全局的主动发起连接被失败回调
		*/
		void							delConnectFailCallback(const std::string& szKey);

	private:
		void							onConnect(uint64_t nSocketID, const std::string& szContext, uint32_t nType, const SNetAddr& sLocalAddr, const SNetAddr& sRemoteAddr);
		void							onDisconnect(uint64_t nSocketID);
		void							onConnectFail(const std::string& szContext);

	private:
		std::map<uint64_t, CBaseConnection*>							m_mapBaseConnectionByID;
		std::map<uint32_t, std::map<uint64_t, CBaseConnection*>>		m_mapBaseConnectionByType;
		std::map<uint32_t, CBaseConnectionFactory*>						m_mapBaseConnectionFactory;

		std::map<std::string, std::function<void(CBaseConnection*)>>	m_mapConnectCalback;
		std::map<std::string, std::function<void(CBaseConnection*)>>	m_mapDisconnectCallback;
		std::map<std::string, std::function<void(const std::string&)>>	m_mapConnectFailCallback;
	};
}