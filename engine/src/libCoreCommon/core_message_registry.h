#pragma once
#include "libBaseCommon/noncopyable.h"

#include "core_common.h"
#include "protobuf_helper.h"

#include <map>
#include <vector>

namespace core
{
	class CCoreMessageRegistry :
		public base::noncopyable
	{
	public:
		CCoreMessageRegistry();
		~CCoreMessageRegistry();

		bool	init();

		void	registerCallback(uint16_t nServiceID, const std::string& szMessageName, const std::function<void(SServiceSessionInfo, google::protobuf::Message*)>& callback);
		void	registerGateForwardCallback(uint16_t nServiceID, const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback);
		
		std::function<void(SServiceSessionInfo, google::protobuf::Message*)>&
				getCallback(uint16_t nServiceID, const std::string& szMessageName);
		std::function<void(SClientSessionInfo, google::protobuf::Message*)>&
				getGateForwardCallback(uint16_t nServiceID, const std::string& szMessageName);

		void	onConnectToMaster();

	private:
		struct SServiceRegistryInfo
		{
			std::map<std::string, std::function<void(SServiceSessionInfo, google::protobuf::Message*)>>		mapServiceCallback;
			std::map<std::string, std::function<void(SClientSessionInfo, google::protobuf::Message*)>>		mapGateForwardCallback;
		};
		
		std::map<uint16_t, SServiceRegistryInfo>	m_mapServiceRegistryInfo;
	};
}