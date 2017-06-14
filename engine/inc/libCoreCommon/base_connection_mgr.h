#pragma once

#include <list>
#include <map>
#include <functional>

#include "core_common.h"


namespace core
{
	class CBaseConnection;
	class CCoreConnectionMgr;
	class CCoreConnection;
	class CBaseConnectionFactory;
	/**
	@brief: �������ӹ����࣬��Ҫ������������Լ��������Ӹ��������
	*/
	class CBaseConnectionMgr :
		public base::noncopyable
	{
		friend class CBaseConnection;
		friend class CCoreConnectionMgr;
		friend class CCoreConnection;

	public:
		CBaseConnectionMgr();
		~CBaseConnectionMgr();

		/**
		@brief: ��������һ�����ӣ��첽��
		*/
		bool							connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize);
		/**
		@brief: ����һ������
		*/
		bool							listen(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize);

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
		void							onConnect(CBaseConnection* pBaseConnection);
		void							onDisconnect(CBaseConnection* pBaseConnection);
		void							onConnectFail(const std::string& szContext);

	private:
		CCoreConnectionMgr*							m_pCoreConnectionMgr;
		std::map<uint32_t, CBaseConnectionFactory*>	m_mapBaseConnectionFactory;
		std::function<void(CBaseConnection*)>		m_funConnect;
		std::function<void(CBaseConnection*)>		m_funDisconnect;
		std::function<void(const std::string&)>		m_funConnectFail;
	};
}