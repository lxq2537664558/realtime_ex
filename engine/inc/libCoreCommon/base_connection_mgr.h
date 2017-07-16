#pragma once

#include <list>
#include <map>
#include <vector>
#include <functional>

#include "core_common.h"

namespace core
{
	class CBaseConnection;
	class CLogicRunnable;
	class CBaseConnectionFactory;
	/**
	@brief: �������ӹ����࣬��Ҫ������������Լ��������Ӹ��������
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
		@brief: ��������һ�����ӣ��첽��
		*/
		void							connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, const MessageParser& messageParser);
		/**
		@brief: ֱ������Ŀ�����
		*/
		void							connect_s(const std::string& szHost, uint16_t nPort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize);
		/**
		@brief: ����һ������
		*/
		void							listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser);

		/**
		@brief: ����ĳһ�����͵����Ӵ�������
		*/
		void							setBaseConnectionFactory(uint32_t nType, CBaseConnectionFactory* pBaseConnectionFactory);
		/**
		@brief: ��ȡĳһ�����͵����Ӵ�������
		*/
		CBaseConnectionFactory*			getBaseConnectionFactory(uint32_t nType) const;
		/**
		@brief: ��������ID��ȡһ������
		*/
		CBaseConnection*				getBaseConnectionByID(uint64_t nID) const;
		/**
		@brief: �������ӵ���������ȡ���л�����������ഴ�������Ӷ���
		*/
		std::vector<CBaseConnection*>	getBaseConnection(uint32_t nType) const;
		/**
		@brief: �������ӵ���������ȡ���л�����������ഴ������������
		*/
		uint32_t						getBaseConnectionCount(uint32_t nType) const;
		/**
		@brief: �������ӵ��������㲥��Ϣ
		*/
		void							broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nSize, const std::vector<uint64_t>* vecExcludeID);
		/**
		@brief: ��һ���ض����ӹ㲥��Ϣ
		*/
		void							broadcast(std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize);
		/**
		@brief: ����ȫ�ֵ����ӳɹ��ص�
		*/
		void							setConnectCallback(std::function<void(CBaseConnection*)> funConnect);
		/**
		@brief: ����ȫ�ֵ����ӶϿ��ص�
		*/
		void							setDisconnectCallback(std::function<void(CBaseConnection*)> funDisconnect);
		/**
		@brief: ����ȫ�ֵ������������ӱ�ʧ�ܻص�
		*/
		void							setConnectFailCallback(std::function<void(const std::string&)> funConnectFail);

	private:
		void							onConnect(uint64_t nSocketID, const std::string& szContext, uint32_t nType, const SNetAddr& sLocalAddr, const SNetAddr& sRemoteAddr);
		void							onDisconnect(uint64_t nSocketID);
		void							onConnectFail(const std::string& szContext);

	private:
		std::map<uint64_t, CBaseConnection*>						m_mapBaseConnectionByID;
		std::map<uint32_t, std::map<uint64_t, CBaseConnection*>>	m_mapBaseConnectionByType;
		std::map<uint32_t, CBaseConnectionFactory*>					m_mapBaseConnectionFactory;
		std::function<void(CBaseConnection*)>						m_funConnect;
		std::function<void(CBaseConnection*)>						m_funDisconnect;
		std::function<void(const std::string&)>						m_funConnectFail;
	};
}