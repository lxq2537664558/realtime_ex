#pragma once

#include <string>
#include <functional>

#include "google/protobuf/message.h"

#ifdef _WIN32

#	ifdef __BUILD_BASE_DATABASE_DLL__
#		define __BASE_DATABASE_API__ __declspec(dllexport)
#	else
#		define __BASE_DATABASE_API__ __declspec(dllimport)
#	endif

#else

#	define __BASE_DATABASE_API__

#endif

namespace base
{
	namespace db
	{
		enum EDbCommandType
		{
			eDBCT_None = 0,
			eDBCT_Query = 1,
			eDBCT_Select = 2,
			eDBCT_Insert = 3,
			eDBCT_Delete = 4,
			eDBCT_Update = 5,
			eDBCT_Call = 6,
			eDBCT_Flush = 7,
			eDBCT_Nop = 8,
		};

		enum EDbResultCode
		{
			eDBRC_OK = 1,
			eDBRC_EmptyRecordset = 10,
			eDBRC_ProtobufError,
			eDBRC_MysqlError,
			eDBRC_SQLArgError,
			eDBRC_LostConnection,

			eDBRC_Unknown,
		};

		struct SDbOptions
		{
			std::string					szProtoDir;
			uint32_t					nDbThreadCount;
			std::function<google::protobuf::Message*(const std::string&)>
										funcCreateMessage;		// 直接使用这个库操作数据的，需要提供返回对象的创建函数
			std::function<void(google::protobuf::Message*)>
										funcDestroyMessage;		// 直接使用这个库操作数据的，需要提供返回对象的销毁函数
			std::vector<std::string>	vecCacheTable;
			uint64_t					nMaxCacheSize;
			uint32_t					nCacheWritebackTime;
		};

		__BASE_DATABASE_API__ uint32_t	create(const std::string& szHost, uint16_t nPort, const std::string& szDb, const std::string& szUser, const std::string& szPassword, const std::string& szCharset, const SDbOptions& sDbOptions);
		__BASE_DATABASE_API__ void		query(uint32_t nID, const google::protobuf::Message* pRequest, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		__BASE_DATABASE_API__ void		update(uint32_t nID);
		__BASE_DATABASE_API__ void		release(uint32_t nID);
		__BASE_DATABASE_API__ uint32_t	getThreadCount(uint32_t nID);
		__BASE_DATABASE_API__ uint32_t	getQueueSize(uint32_t nID, uint32_t nThreadIndex);
	}
}