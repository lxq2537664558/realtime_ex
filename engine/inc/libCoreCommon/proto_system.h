#pragma once
#include "libCoreCommon/core_common.h"
#include "libBaseCommon/buf_file.h"

#include "core_common.h"

#include <vector>
#include <map>
#include <set>

enum ESystemMessageType
{
	eSMT_register_node_base_info	= 1,	// 各个节点向master节点注册节点
	eSMT_unregister_node_base_info	= 2,	// 各个节点向master节点反注册节点
	eSMT_sync_master_info			= 3,	// master向各个节点同步master信息
	eSMT_sync_node_base_info		= 4,	// master向各个节点同步其他节点信息，在新节点加入的时候将新节点同步给其他节点
	eSMT_sync_all_node_base_info	= 5,	// master向各个节点同步所有其他节点信息
	eSMT_remove_node_base_info		= 6,	// master向各个节点同步其他节点离开信息
	eSMT_node_handshake_request		= 7,	// 主动发起的节点向被动接收的节点请求握手
	eSMT_node_handshake_response	= 8,	// 被动接收的节点向主动发起的节点请响应握手
};

#define system_message_begin(MessageName, nMessageID) \
class MessageName : public core::message_header\
{\
public:\
	MessageName() : core::message_header(nMessageID) { nMessageSize = sizeof(MessageName); }\
	static  uint32_t	getMessageID() { return nMessageID; }\
	static  const char*	getMessageName() { return #MessageName; }

#define system_message_end };

#define system_pack_begin(writeBuf)\
	writeBuf.clear();\
	writeBuf.write(this, sizeof(core::message_header));

#define system_pack_end(writeBuf)\
	do\
	{\
		uint16_t nPos = (uint16_t)writeBuf.getCurSize(); \
		writeBuf.seek(base::eBST_Begin, 0); \
		writeBuf.write(nPos); \
		writeBuf.seek(base::eBST_Begin, nPos);\
	} while(0)

#define system_unpack_begin(buf, size)\
	base::CReadBuf readBuf;\
	readBuf.init(buf, size);\
	readBuf.read(this, sizeof(core::message_header));

#define system_unpack_end()

namespace core
{
system_message_begin(smt_register_node_base_info, eSMT_register_node_base_info)
	SNodeBaseInfo					sNodeBaseInfo;
	std::vector<SServiceBaseInfo>	vecServiceBaseInfo;
	std::set<std::string>			setConnectServiceName;
	std::set<std::string>			setConnectServiceType;

	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		writeBuf.write(sNodeBaseInfo.nID);
		writeBuf.write(sNodeBaseInfo.szName);
		writeBuf.write(sNodeBaseInfo.szGroup);
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

		uint16_t nServiceNameCount = (uint16_t)setConnectServiceName.size();
		writeBuf.write(nServiceNameCount);
		for (auto iter = setConnectServiceName.begin(); iter != setConnectServiceName.end(); ++iter)
		{
			writeBuf.write(*iter);
		}

		uint16_t nServiceTypeCount = (uint16_t)setConnectServiceType.size();
		writeBuf.write(nServiceTypeCount);
		for (auto iter = setConnectServiceType.begin(); iter != setConnectServiceType.end(); ++iter)
		{
			writeBuf.write(*iter);
		}

		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		readBuf.read(sNodeBaseInfo.nID);
		readBuf.read(sNodeBaseInfo.szName);
		readBuf.read(sNodeBaseInfo.szGroup);
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

		uint16_t nServiceNameCount = 0;
		readBuf.read(nServiceNameCount);
		for (uint16_t i = 0; i < nServiceNameCount; ++i)
		{
			std::string szName;

			readBuf.read(szName);

			setConnectServiceName.insert(szName);
		}

		uint16_t nServiceTypeCount = 0;
		readBuf.read(nServiceTypeCount);
		for (uint16_t i = 0; i < nServiceTypeCount; ++i)
		{
			std::string szType;

			readBuf.read(szType);

			setConnectServiceName.insert(szType);
		}

		system_unpack_end();
	}

system_message_end

system_message_begin(smt_unregister_node_base_info, eSMT_unregister_node_base_info)
	std::string	szName;

	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		writeBuf.write(szName);

		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		readBuf.read(szName);

		system_unpack_end();
	}
system_message_end

system_message_begin(smt_sync_master_info, eSMT_sync_master_info)
	uint32_t nMasterID;

	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		writeBuf.write(nMasterID);

		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		readBuf.read(nMasterID);

		system_unpack_end();
	}
system_message_end

system_message_begin(smt_sync_node_base_info, eSMT_sync_node_base_info)
	SNodeBaseInfo					sNodeBaseInfo;
	std::vector<SServiceBaseInfo>	vecServiceBaseInfo;
	uint8_t							bExcludeConnect;

	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		writeBuf.write(sNodeBaseInfo.nID);
		writeBuf.write(sNodeBaseInfo.szName);
		writeBuf.write(sNodeBaseInfo.szGroup);
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

		writeBuf.write(bExcludeConnect);

		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		readBuf.read(sNodeBaseInfo.nID);
		readBuf.read(sNodeBaseInfo.szName);
		readBuf.read(sNodeBaseInfo.szGroup);
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

		readBuf.read(bExcludeConnect);

		system_unpack_end();
	}
