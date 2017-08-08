#pragma once

#include "core_common.h"

#include "libBaseNetwork/network.h"
#include "libBaseCommon/noncopyable.h"

namespace core
{
	class CCoreConnection;
	class CBaseConnectionMgrImpl;

	/**
	@brief: ���ӻ����࣬��ܵ��������Ӷ��̳��������
	*/
	class __CORE_COMMON_API__ CBaseConnection :
		public base::noncopyable
	{
		friend class CBaseConnectionMgrImpl;

	public:
		CBaseConnection();
		virtual ~CBaseConnection();

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
		virtual void			onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize) = 0;
		/**
		@brief: ��ȡ��������
		*/
		const std::string&		getType() const;
		/**
		@brief: ��ȡ����ģʽ���������ӣ���������
		*/
		base::ENetConnecterMode	getMode() const;
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
		void					shutdown(bool bForce, const std::string& szMsg);
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
		/**
		@brief: ��ȡ������
		*/
		const std::string&		getContext() const;
		/**
		@brief: ����session_id
		*/
		void					setSessionID(uint64_t nSessionID);

	private:
		CCoreConnection*	m_pCoreConnection;
	};
}