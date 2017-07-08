#pragma once

#include <functional>

#include "libBaseNetwork/network.h"

#include "ticker.h"

namespace core
{
#pragma pack(push,1)
	// ��Ϣͷ
	struct message_header
	{
		uint16_t	nMessageSize;	// ������Ϣͷ��
		uint16_t	nMessageID;

		message_header(uint16_t nMessageID) : nMessageID(nMessageID) { }
		message_header() {}
	};

#pragma pack(pop)
}

#define message_begin(MessageName, nMessageID) \
class MessageName : public core::message_header\
{\
public:\
	MessageName() : core::message_header(nMessageID) { nMessageSize = sizeof(MessageName); }\
	static  uint16_t	getMessageID() { return nMessageID; }\
	static  const char*	getMessageName() { return #MessageName; }

#define message_end };

#define pack_begin(writeBuf)\
	writeBuf.clear();\
	writeBuf.write(this, sizeof(core::message_header));

#define pack_end(writeBuf)\
	do\
	{\
		uint16_t nPos = (uint16_t)writeBuf.getCurSize(); \
		writeBuf.seek(base::eBST_Begin, 0); \
		writeBuf.write(nPos); \
		writeBuf.seek(base::eBST_Begin, nPos);\
	} while(0)

#define unpack_begin(buf, size)\
	base::CReadBuf readBuf;\
	readBuf.init(buf, size);\
	readBuf.read(this, sizeof(core::message_header));

#define unpack_end()



#define _INVALID_SOCKET_ID			-1

enum EMessageType
{
	eMT_HEARTBEAT			= 1,		// ����֮��������Ϣ
	eMT_SYSTEM				= 2,		// ����֮���ϵͳ��Ϣ
	eMT_REQUEST				= 3,		// ����֮���������Ϣ
	eMT_RESPONSE			= 4,		// ����֮�����Ӧ��Ϣ
	eMT_GATE_FORWARD		= 5,		// �ͻ���ͨ�����ط���ת��������������Ϣ
	eMT_ACTOR_REQUEST		= 6,		// actor֮���������Ϣ
	eMT_ACTOR_RESPONSE		= 7,		// actor֮�����Ӧ��Ϣ
	eMT_ACTOR_GATE_FORWARD	= 8,		// �ͻ���ͨ�����ط���ת��������actor��Ϣ
	eMT_TO_GATE				= 9,		// ��������ͨ�����ط���ת���ͻ�����Ϣ
	eMT_CLIENT				= 10,		// �ͻ�����Ϣ
	
	eMT_TYPE_MASK			= 0x00ff,	// ��������
	eMT_LUA					= 0x0100,	// lua��Ϣ

	// ������
	eMT_BROADCAST			= 0x200,	// �㲥��Ϣ
};

namespace core
{
	typedef std::function<int32_t(const char*, uint32_t, uint8_t&)>	MessageParser;	// ԭ����Ϣ

	struct	SMessagePacket
	{
		uint8_t		nType;
		uint32_t	nDataSize;
		void*		pData;
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

#define _BASE_CONNECTION_TYPE_BEGIN	100