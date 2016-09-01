#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

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

		void	registerCallback(uint16_t nMessageID, const std::function<bool(uint16_t, CMessagePtr<char>)>& callback);
		void	registerGateForwardCallback(uint16_t nMessageID, const std::function<bool(SClientSessionInfo, CMessagePtr<char>)>& callback);
		std::function<bool(uint16_t, CMessagePtr<char>)>&
				getCallback(uint32_t nMessageID);
		std::function<bool(SClientSessionInfo, CMessagePtr<char>)>&
				getGateForwardCallback(uint32_t nMessageID);

		void	onConnectToMaster();

	private:
		std::map<uint32_t, std::function<bool(uint16_t, CMessagePtr<char>)>>
				m_mapNodeCallback;
		std::map<uint32_t, std::function<bool(SClientSessionInfo, CMessagePtr<char>)>>
				m_mapGateForwardCallback;
		std::map<uint32_t, std::string>
				m_mapMessageName;
	};
}