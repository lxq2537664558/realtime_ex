#pragma once
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 22
#endif
#else
#include <netinet/in.h>
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
#endif

struct SNetAddr
{
	char		szHost[INET_ADDRSTRLEN];
	uint16_t	nPort;

	SNetAddr(const char* szIP, uint16_t nPort)
		: nPort(nPort)
	{
		strncpy(this->szHost, szIP, INET_ADDRSTRLEN);
	}

	SNetAddr()
		: nPort(0)
	{
		strncpy(this->szHost, "0.0.0.0", INET_ADDRSTRLEN);
	}

	bool operator == (const SNetAddr& rhs) const
	{
		return strncmp(this->szHost, rhs.szHost, INET_ADDRSTRLEN) == 0 && this->nPort == rhs.nPort;
	}

	bool isAnyIP() const
	{
		if (memcmp(this->szHost, "0.0.0.0", strnlen("0.0.0.0", INET_ADDRSTRLEN - 1)) == 0)
			return true;

		return false;
	}
};

#ifdef _WIN32
#pragma warning(pop)
#endif

namespace base
{

#ifdef _WIN32

#	ifdef __BUILD_BASE_NETWORK_DLL__
#		define __BASE_NETWORK_API__ __declspec(dllexport)
#	else
#		define __BASE_NETWORK_API__ __declspec(dllimport)
#	endif

#else

#	define __BASE_NETWORK_API__

#endif

	enum ENetConnecterState
	{
		eNCS_Connecting,		// ���ӽ�����
		eNCS_Connected,			// �������
		eNCS_Disconnecting,		// �Ͽ����ӽ�����
		eNCS_Disconnected,		// �Ͽ��������

		eNCS_Unknown
	};

	enum ENetConnecterMode
	{
		eNCM_Initiative,	// ��������
		eNCM_Passive,		// ��������

		eNCM_Unknown
	};

	class INetConnecter;
	/**
	@brief: �������Ӵ�����
	���ӵĳ�ʱ������Ҫ�߼���ȥ�����������һֱ���Ͳ��ˣ����������Ѿ��ǶϿ������У�ȷ�ٳ��޷�ȫ���Ͽ����ܿ���������һֱ���Ͳ��˵��µģ����ʱ��Ӧ���ߵ�����
	*/
	class INetConnecterHandler
	{
	protected:
		INetConnecter* m_pNetConnecter;	// ���Ӷ���

	public:
		INetConnecterHandler() : m_pNetConnecter(nullptr) {}
		virtual ~INetConnecterHandler() {}

