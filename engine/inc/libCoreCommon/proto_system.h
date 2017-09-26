#pragma once
#include "libCoreCommon/core_common.h"
#include "libBaseCommon/buf_file.h"

#include "core_common.h"

#include <vector>

enum ESystemMessageType
{
	eSMT_register_node_base_info	= 1,	// 各个节点向master节点注册节点
	eSMT_unregister_node_base_info	= 2,	// 各个节点向master节点反注册节点
	eSMT_sync_master_info			= 3,	// master向各个节点同步master信息
	eSMT_sync_node_base_info		= 4,	// master向各个节点同步其他节点信息
	eSMT_remove_node_base_info		= 5,	// master向各个节点同步其他节点离开信息
	eSMT_notify_node_base_info		= 6,	// 主动发起的节点向被动接收的节点通知节点基本信息
	eSMT_notify_ack_node_base_info	= 7,	// 被动接收的节点向主动发起的节点确认节点基本信息
	eSMT_request_service_health		= 8,	// 服务之间的健康度请求消息
	eSMT_response_service_health	= 9,	// 服务之间的健康度响应消息
};

namespace core
{
message_begin(smt_register_node_base_info, eSMT_register_node_base_info)
	SNodeBaseInfo					sNodeBaseInfo;
	std::vector<SServiceBaseInfo>	vecServiceBaseInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(sNodeBaseInfo.nID);
		writeBuf.write(sNodeBaseInfo.szName);
		writeBuf.write(sNodeBaseInfo.szHost);
		writeBuf.write(sNodeBaseInfo.nPort);
		writeBuf.write(sNodeBaseInfo.nRecvBufSize);
		writeBuf.write(sNodeBaseInfo.nSendBufSize);

		uint16_t nServiceCount = (uint16_t)vecServiceBaseInfo.size();
		writeBuf.write(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			writeBuf.write(vecServiceBaseInfo[i].nID);
			writeBuf.write(vecServiceBaseInfo[i].szName);
			writeBuf.write(vecServiceBaseInfo[i].szType);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(sNodeBaseInfo.nID);
		readBuf.read(sNodeBaseInfo.szName);
		readBuf.read(sNodeBaseInfo.szHost);
		readBuf.read(sNodeBaseInfo.nPort);
		readBuf.read(sNodeBaseInfo.nRecvBufSize);
		readBuf.read(sNodeBaseInfo.nSendBufSize);
		uint16_t nServiceCount = 0;
		readBuf.read(nServiceCount);
		vecServiceBaseInfo.resize(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			readBuf.read(vecServiceBaseInfo[i].nID);
			readBuf.read(vecServiceBaseInfo[i].szName);
			readBuf.read(vecServiceBaseInfo[i].szType);
		}
		unpack_end();
	}

message_end

message_begin(smt_unregister_node_base_info, eSMT_unregister_node_base_info)
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

message_begin(smt_sync_master_info, eSMT_sync_master_info)
	uint32_t nMasterID;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(nMasterID);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(nMasterID);

		unpack_end();
	}
message_end

message_begin(smt_sync_node_base_info, eSMT_sync_node_base_info)
	SNodeBaseInfo					sNodeBaseInfo;
	std::vector<SServiceBaseInfo>	vecServiceBaseInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(sNodeBaseInfo.nID);
		writeBuf.write(sNodeBaseInfo.szName);
		writeBuf.write(sNodeBaseInfo.szHost);
		writeBuf.write(sNodeBaseInfo.nPort);
		writeBuf.write(sNodeBaseInfo.nRecvBufSize);
		writeBuf.write(sNodeBaseInfo.nSendBufSize);

		uint16_t nServiceCount = (uint16_t)vecServiceBaseInfo.size();
		writeBuf.write(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			writeBuf.write(vecServiceBaseInfo[i].nID);
			writeBuf.write(vecServiceBaseInfo[i].szName);
			writeBuf.write(vecServiceBaseInfo[i].szType);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(sNodeBaseInfo.nID);
		readBuf.read(sNodeBaseInfo.szName);
		readBuf.read(sNodeBaseInfo.szHost);
		readBuf.read(sNodeBaseInfo.nPort);
		readBuf.read(sNodeBaseInfo.nRecvBufSize);
		readBuf.read(sNodeBaseInfo.nSendBufSize);
		uint16_t nServiceCount = 0;
		readBuf.read(nServiceCount);
		vecServiceBaseInfo.resize(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			readBuf.read(vecServiceBaseInfo[i].nID);
			readBuf.read(vecServiceBaseInfo[i].szName);
			readBuf.read(vecServiceBaseInfo[i].szType);
		}
		unpack_end();
	}

message_end

message_begin(smt_remove_node_base_info, eSMT_remove_node_base_info)
	uint32_t nNodeID;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(nNodeID);
		
		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(nNodeID);
		
		unpack_end();
	}
message_end

message_begin(smt_notify_node_base_info, eSMT_notify_node_base_info)
	SNodeBaseInfo					sNodeBaseInfo;
	std::vector<SServiceBaseInfo>	vecServiceBaseInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(sNodeBaseInfo.nID);
		writeBuf.write(sNodeBaseInfo.szName);
		writeBuf.write(sNodeBaseInfo.szHost);
		writeBuf.write(sNodeBaseInfo.nPort);
		writeBuf.write(sNodeBaseInfo.nRecvBufSize);
		writeBuf.write(sNodeBaseInfo.nSendBufSize);

		uint16_t nServiceCount = (uint16_t)vecServiceBaseInfo.size();
		writeBuf.write(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			writeBuf.write(vecServiceBaseInfo[i].nID);
			writeBuf.write(vecServiceBaseInfo[i].szName);
			writeBuf.write(vecServiceBaseInfo[i].szType);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(sNodeBaseInfo.nID);
		readBuf.read(sNodeBaseInfo.szName);
		readBuf.read(sNodeBaseInfo.szHost);
		readBuf.read(sNodeBaseInfo.nPort);
		readBuf.read(sNodeBaseInfo.nRecvBufSize);
		readBuf.read(sNodeBaseInfo.nSendBufSize);
		uint16_t nServiceCount = 0;
		readBuf.read(nServiceCount);
		vecServiceBaseInfo.resize(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			readBuf.read(vecServiceBaseInfo[i].nID);
			readBuf.read(vecServiceBaseInfo[i].szName);
			readBuf.read(vecServiceBaseInfo[i].szType);
		}
		unpack_end();
	}
	message_end

message_begin(smt_notify_ack_node_base_info, eSMT_notify_ack_node_base_info)
	uint32_t nNodeID;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(nNodeID);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(nNodeID);

		unpack_end();
	}
message_end
// 
// message_begin(smt_request_service_health, eSMT_request_service_health)
// 	uint32_t nToServiceID;
// 	
// 
// 	void pack(base::CWriteBuf& writeBuf)
// 	{
// 		pack_begin(writeBuf);
// 
// 		writeBuf.write(nNodeID);
// 
// 		pack_end(writeBuf);
// 	}
// 
// 	void unpack(const void* pBuf, uint16_t nSize)
// 	{
// 		unpack_begin(pBuf, nSize);
// 
// 		readBuf.read(nNodeID);
// 
// 		unpack_end();
// 	}
// message_end
}