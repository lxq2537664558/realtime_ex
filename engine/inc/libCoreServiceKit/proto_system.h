#pragma once
#include "libCoreCommon/core_common.h"
#include "libBaseCommon/buf_file.h"

#include "core_service_kit_define.h"

#include <vector>

enum ESystemMessageType
{
	eSMT_register_service_base_info		= 1,	// 各个服务向master服务注册服务
	eSMT_unregister_service_base_info	= 2,	// 各个服务向master服务反注册服务
	eSMT_sync_service_base_info			= 3,	// master向各个服务同步其他服务信息
	eSMT_remove_service_base_info		= 4,	// master向各个服务同步其他服务离开信息
	eSMT_notify_service_base_info		= 3,	// 主动发起的服务向被动接收的服务通知服务基本信息
	eSMT_register_service_message_info	= 4,	// 各个服务向master服务服务的API信息
	eSMT_sync_service_message_info		= 5,	// master服务向各个服务同步其他服务的的API信息
};

namespace core
{
message_begin(smt_register_service_base_info, eSMT_register_service_base_info)
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

message_begin(smt_unregister_service_base_info, eSMT_unregister_service_base_info)
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
	std::string	szKey;
	std::string	szServiceName;
	
	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(szKey);
		writeBuf.write(szServiceName);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(szKey);
		readBuf.read(szServiceName);

		unpack_end();
	}
message_end

message_begin(smt_register_service_message_info, eSMT_register_service_message_info)
	std::vector<SMessageProxyInfo>	vecMessageProxyInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		uint16_t nCount = (uint16_t)vecMessageProxyInfo.size();
		writeBuf.write(nCount);
		for (uint16_t i = 0; i < nCount; ++i)
		{
			writeBuf.write(vecMessageProxyInfo[i].szServiceName);
			writeBuf.write(vecMessageProxyInfo[i].szServiceGroup);
			writeBuf.write(vecMessageProxyInfo[i].szMessageName);
			writeBuf.write(vecMessageProxyInfo[i].nWeight);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		uint16_t nCount = 0;
		readBuf.read(nCount);
		vecMessageProxyInfo.reserve(nCount);
		for (uint16_t i = 0; i < nCount; ++i)
		{
			SMessageProxyInfo sMessageProxyInfo;
			readBuf.read(sMessageProxyInfo.szServiceName);
			readBuf.read(sMessageProxyInfo.szServiceGroup);
			readBuf.read(sMessageProxyInfo.szMessageName);
			readBuf.read(sMessageProxyInfo.nWeight);

			vecMessageProxyInfo.push_back(sMessageProxyInfo);
		}

		unpack_end();
	}
message_end

message_begin(smt_sync_service_message_info, eSMT_sync_service_message_info)
	std::string						szServiceName;
	std::vector<SMessageProxyInfo>	vecMessageProxyInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(szServiceName);
		uint16_t nCount = (uint16_t)vecMessageProxyInfo.size();
		writeBuf.write(nCount);
		for (uint16_t i = 0; i < nCount; ++i)
		{
			writeBuf.write(vecMessageProxyInfo[i].szServiceName.c_str());
			writeBuf.write(vecMessageProxyInfo[i].szServiceGroup.c_str());
			writeBuf.write(vecMessageProxyInfo[i].szMessageName.c_str());
			writeBuf.write(vecMessageProxyInfo[i].nWeight);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(szServiceName);
		uint16_t nCount = 0;
		readBuf.read(nCount);
		vecMessageProxyInfo.reserve(nCount);
		for (uint16_t i = 0; i < nCount; ++i)
		{
			SMessageProxyInfo sMessageProxyInfo;
			readBuf.read(sMessageProxyInfo.szServiceName);
			readBuf.read(sMessageProxyInfo.szServiceGroup);
			readBuf.read(sMessageProxyInfo.szMessageName);
			readBuf.read(sMessageProxyInfo.nWeight);

			vecMessageProxyInfo.push_back(sMessageProxyInfo);
		}

		unpack_end();
	}
message_end

}