		/**
		@brief: �������Ӵ�����
		*/
		void				setNetConnecter(INetConnecter* pConnecter) { this->m_pNetConnecter = pConnecter; }
		/**
		@brief: ��ȡ���Ӵ�����
		*/
		INetConnecter*		getNetConnecter() const { return this->m_pNetConnecter; }
		/**
		@brief: �������ݣ����ں�˻���֡�ķ��������Կ���cache�ķ�ʽ���������ط����������а����Կ���cache�����а�ֱ�ӷ��ͱȽϺã��������ط�������ò��û����¼������ķ�ʽ
		*/
		inline void			send(const void* pData, uint32_t nSize, bool bCache);
		/**
		@brief: ���ݵ���ص������pData�����ڻص������ѵ�������ص�������pData����Ч�Բ���֤
		*/
		virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize) = 0;
		/**
		@brief: ���ݷ�����ɻص��������Ƿ��͵���socket�����У����������͵��Զˣ�����˵���Է�Ӧ�ò㴦������Ҫ׼ȷ�ķ������ݱ��Է����յ�������ҪӦ�ò�ȥ����
		*/
		virtual void		onSendComplete(uint32_t nSize) = 0;
		/**
		@brief: ������ɻص�
		*/
		virtual void		onConnect() = 0;
		/**
		@brief: ���ӶϿ��ص�
		*/
		virtual void		onDisconnect() = 0;
		/**
		@brief: ��������ʧ�ܣ��ڳٳ�δ�յ�syn����Ӧ������£�rst��Ҳû�У���tcp����ָ�����ˣ����ʱ���ڸ���ƽ̨��һ�£��������ٴ�ʮ�룬����������ӹ������ϵĵ�ַ�ϲ�����Լ�����ʱ��������������onConnectFail�ص�
		*/
		virtual void		onConnectFail() = 0;
	};

	class INetAccepter;
	/**
	@brief: �������Ӽ���������
	*/
	class INetAccepterHandler
	{
	protected:
		INetAccepter* m_pNetAccepter;	// ��������

	public:
		INetAccepterHandler() : m_pNetAccepter(nullptr) {}
		virtual ~INetAccepterHandler() {}

		/**
		@brief: ���ü���������
		*/
		void                          setNetAccepter(INetAccepter* pAccepter) { this->m_pNetAccepter = pAccepter; }
		/**
		@brief: ��ȡ����������
		*/
		INetAccepter*                 getNetAccepter() const { return this->m_pNetAccepter; }
		/**
		@brief: �����Ķ˿������ӽ��������Ż�ȡ��Ӧ���ӵ����Ӵ�����
		*/
		virtual INetConnecterHandler* onAccept(INetConnecter* pNetConnecter) = 0;
	};

	/**
	@brief: ����������ӿ�
	*/
	class INetAccepter
	{
	public:
		INetAccepter() { }
		virtual ~INetAccepter() {}

		/**
		@brief: ���ü�����������
		*/
		virtual void            setHandler(INetAccepterHandler* pHandler) = 0;
		/**
		@brief: ��ȡ������ַ
		*/
		virtual const SNetAddr& getListenAddr() const = 0;
		/**
		@brief: �رռ�����
		*/
		virtual void            shutdown() = 0;
	};

	/**
	@brief: �����������ӿ�
	*/
	class INetConnecter
	{
	public:
		INetConnecter() { }
		virtual ~INetConnecter() { }

		/**
		@brief: ��������, ����ǻ��淢�͵�ģʽ����ֱ�Ӱ����ݿ����������Ļ����У���Ȼ�������ŷ��ͣ����Ͳ��˲Ż���
		*/
		virtual bool				send(const void* pData, uint32_t nSize, bool bCache) = 0;
		/**
		@brief: �ر�����
		*/
		virtual void				shutdown(bool bForce, const char* szFormat, ...) = 0;
		/**
		@brief: �������Ӵ�����
		*/
		virtual void				setHandler(INetConnecterHandler* pHandler) = 0;
		/**
		@brief: ��ȡ���ӱ��ص�ַ
		*/
		virtual const SNetAddr&		getLocalAddr() const = 0;
		/**
		@brief: ��ȡ����Զ�˵�ַ
		*/
		virtual const SNetAddr&		getRemoteAddr() const = 0;
		/**
		@brief: ��ȡ������ģʽ
		*/
		virtual ENetConnecterMode	getConnecterMode() const = 0;
		/**
		@brief: ��ȡ������״̬
		*/
		virtual ENetConnecterState	getConnecterState() const = 0;
		/**
		@brief: ��ȡ���ͻ������е����ݴ�С�����һ�㻺��һ֡���ݣ����һ֡���޷����������ݻ��ǻỺ��
		*/
		virtual	uint32_t			getSendDataSize() const = 0;
		/**
		@brief: ��ȡ���ջ�������С
		*/
		virtual	uint32_t			getRecvDataSize() const = 0;
		/**
		@brief: �����Ƿ�����tcpЭ��ջ��nodelay�㷨
		*/
		virtual bool				setNoDelay(bool bEnable) = 0;
	};

	/**
	@brief: �����¼�ѭ����
	*/
	class INetEventLoop
	{
	public:
		INetEventLoop() { }
		virtual ~INetEventLoop() {}

		/**
		@brief: ��ʼ��
		*/
		virtual bool	init(uint32_t nMaxSocketCount) = 0;
		/**
		@brief: ����һ������
		*/
		virtual bool	listen(const SNetAddr& netAddr, bool bReusePort, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetAccepterHandler* pHandler) = 0;
		/**
		@brief: ����һ������
		*/
		virtual bool	connect(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetConnecterHandler* pHandler) = 0;
		/**
		@brief: �ƶ������¼�ѭ����
		*/
		virtual void	update(uint32_t nTime) = 0;
		/**
		@brief: ���¼�ѭ���ȴ��л��ѵȴ�
		*/
		virtual void	wakeup() = 0;
		/**
		@brief: �ͷ������¼�ѭ����
		*/
		virtual void	release() = 0;
	};

	void INetConnecterHandler::send(const void* pData, uint32_t nSize, bool bCache)
	{
		if (this->m_pNetConnecter == nullptr)
			return;
		
		this->m_pNetConnecter->send(pData, nSize, bCache);
	}

	__BASE_NETWORK_API__ bool			startupNetwork();
	__BASE_NETWORK_API__ void			cleanupNetwork();

	__BASE_NETWORK_API__ INetEventLoop*	createNetEventLoop();
}