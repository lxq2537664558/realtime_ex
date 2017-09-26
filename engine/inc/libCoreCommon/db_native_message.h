#pragma once
#include "native_message.h"

namespace core
{
native_message_begin(call_command)
	uint32_t	channel_id;
	std::string sql;
	std::vector<std::string>
				args;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(channel_id);
		writeBuf.write(sql);
		uint16_t arg_count = (uint16_t)args.size();
		writeBuf.write(arg_count);
		for (uint16_t i = 0; i < arg_count; ++i)
		{
			writeBuf.write(args[i]);
		}

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(channel_id);
		readBuf.read(sql);
		uint16_t arg_count = 0;
		readBuf.read(arg_count);
		args.resize(arg_count);
		for (uint16_t i = 0; i < arg_count; ++i)
		{
			readBuf.read(args[i]);
		}

		native_unpack_end();

		return true;
	}
native_message_end(call_command)


native_message_begin(delete_command)
	uint64_t	id;
	std::string table_name;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(id);
		writeBuf.write(table_name);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(id);
		readBuf.read(table_name);

		native_unpack_end();

		return true;
	}
native_message_end(delete_command)


native_message_begin(flush_command)
	uint64_t id;
	uint32_t type;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(id);
		writeBuf.write(type);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(id);
		readBuf.read(type);

		native_unpack_end();

		return true;
	}
native_message_end(flush_command)


native_message_begin(insert_command)
	std::string message_name;
	std::string message_content;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(message_name);
		writeBuf.write(message_content);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(message_name);
		readBuf.read(message_content);

		native_unpack_end();

		return true;
	}
native_message_end(insert_command)


native_message_begin(nop_command)
	uint32_t channel_id;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(channel_id);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(channel_id);

		native_unpack_end();

		return true;
	}
native_message_end(nop_command)


native_message_begin(query_command)
	uint32_t	channel_id;
	std::string table_name;
	std::string where_clause;
	std::vector<std::string>
				args;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(channel_id);
		writeBuf.write(table_name);
		writeBuf.write(where_clause);
		uint16_t arg_count = (uint16_t)args.size();
		writeBuf.write(arg_count);
		for (uint16_t i = 0; i < arg_count; ++i)
		{
			writeBuf.write(args[i]);
		}

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(channel_id);
		readBuf.read(table_name);
		readBuf.read(where_clause);
		uint16_t arg_count = 0;
		readBuf.read(arg_count);
		args.resize(arg_count);
		for (uint16_t i = 0; i < arg_count; ++i)
		{
			readBuf.read(args[i]);
		}

		native_unpack_end();

		return true;
	}
native_message_end(query_command)


native_message_begin(select_command)
	uint64_t	id;
	std::string table_name;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(id);
		writeBuf.write(table_name);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(id);
		readBuf.read(table_name);

		native_unpack_end();

		return true;
	}
native_message_end(select_command)


native_message_begin(update_command)
	std::string message_name;
	std::string message_content;

	void pack(base::CWriteBuf& writeBuf) const
	{
		native_pack_begin(writeBuf);

		writeBuf.write(message_name);
		writeBuf.write(message_content);

		native_pack_end(writeBuf);
	}

	bool unpack(const void* pBuf, uint16_t nSize)
	{
		native_unpack_begin(pBuf, nSize);

		readBuf.read(message_name);
		readBuf.read(message_content);

		native_unpack_end();

		return true;
	}
native_message_end(update_command)

}