system_message_end

system_message_begin(smt_sync_all_node_base_info, eSMT_sync_all_node_base_info)
	std::map<uint32_t, SNodeInfo>	mapNodeInfo;
	std::set<uint32_t>				setExcludeConnectNodeID;	// 因为double link排除掉的节点
	
	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		uint16_t nNodeCount = (uint16_t)mapNodeInfo.size();
		writeBuf.write(nNodeCount);
		for (auto iter = mapNodeInfo.begin(); iter != mapNodeInfo.end(); ++iter)
		{
			const SNodeInfo& sNodeInfo = iter->second;

			writeBuf.write(sNodeInfo.sNodeBaseInfo.nID);
			writeBuf.write(sNodeInfo.sNodeBaseInfo.szName);
			writeBuf.write(sNodeInfo.sNodeBaseInfo.szGroup);
			writeBuf.write(sNodeInfo.sNodeBaseInfo.szHost);
			writeBuf.write(sNodeInfo.sNodeBaseInfo.nPort);
			writeBuf.write(sNodeInfo.sNodeBaseInfo.nRecvBufSize);
			writeBuf.write(sNodeInfo.sNodeBaseInfo.nSendBufSize);

			uint16_t nServiceCount = (uint16_t)sNodeInfo.vecServiceBaseInfo.size();
			writeBuf.write(nServiceCount);
			for (uint16_t j = 0; j < nServiceCount; ++j)
			{
				const SServiceBaseInfo& sServiceBaseInfo = sNodeInfo.vecServiceBaseInfo[j];

				writeBuf.write(sServiceBaseInfo.nID);
				writeBuf.write(sServiceBaseInfo.szName);
				writeBuf.write(sServiceBaseInfo.szType);
			}
		}

		uint16_t nExcludeConnectNodeCount = (uint16_t)setExcludeConnectNodeID.size();
		writeBuf.write(nExcludeConnectNodeCount);
		for (auto iter = setExcludeConnectNodeID.begin(); iter != setExcludeConnectNodeID.end(); ++iter)
		{
			uint32_t nNodeID = *iter;

			writeBuf.write(nNodeID);
		}

		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		uint16_t nNodeCount = 0;
		readBuf.read(nNodeCount);
		for (uint16_t i = 0; i < nNodeCount; ++i)
		{
			SNodeInfo sNodeInfo;

			readBuf.read(sNodeInfo.sNodeBaseInfo.nID);
			readBuf.read(sNodeInfo.sNodeBaseInfo.szName);
			readBuf.read(sNodeInfo.sNodeBaseInfo.szGroup);
			readBuf.read(sNodeInfo.sNodeBaseInfo.szHost);
			readBuf.read(sNodeInfo.sNodeBaseInfo.nPort);
			readBuf.read(sNodeInfo.sNodeBaseInfo.nRecvBufSize);
			readBuf.read(sNodeInfo.sNodeBaseInfo.nSendBufSize);
			uint16_t nServiceCount = 0;
			readBuf.read(nServiceCount);
			sNodeInfo.vecServiceBaseInfo.resize(nServiceCount);
			for (uint16_t j = 0; j < nServiceCount; ++j)
			{
				SServiceBaseInfo& sServiceBaseInfo = sNodeInfo.vecServiceBaseInfo[j];

				readBuf.read(sServiceBaseInfo.nID);
				readBuf.read(sServiceBaseInfo.szName);
				readBuf.read(sServiceBaseInfo.szType);
			}

			mapNodeInfo[sNodeInfo.sNodeBaseInfo.nID] = sNodeInfo;
		}

		uint16_t nExcludeConnectNodeCount = 0;
		readBuf.read(nExcludeConnectNodeCount);
		for (uint16_t i = 0; i < nExcludeConnectNodeCount; ++i)
		{
			uint32_t nNodeID = 0;

			readBuf.read(nNodeID);

			setExcludeConnectNodeID.insert(nNodeID);
		}

		system_unpack_end();
	}
system_message_end

system_message_begin(smt_remove_node_base_info, eSMT_remove_node_base_info)
	uint32_t nNodeID;

	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		writeBuf.write(nNodeID);
		
		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		readBuf.read(nNodeID);
		
		system_unpack_end();
	}
system_message_end

system_message_begin(smt_node_handshake_request, eSMT_node_handshake_request)
	uint32_t nNodeID;
	
	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		writeBuf.write(nNodeID);
		
		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		readBuf.read(nNodeID);
		
		system_unpack_end();
	}
	system_message_end

system_message_begin(smt_node_handshake_response, eSMT_node_handshake_response)
	uint32_t nNodeID;

	void pack(base::CWriteBuf& writeBuf)
	{
		system_pack_begin(writeBuf);

		writeBuf.write(nNodeID);

		system_pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		system_unpack_begin(pBuf, nSize);

		readBuf.read(nNodeID);

		system_unpack_end();
	}
system_message_end
}