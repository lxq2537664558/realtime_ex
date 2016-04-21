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

		bool	init();

		void	registerCallback(const std::string& szMessageName, const ServiceCallback& callback);
		void	registerCallback(const std::string& szMessageName, const GateClientCallback& callback);
		ServiceCallback&	
				getCallback(uint32_t nMessageID);
		GateClientCallback&
				getGateClientCallback(uint32_t nMessageID);

		void	addGlobalBeforeFilter(const ServiceGlobalFilter& callback);
		void	addGlobalAfterFilter(const ServiceGlobalFilter& callback);
		const std::vector<ServiceGlobalFilter>&
				getGlobalBeforeFilter();
		const std::vector<ServiceGlobalFilter>&
				getGlobalAfterFilter();

		const std::string&
				getMessageName(uint32_t nMessageID) const;

		void	onConnectToMaster();

		void	addMessage(const std::string& szServiceName, uint32_t nMessageID, bool bGate);
		void	delMessage(const std::string& szServiceName, uint32_t nMessageID, bool bGate);
		const std::vector<std::string>&
				getServiceName(const std::string& szMessageName, bool bGate) const;

	private:
		void	sendMessageInfo(const std::string& szMessageName, bool bGate);

	private:
		std::map<uint32_t, ServiceCallback>				m_mapServiceCallback;
		std::map<uint32_t, GateClientCallback>			m_mapGateClientCallback;
		std::map<uint32_t, std::string>					m_mapMessageName;

		std::vector<ServiceGlobalFilter>				m_vecServiceGlobalBeforeFilter;
		std::vector<ServiceGlobalFilter>				m_vecServiceGlobalAfterFilter;

		std::map<uint32_t, std::vector<std::string>>	m_mapServiceMessageDirectory;
		std::map<uint32_t, std::vector<std::string>>	m_mapGateClientMessageDirectory;
	};
}