#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/ticker.h"

#include "google/protobuf/message.h"

#include <functional>
#include <map>

namespace core
{
	enum EResponseResultType
	{
		eRRT_OK,
		eRRT_TIME_OUT,
		eRRT_ERROR,
	};

	class CMessagePort :
		public base::noncopyable
	{
	public:
		CMessagePort();
		~CMessagePort();

		bool		init();

		bool		call(const std::string& szServiceName, const google::protobuf::Message* pMessage);
		bool		call_r(const std::string& szServiceName, const google::protobuf::Message* pMessage, std::function<void(const google::protobuf::Message*, EResponseResultType)>& callback);
		bool		broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage);
		void		response(const google::protobuf::Message* pMessage);
		void		registMessageHandler(const std::string& szMessageName, std::function<void(const google::protobuf::Message*)>& callback);
	};
}