#pragma once
#include "libCoreCommon/core_common.h"
#include "libCoreServiceKit/promise.h"

#include "message_ptr.h"

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

#define _REMOTE_ACTOR_BIT 48

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
		uint16_t	nGateNodeID;
		uint64_t	nSessionID;

		SClientSessionInfo(uint16_t	nGateNodeID, uint64_t nSessionID)
			: nGateNodeID(nGateNodeID), nSessionID(nSessionID)
		{}
	};

	struct SServiceSessionInfo
	{
		uint16_t	nServiceID;
		uint64_t	nSessionID;
	};

	struct SActorSessionInfo
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
	};

	typedef core::CMessagePtr<message_header>	CMessage;
	
	typedef std::function<void(uint64_t, const message_header*)>					ClientCallback;				// �ͻ�����Ϣ����������
	typedef std::function<bool(uint64_t, uint16_t, uint8_t, const void*, uint16_t)>	GlobalBeforeFilter;			// ȫ�ֵ���Ϣ����������
	typedef std::function<void(uint64_t, uint16_t, uint8_t, const void*, uint16_t)>	GlobalAfterFilter;			// ȫ�ֵ���Ϣ����������
	
#pragma pack(push,1)
	struct gate_forward_cookice
	{
		uint64_t nActorID;
		uint64_t nSessionID;
	};

	struct gate_send_cookice
	{
		uint64_t nSessionID;
	};
	
	struct gate_broadcast_cookice :
		public message_header
	{
		uint16_t nCount;
	};

	struct request_cookice
	{
		uint64_t nSessionID;
		uint64_t nFromID;
		uint64_t nToID;
	};

	struct response_cookice
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
		uint8_t		nResult;
	};

#pragma pack(pop)
}