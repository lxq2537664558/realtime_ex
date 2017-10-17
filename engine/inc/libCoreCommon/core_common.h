#pragma once

#include <functional>
#include <memory>

#ifdef _WIN32

#	ifdef __BUILD_CORE_COMMON_DLL__
#		define __CORE_COMMON_API__ __declspec(dllexport)
#	else
#		define __CORE_COMMON_API__ __declspec(dllimport)
#	endif

#else

#	define __CORE_COMMON_API__

#endif

#define _GET_MESSAGE_ID(szMessageName) (base::function_util::hash(szMessageName.c_str()))
#define _GET_MESSAGE_ID_EX(szMessageName) (base::function_util::hash(szMessageName))

enum EResponseResultType
{
	eRRT_OK			= 1,
	eRRT_TIME_OUT	= 2,
	eRRT_ERROR		= 3,
};

#define _INVALID_SOCKET_ID			-1

enum EMessageType
{
	eMT_NONE				= 0,
	eMT_HEARTBEAT			= 1,		// ����֮��������Ϣ
	eMT_SYSTEM				= 2,		// ����֮���ϵͳ��Ϣ
	eMT_REQUEST				= 3,		// ����֮���������Ϣ
	eMT_RESPONSE			= 4,		// ����֮�����Ӧ��Ϣ
	eMT_GATE_FORWARD		= 5,		// �ͻ���ͨ�����ط���ת��������������Ϣ
	eMT_TO_GATE				= 6,		// ��������ͨ�����ط���ת���ͻ�����Ϣ
	eMT_TO_GATE_BROADCAST	= 7,		// ��������ͨ�����ط���㲥�ͻ�����Ϣ
	eMT_CLIENT				= 8,		// �ͻ�����Ϣ
};

enum ECoreConnectionType
{
	eCCT_Normal,
	eCCT_Websocket,
};

#define _MAX_MESSAGE_NAME_LEN 128

namespace core
{
#pragma pack(push,1)
	// ��Ϣͷ
	struct message_header
	{
		uint16_t	nMessageSize;	// ������Ϣͷ��
		uint32_t	nMessageID;

		message_header(uint32_t nMessageID) : nMessageID(nMessageID) { }
		message_header() {}
	};

#pragma pack(pop)

	typedef std::function<int32_t(const char*, uint32_t, uint8_t&)>	MessageParser;	// ԭ����Ϣ

	struct SNodeBaseInfo
	{
		uint32_t	nID;
		std::string	szName;			// �ڵ�����
		std::string	szHost;			// ������IP
		uint16_t	nPort;			// 0��ʾ�ýڵ�û�м�����ַ
		uint32_t	nRecvBufSize;	// ���ջ�������С
		uint32_t	nSendBufSize;	// ���ͻ�������С
	};

	struct SServiceBaseInfo
	{
		uint32_t	nID;
		std::string	szName;			// ��������
		std::string	szType;			// �������ͣ�����gate, gas��
	};

	struct SClientSessionInfo
	{
		uint32_t	nGateServiceID;
		uint64_t	nSessionID;
	};

	struct SSessionInfo
	{
		uint32_t	nFromServiceID;
		uint64_t	nSessionID;
	};
}

inline int32_t default_client_message_parser(const char* pData, uint32_t nSize, uint8_t& nMessageType)
{
	if (nSize < sizeof(core::message_header))
		return 0;

	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	if (pHeader->nMessageSize < sizeof(core::message_header))
		return -1;

	// ������������Ϣ
	if (nSize < pHeader->nMessageSize)
		return 0;

	nMessageType = eMT_CLIENT;

	return pHeader->nMessageSize;
}