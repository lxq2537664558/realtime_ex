#pragma once
#include "core_common.h"
#include "promise.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

#define _GET_MESSAGE_ID(szMessageName) (base::hash(szMessageName.c_str()))

enum EResponseResultType
{
	eRRT_OK,
	eRRT_TIME_OUT,
	eRRT_ERROR,
};

namespace core
{

	struct SNodeBaseInfo
	{
		uint16_t	nID;
		std::string	szName;			// �ڵ�����
		std::string	szHost;			// ������IP
		uint16_t	nPort;			// 0��ʾ�ýڵ�û�м�����ַ
		uint32_t	nRecvBufSize;	// ���ջ�������С
		uint32_t	nSendBufSize;	// ���ͻ�������С
	};

	struct SServiceBaseInfo
	{
		uint16_t	nID;
		std::string	szName;			// ��������
		std::string	szType;			// �������ͣ�����gate, gas��
	};

	struct SClientSessionInfo
	{
		uint16_t	nGateServiceID;
		uint64_t	nSessionID;

		SClientSessionInfo(uint16_t	nGateServiceID, uint64_t nSessionID)
			: nGateServiceID(nGateServiceID), nSessionID(nSessionID)
		{}

		SClientSessionInfo()
			: nGateServiceID(0), nSessionID(0)
		{}
	};

	struct SServiceSessionInfo
	{
		uint16_t	nServiceID;
		uint64_t	nSessionID;

		SServiceSessionInfo(uint16_t nServiceID, uint64_t nSessionID)
			: nServiceID(nServiceID), nSessionID(nSessionID)
		{}

		SServiceSessionInfo()
			: nServiceID(0), nSessionID(0)
		{}
	};

	struct SActorSessionInfo
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
	};

	typedef std::function<void(uint64_t, const message_header*)> ClientCallback;	// �ͻ�����Ϣ����������

#pragma pack(push,1)
	struct gate_forward_cookice
	{
		uint16_t	nFromServiceID;
		uint16_t	nToServiceID;
		uint64_t	nSessionID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct actor_gate_forward_cookice
	{
		uint16_t	nFromServiceID;
		uint64_t	nToActorID;
		uint64_t	nSessionID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct gate_send_cookice
	{
		uint32_t	nToServiceID;
		uint64_t	nSessionID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};
	
	struct gate_broadcast_cookice :
		public message_header
	{
		uint32_t	nToServiceID;
		uint16_t	nSessionCount;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct request_cookice
	{
		uint64_t	nSessionID;
		uint16_t	nFromServiceID;
		uint16_t	nToServiceID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct response_cookice
	{
		uint64_t	nSessionID;
		uint8_t		nResult;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct actor_request_cookice
	{
		uint64_t	nSessionID;
		uint64_t	nFromActorID;
		uint64_t	nToActorID;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct actor_response_cookice
	{
		uint64_t	nToActorID;
		uint64_t	nSessionID;
		uint8_t		nResult;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

#pragma pack(pop)
}