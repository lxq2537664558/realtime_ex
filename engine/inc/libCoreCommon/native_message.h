#pragma once

#include "libBaseCommon/buf_file.h"

#include "core_common.h"

namespace core
{
	// ÏûÏ¢Í·
	class __CORE_COMMON_API__ native_message_header
	{
	public:
		native_message_header() { }
		virtual ~native_message_header() { }

		virtual const char* getMessageName() const = 0;
		virtual void		pack(base::CWriteBuf& writeBuf) const = 0;
		virtual bool		unpack(const void* pBuf, uint16_t nSize) = 0;

		static void			registerNativeMessage(const std::string& szName, const std::function<native_message_header*()>& fnCreate, const std::function<void(native_message_header*)>& fnDestroy);
		static native_message_header*
							createNativeMessage(const std::string& szName);
		static void			destroyNativeMessage(native_message_header* pHeader);
	};
}

#define native_message_begin(MessageName) \
class MessageName : public core::native_message_header\
{\
public:\
	virtual  const char*			getMessageName() const { return #MessageName; }

#define native_message_end(MessageName) };\
	static core::native_message_header*	create##MessageName()\
	{\
		return new MessageName();\
	}\
	static void destroy##MessageName(core::native_message_header* pHeader)\
	{\
		delete pHeader;\
	}\
	struct SOnce##MessageName\
	{\
		SOnce##MessageName()\
		{\
			auto fnCreate = std::bind(&create##MessageName);\
			auto fnDestroy = std::bind(&destroy##MessageName, std::placeholders::_1);\
			core::native_message_header::registerNativeMessage(#MessageName, fnCreate, fnDestroy);\
		}\
	};\
	static SOnce##MessageName s_Once##MessageName;

#define native_pack_begin(writeBuf)\
	writeBuf.clear();\

#define native_pack_end(writeBuf)

#define native_unpack_begin(buf, size)\
	base::CReadBuf readBuf;\
	readBuf.init(buf, size);

#define native_unpack_end()