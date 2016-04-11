#pragma once
#include "core_common.h"

#include "libBaseCommon/buf_file.h"

#include <vector>

enum ESystemMessageType
{
	eSMT_sync_service_base_info		= 1,	// 各个服务跟master服务之间同步服务基本信息
	eSMT_remove_service_base_info	= 2,	// 各个服务跟master服务之间同步移除服务
	eSMT_notify_service_base_info	= 3,	// 主动发起的服务向被动接收的服务通知服务基本信息
	eSMT_sync_service_message_info		= 4,	// 各个服务跟master服务之间同步服务的API信息
};

message_begin(smt_sync_service_base_info, eSMT_sync_service_base_info)
	SServiceBaseInfo	sServiceBaseInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(sServiceBaseInfo.szType);
		writeBuf.write(sServiceBaseInfo.szName);
		writeBuf.write(sServiceBaseInfo.szHost);
		writeBuf.write(sServiceBaseInfo.nPort);
		writeBuf.write(sServiceBaseInfo.nRecvBufSize);
		writeBuf.write(sServiceBaseInfo.nSendBufSize);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(sServiceBaseInfo.szType);
		readBuf.read(sServiceBaseInfo.szName);
		readBuf.read(sServiceBaseInfo.szHost);
		readBuf.read(sServiceBaseInfo.nPort);
		readBuf.read(sServiceBaseInfo.nRecvBufSize);
		readBuf.read(sServiceBaseInfo.nSendBufSize);

		unpack_end();
	}

message_end

message_begin(smt_remove_service_base_info, eSMT_remove_service_base_info)
	std::string	szName;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(szName);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(szName);

		unpack_end();
	}
message_end

message_begin(smt_notify_service_base_info, eSMT_notify_service_base_info)
	std::string	szName;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(szName);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(szName);

		unpack_end();
	}
message_end

message_begin(smt_sync_service_message_info, eSMT_sync_service_message_info)
	uint8_t							nAdd;
	std::vector<SMessageSyncInfo>	vecMessageSyncInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(nAdd);
		uint16_t nCount = (uint16_t)vecMessageSyncInfo.size();
		writeBuf.write(nCount);
		for (uint16_t i = 0; i < nCount; ++i)
		{
			writeBuf.write(vecMessageSyncInfo[i].nGate);
			writeBuf.write(vecMessageSyncInfo[i].nMessageID);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(nAdd);
		uint16_t nCount = 0;
		readBuf.read(nCount);
		vecMessageSyncInfo.reserve(nCount);
		for (uint16_t i = 0; i < nCount; ++i)
		{
			SMessageSyncInfo sMessageSyncInfo;
			readBuf.read(sMessageSyncInfo.nGate);
			readBuf.read(sMessageSyncInfo.nMessageID);

			vecMessageSyncInfo.push_back(sMessageSyncInfo);
		}

		unpack_end();
	}
message_end