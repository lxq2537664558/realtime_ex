#pragma once
#include "libCoreCommon/core_common.h"
#include "libBaseCommon/buf_file.h"

#include "core_service_kit_define.h"

#include <vector>

enum ESystemMessageType
{
	eSMT_register_node_base_info	= 1,	// �����ڵ���master�ڵ�ע��ڵ�
	eSMT_unregister_node_base_info	= 2,	// �����ڵ���master�ڵ㷴ע��ڵ�
	eSMT_sync_node_base_info		= 3,	// master������ڵ�ͬ�������ڵ���Ϣ
	eSMT_remove_node_base_info		= 4,	// master������ڵ�ͬ�������ڵ��뿪��Ϣ
	eSMT_notify_node_base_info		= 5,	// ��������Ľڵ��򱻶����յĽڵ�֪ͨ�ڵ������Ϣ
	eSMT_notify_ack_node_base_info	= 6,	// �������յĽڵ�����������Ľڵ�ȷ�Ͻڵ������Ϣ
};

namespace core
{
message_begin(smt_register_service_base_info, eSMT_register_node_base_info)
	SNodeBaseInfo					sNodeBaseInfo;
	std::vector<SServiceBaseInfo>	vecServiceBaseInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(sNodeBaseInfo.nID);
		writeBuf.write(sNodeBaseInfo.szName);

		uint16_t nServiceCount = (uint16_t)vecServiceBaseInfo.size();
		writeBuf.write(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			writeBuf.write(vecServiceBaseInfo[i].nID);
			writeBuf.write(vecServiceBaseInfo[i].szName);
			writeBuf.write(vecServiceBaseInfo[i].szType);
			writeBuf.write(vecServiceBaseInfo[i].szHost);
			writeBuf.write(vecServiceBaseInfo[i].nPort);
			writeBuf.write(vecServiceBaseInfo[i].nRecvBufSize);
			writeBuf.write(vecServiceBaseInfo[i].nSendBufSize);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(sNodeBaseInfo.nID);
		readBuf.read(sNodeBaseInfo.szName);
		uint16_t nServiceCount = 0;
		readBuf.read(nServiceCount);
		vecServiceBaseInfo.resize(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			readBuf.read(vecServiceBaseInfo[i].nID);
			readBuf.read(vecServiceBaseInfo[i].szName);
			readBuf.read(vecServiceBaseInfo[i].szType);
			readBuf.read(vecServiceBaseInfo[i].szHost);
			readBuf.read(vecServiceBaseInfo[i].nPort);
			readBuf.read(vecServiceBaseInfo[i].nRecvBufSize);
			readBuf.read(vecServiceBaseInfo[i].nSendBufSize);
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

message_begin(smt_sync_node_base_info, eSMT_sync_node_base_info)
	SNodeBaseInfo					sNodeBaseInfo;
	std::vector<SServiceBaseInfo>	vecServiceBaseInfo;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(sNodeBaseInfo.nID);
		writeBuf.write(sNodeBaseInfo.szName);

		uint16_t nServiceCount = (uint16_t)vecServiceBaseInfo.size();
		writeBuf.write(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			writeBuf.write(vecServiceBaseInfo[i].nID);
			writeBuf.write(vecServiceBaseInfo[i].szName);
			writeBuf.write(vecServiceBaseInfo[i].szType);
			writeBuf.write(vecServiceBaseInfo[i].szHost);
			writeBuf.write(vecServiceBaseInfo[i].nPort);
			writeBuf.write(vecServiceBaseInfo[i].nRecvBufSize);
			writeBuf.write(vecServiceBaseInfo[i].nSendBufSize);
		}

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(sNodeBaseInfo.nID);
		readBuf.read(sNodeBaseInfo.szName);
		uint16_t nServiceCount = 0;
		readBuf.read(nServiceCount);
		vecServiceBaseInfo.resize(nServiceCount);
		for (uint16_t i = 0; i < nServiceCount; ++i)
		{
			readBuf.read(vecServiceBaseInfo[i].nID);
			readBuf.read(vecServiceBaseInfo[i].szName);
			readBuf.read(vecServiceBaseInfo[i].szType);
			readBuf.read(vecServiceBaseInfo[i].szHost);
			readBuf.read(vecServiceBaseInfo[i].nPort);
			readBuf.read(vecServiceBaseInfo[i].nRecvBufSize);
			readBuf.read(vecServiceBaseInfo[i].nSendBufSize);
		}
		unpack_end();
	}

message_end

message_begin(smt_remove_node_base_info, eSMT_remove_node_base_info)
	uint16_t nNodeID;
	uint8_t	 nForce;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(nNodeID);
		writeBuf.write(nForce);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(nNodeID);
		readBuf.read(nForce);

		unpack_end();
	}
message_end

message_begin(smt_notify_node_base_info, eSMT_notify_node_base_info)
	uint16_t nFromServiceID;
	
	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(nFromServiceID);
		
		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(nFromServiceID);
		
		unpack_end();
	}
message_end

message_begin(smt_notify_ack_node_base_info, eSMT_notify_ack_node_base_info)
	std::string	szNodeName;

	void pack(base::CWriteBuf& writeBuf)
	{
		pack_begin(writeBuf);

		writeBuf.write(szNodeName);

		pack_end(writeBuf);
	}

	void unpack(const void* pBuf, uint16_t nSize)
	{
		unpack_begin(pBuf, nSize);

		readBuf.read(szNodeName);

		unpack_end();
	}
message_end

}