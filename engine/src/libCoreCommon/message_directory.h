#pragma once
#include "libBaseCommon/noncopyable.h"

#include "core_common.h"

#include <map>
#include <vector>

namespace core
{
	class CMessageDirectory :
		public base::noncopyable
	{
	public:
		CMessageDirectory();
		~CMessageDirectory();

		bool init();

		void					registCallback(uint32_t nMessageID, const ServiceCallback& callback);
		void					registCallback(uint32_t nMessageID, const GateClientCallback& callback);
		ServiceCallback&		getCallback(uint32_t nMessageID);
		GateClientCallback&		getGateClientCallback(uint32_t nMessageID);

		void					registServiceGlobalBeforeCallback(const ServiceGlobalCallback& callback);
		void					registServiceGlobalAfterCallback(const ServiceGlobalCallback& callback);
		const std::vector<ServiceGlobalCallback>&
								getServiceGlobalBeforeCallback();
		const std::vector<ServiceGlobalCallback>&
								getServiceGlobalAfterCallback();

		void					onConnectToMaster();

		void					addMessage(const std::string& szServiceName, uint32_t nMessageID, bool bGate);
		void					delMessage(const std::string& szServiceName, uint32_t nMessageID, bool bGate);
		const std::vector<std::string>&
								getMessageServiceName(uint32_t nMessageID, bool bGate) const;
	private:
		void					sendMessageInfo(bool bGate, uint32_t nMessageID, bool bFull);

	private:
		std::map<uint32_t, ServiceCallback>				m_mapServiceCallback;
		std::map<uint32_t, GateClientCallback>			m_mapGateClientCallback;

		std::vector<ServiceGlobalCallback>				m_vecServiceGlobalBeforeCallback;
		std::vector<ServiceGlobalCallback>				m_vecServiceGlobalAfterCallback;

		std::map<uint32_t, std::vector<std::string>>	m_mapServiceMessageDirectory;
		std::map<uint32_t, std::vector<std::string>>	m_mapGateClientMessageDirectory;
	};
}