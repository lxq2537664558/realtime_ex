#pragma once

#include "core_common.h"

namespace core
{

	class CBaseConnectionMgr;
	class CCoreConnection;

	/**
	@brief: ���ӻ����࣬��ܵ��������Ӷ��̳��������
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
		@brief: �ͷŶ���
		*/
		virtual void			release() = 0;
		/**
		@brief: ���ӳɹ��ص�
		*/
		virtual void			onConnect() = 0;
		/**
		@brief: ���ӶϿ��ص�
		*/
		virtual void			onDisconnect() = 0;
		/**
		@brief: ��Ϣ�ص�
		*/
		virtual void			onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pContext) = 0;
		/**
		@brief: ��ȡ��������
		*/
		uint32_t				getType() const;
		/**
		@brief: ������Ϣ
		*/
		void					send(uint8_t nMessageType, const void* pData, uint16_t nSize);
		/**
		@brief: ������Ϣ�����Դ��϶�������
		*/
		void					send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);
		/**
		@brief: �ر�����
		*/
		void					shutdown(base::ENetConnecterCloseType eType, const std::string& szMsg);
		/**
		@brief: ��Ϣ������
		*/
		void					setMessageParser(MessageParser parser);
		/**
		@brief: �򿪹ر�����
		*/
		void					enableHeartbeat(bool bEnable);
		/**
		@brief: ��ȡ����ID
		*/
		uint64_t				getID() const;
		/**
		@brief: ��ȡ���ص�ַ
		*/
		const SNetAddr&			getLocalAddr() const;
		/**
		@brief: ��ȡԶ�̵�ַ
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