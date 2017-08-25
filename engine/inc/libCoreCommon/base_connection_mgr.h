#pragma once

#include <functional>

#include "libBaseCommon/noncopyable.h"

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
		void							connect(const std::string& szHost, uint16_t nPort, const std::string& szType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, const MessageParser& messageParser);
		/**
		@brief: ֱ������Ŀ��ڵ�
		*/
		void							connect_n(const std::string& szHost, uint16_t nPort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize);
		/**
		@brief: ����һ������
		*/
		void							listen(const std::string& szHost, uint16_t nPort, bool bReusePort, const std::string& szType, const std::string& szContext, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, MessageParser messageParser, ECoreConnectionType eCoreConnectionType = eCCT_Normal);

		/**
		@brief: ����ĳһ�����͵����Ӵ�������
		*/
		void							setBaseConnectionFactory(const std::string& szType, CBaseConnectionFactory* pBaseConnectionFactory);
		/**
		@brief: ��ȡĳһ�����͵����Ӵ�������
		*/
		CBaseConnectionFactory*			getBaseConnectionFactory(const std::string& szType) const;
		/**
		@brief: ��������ID��ȡһ������
		*/
		CBaseConnection*				getBaseConnectionBySocketID(uint64_t nID) const;
		/**
		@brief: �������ӵ�����ö�����Ӷ���,�ص���������falseֹͣö��
		*/
		void							enumBaseConnection(const std::string& szType, const std::function<bool(CBaseConnection* pBaseConnection)>& callback) const;
		/**
		@brief: �������ӵ���������ȡ���л�����������ഴ������������
		*/
		uint32_t						getBaseConnectionCount(const std::string& szType) const;
		/**
		@brief: �������ӵ��������㲥��Ϣ
		*/
		void							broadcast(const std::string& szType, uint8_t nMessageType, const void* pData, uint16_t nDataSize, const std::vector<uint64_t>* vecExcludeID);
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
		void							addDisconnectCallback(const std::string& szKey, const std::function<void(CBaseConnection*)>& callback);
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