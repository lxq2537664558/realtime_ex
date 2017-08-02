#pragma once

#include <functional>

#include "libBaseCommon\noncopyable.h"

#include "core_common.h"

namespace core
{
	class CBaseConnection;
	class CBaseConnectionFactory;
	/**
	@brief: �������ӹ����࣬��Ҫ������������Լ��������Ӹ��������
	*/
	class __CORE_COMMON_API__ CBaseConnectionMgr :
		public base::noncopyable
	{
	public:
		CBaseConnectionMgr();
		~CBaseConnectionMgr();

		/**
		@brief: ��������һ�����ӣ��첽��
		*/
		void							connect(const std::string& szHost, uint16_t nPort, uint32_t nType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, const MessageParser& messageParser);
		/**
		@brief: ֱ������Ŀ��ڵ�
		*/
		void							connect_n(const std::string& szHost, uint16_t nPort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize);
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
		CBaseConnection*				getBaseConnectionBySocketID(uint64_t nID) const;
		/**
		@brief: ���ݷ���ID��ȡһ������
		*/
		CBaseConnection*				getBaseConnectionByServiceID(uint32_t nID) const;
		/**
		@brief: �������ӵ�����ö�����Ӷ���,�ص���������falseֹͣö��
		*/
		void							enumBaseConnection(uint32_t nType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const;
		/**
		@brief: �������ӵ���������ȡ���л�����������ഴ������������
		*/
		uint32_t						getBaseConnectionCount(uint32_t nType) const;
		/**
		@brief: �������ӵ��������㲥��Ϣ
		*/
		void							broadcast(uint32_t nType, uint8_t nMessageType, const void* pData, uint16_t nDataSize, const std::vector<uint64_t>* vecExcludeID);
		/**
		@brief: ��һ���ض����ӹ㲥��Ϣ
		*/
		void							broadcast(const std::vector<uint64_t>& vecSocketID, uint8_t nMessageType, const void* pData, uint16_t nDataSize);
		/**
		@brief: ���ȫ�ֵ����ӳɹ��ص�
		*/
		void							addConnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
		/**
		@brief: ɾ��ȫ�ֵ����ӳɹ��ص�
		*/
		void							delConnectCallback(const std::string& szKey);
		/**
		@brief: ���ȫ�ֵ����ӶϿ��ص�
		*/
		void							addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);/**
		/**
		@brief: ɾ��ȫ�ֵ����ӶϿ��ص�
		*/
		void							delDisconnectCallback(const std::string& szKey);
		/**
		@brief: ���ȫ�ֵ������������ӱ�ʧ�ܻص�
		*/
		void							addConnectFailCallback(const std::string& szKey, const std::function<void(const std::string&)>& callback);
		/**
		@brief: ɾ��ȫ�ֵ������������ӱ�ʧ�ܻص�
		*/
		void							delConnectFailCallback(const std::string& szKey);
	};
}